#ifndef REDUCER_H
#define REDUCER_H

#include <QThread>
#include <QPixmap>
#include <tiffio.h>

class Reducer : public QThread
{
    Q_OBJECT
public:
    explicit Reducer(int ratio, int threadId = 0, QObject *parent = 0);
    void setTask(int taskId, QString filename, int dirIndex);
    inline int getTaskId() const;
    inline QPixmap getPixmap() const;
protected:
    void run();
signals:
    void renderingStatus(int threadId, int value);
private:
    void loadTIFF();
    template <typename Rendering> bool rTIFFCaller(QRgb *img, TIFF *tiffFile, unsigned int PW, unsigned int PH);
    template <typename Rendering, typename SamplingT, typename Sampling>
        bool renderTIFF(QRgb *img, TIFF *tiffFile, unsigned int PW, unsigned int PH);
    void updateProgress(int &lastP, unsigned int &current, unsigned int &max);
private:
    class Sample8b
    {
    public:
        static const uint32 div = 1;
        static inline uint32 get8Bits(uint8 d)
        {
            return (uint32) d;
        }
    };

    class Sample16b
    {
    public:
        static const uint32 div = 0x100;
        static inline uint32 get8Bits(uint16 d)
        {
            return ((uint32) d) >> 8;
        }
    };
private:
    int threadId, ratio;
    int taskId;
    QString filename;
    int dirIndex;
    QPixmap result;
};

inline int Reducer::getTaskId() const
{
    return taskId;
}

inline QPixmap Reducer::getPixmap() const
{
    return result;
}

template <typename Rendering> bool Reducer::rTIFFCaller(QRgb *img, TIFF *tiffFile, unsigned int PW, unsigned int PH)
{
    uint16 bps;
    TIFFGetField(tiffFile, TIFFTAG_BITSPERSAMPLE, &bps);
    if (bps == 16)
    {
        return renderTIFF<Rendering, uint16, Sample16b>(img, tiffFile, PW, PH);
    } else if (bps == 8)
    {
        return renderTIFF<Rendering, uint8, Sample8b>(img, tiffFile, PW, PH);
    } else {
        fprintf(stderr, "Error: Unsupported number of bits per sample (%hu) in file \"%s\".\n", bps, qPrintable(filename));
        return false;
    }
}

template <typename Rendering, typename SamplingT, typename Sampling>
    bool Reducer::renderTIFF(QRgb *img, TIFF *tiffFile, unsigned int PW, unsigned int PH)
{
    uint16 config, nsamples;
    TIFFGetField(tiffFile, TIFFTAG_PLANARCONFIG, &config);
    if ((config != PLANARCONFIG_CONTIG) && (config != PLANARCONFIG_SEPARATE))
    {
        fprintf(stderr, "Error: Unknown planar configuration (%hu) in file \"%s\".\n", config, qPrintable(filename));
        return false;
    }
    TIFFGetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL, &nsamples);
    if (nsamples < 1)
    {
        fprintf(stderr, "Error: No samples in file \"%s\".\n", qPrintable(filename));
        return false;
    }
    tdata_t buf = _TIFFmalloc(TIFFScanlineSize(tiffFile));
    if (!buf)
    {
        fprintf(stderr, "Error: Not enough memory.\n");
        return false;
    }
    uint32 lineRAWSize = (nsamples < 3 ? 1 : 3) * PW;
    uint32 *lineRAWData = (ratio <= 1) ? NULL : (new uint32[lineRAWSize]);
    lineRAWSize <<= 2;
    int lastP = 0;
    if (config == PLANARCONFIG_CONTIG)
    {
        uint32 tiff_y = 0;
        for (unsigned int y = 0; y < PH; ++y)
        {
            updateProgress(lastP, y, PH);
            if (ratio <= 1)
            {
                TIFFReadScanline(tiffFile, buf, tiff_y++);
                SamplingT *buffer = static_cast<SamplingT*>(buf);
                if (nsamples < 3)
                {
                    for (unsigned int x = 0; x < PW; ++x)
                    {
                        uint32 c = Sampling::get8Bits(buffer[x]);
                        Rendering::pixel(img, x, y, PW, PH) = c | (c << 8) | (c << 16) | (0xffLU << 24);
                    }
                } else {
                    for (unsigned int x = 0; x < PW; ++x)
                    {
                        int r = Sampling::get8Bits(*(buffer++));
                        int g = Sampling::get8Bits(*(buffer++));
                        int b = Sampling::get8Bits(*(buffer++));
                        Rendering::pixel(img, x, y, PW, PH) = qRgb(r, g, b);
                    }
                }
                continue;
            }
            memset(lineRAWData, 0, lineRAWSize);
            uint32 *linePtr;
            for (unsigned int sy = ratio; sy > 0; --sy)
            {
                linePtr = lineRAWData;
                TIFFReadScanline(tiffFile, buf, tiff_y++);
                SamplingT *buffer = static_cast<SamplingT*>(buf);
                if (nsamples < 3)
                {
                    for (unsigned int x = 0; x < PW; ++x)
                    {
                        for (unsigned int sx = ratio; sx > 0; --sx)
                        {
                            *linePtr += *buffer;
                            buffer += nsamples;
                        }
                        ++linePtr;
                    }
                } else {
                    for (unsigned int x = 0; x < PW; ++x)
                    {
                        for (unsigned int sx = ratio; sx > 0; --sx)
                        {
                            *linePtr += buffer[0];
                            *(linePtr + 1) += buffer[1];
                            *(linePtr + 2) += buffer[2];
                            buffer += nsamples;
                        }
                        linePtr += 3;
                    }
                }
            }
            if (nsamples < 3)
            {
                uint32 div = Sampling::div * ratio * ratio;
                for (unsigned int x = 0; x < PW; ++x)
                {
                    uint32 c = 0xff & (lineRAWData[x] / div);
                    Rendering::pixel(img, x, y, PW, PH) = c | (c << 8) | (c << 16) | (0xffLU << 24);
                }
            } else {
                linePtr = lineRAWData;
                uint32 div = Sampling::div * ratio * ratio;
                for (unsigned int x = 0; x < PW; ++x, linePtr += 3)
                    Rendering::pixel(img, x, y, PW, PH) = qRgb(linePtr[0] / div, linePtr[1] / div, linePtr[2] / div);
            }
        }
    } else {
        uint32 tiff_y = 0;
        for (unsigned int y = 0; y < PH; ++y)
        {
            updateProgress(lastP, y, PH);
            if (ratio <= 1)
            {
                if (nsamples < 3)
                {
                    TIFFReadScanline(tiffFile, buf, tiff_y++, 0);
                    SamplingT *buffer = static_cast<SamplingT*>(buf);
                    for (unsigned int x = 0; x < PW; ++x)
                    {
                        uint32 c = Sampling::get8Bits(buffer[x]);
                        Rendering::pixel(img, x, y, PW, PH) = c | (c << 8) | (c << 16) | (0xffLU << 24);
                    }
                } else {
                    TIFFReadScanline(tiffFile, buf, tiff_y++, 0);
                    SamplingT *buffer = static_cast<SamplingT*>(buf);
                    for (unsigned int x = 0; x < PW; ++x)
                        Rendering::pixel(img, x, y, PW, PH) = (Sampling::get8Bits(buffer[x]) << 16) | (0xffLU << 24);
                    TIFFReadScanline(tiffFile, buf, tiff_y++, 1);
                    for (unsigned int x = 0; x < PW; ++x)
                        Rendering::pixel(img, x, y, PW, PH) |= Sampling::get8Bits(buffer[x]) << 8;
                    TIFFReadScanline(tiffFile, buf, tiff_y++, 2);
                    for (unsigned int x = 0; x < PW; ++x)
                        Rendering::pixel(img, x, y, PW, PH) |= Sampling::get8Bits(buffer[x]);
                }
                continue;
            }
            memset(lineRAWData, 0, lineRAWSize);
            uint32 *linePtr;
            if (nsamples < 3)
            {
                for (unsigned int sy = ratio; sy > 0; --sy)
                {
                    linePtr = lineRAWData;
                    TIFFReadScanline(tiffFile, buf, tiff_y++, 0);
                    SamplingT *buffer = static_cast<SamplingT*>(buf);
                    for (unsigned int x = 0; x < PW; ++x)
                    {
                        for (unsigned int sx = ratio; sx > 0; --sx)
                            *linePtr += *(buffer++);
                        ++linePtr;
                    }
                }
                uint32 div = Sampling::div * ratio * ratio;
                for (unsigned int x = 0; x < PW; ++x)
                {
                    uint32 c = 0xff & (lineRAWData[x] / div);
                    Rendering::pixel(img, x, y, PW, PH) = c | (c << 8) | (c << 16) | (0xffLU << 24);
                }
            } else {
                for (unsigned int sy = ratio; sy > 0; --sy)
                {
                    for (int sample = 2; sample >= 0; --sample)
                    {
                        linePtr = lineRAWData + sample;
                        TIFFReadScanline(tiffFile, buf, tiff_y++, sample);
                        SamplingT *buffer = static_cast<SamplingT*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                        {
                            for (unsigned int sx = ratio; sx > 0; --sx)
                                *linePtr += *(buffer++);
                            linePtr += 3;
                        }
                    }
                }
                linePtr = lineRAWData;
                uint32 div = Sampling::div * ratio * ratio;
                for (unsigned int x = 0; x < PW; ++x, linePtr += 3)
                    Rendering::pixel(img, x, y, PW, PH) = qRgb(linePtr[0] / div, linePtr[1] / div, linePtr[2] / div);
            }
        }
    }
    delete[] lineRAWData;
    _TIFFfree(buf);
    return true;
}

#endif // REDUCER_H
