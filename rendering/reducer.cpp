#include "reducer.h"

#include <QPainter>

#include <stdio.h>
#include <string.h>

Reducer::Reducer(int ratio, int threadId, QObject *parent) : QThread(parent), threadId(threadId), ratio(ratio), taskId(-1)
{
    Q_ASSERT((ratio <= (1 << 8)) && (ratio >= 1));
}

void Reducer::setTask(int taskId, QString filename, int dirIndex)
{
    Q_ASSERT(dirIndex >= 0);
    result = QPixmap();
    this->taskId = taskId;
    this->filename = filename;
    this->dirIndex = dirIndex;
}

void Reducer::run()
{
    if (dirIndex < 0)
    {
        result = QPixmap(filename);
        if (ratio > 1)
        {
            unsigned int PW = result.width() / ratio, PH = result.height() / ratio;
            if ((PW == 0) || (PH == 0))
            {
                fprintf(stderr, "Error: Picture is too small (\"%s\").\n", qPrintable(filename));
                return;
            }
            emit renderingStatus(threadId, 50);
            QPixmap temp = QPixmap(PW, PH);
            QPainter *painter = new QPainter(&temp);
            painter->drawPixmap(0, 0, PW, PH, result, 0, 0, ratio * PW, ratio * PH);
            /* Note: The last parameters ensure consistency with the TIFF reduction. */
            painter->end();
            delete painter;
            result = temp;
        }
        emit renderingStatus(threadId, 100);
    } else {
        loadTIFF();
    }
}

void Reducer::loadTIFF()
{
    result = QPixmap();
    TIFF *tiffFile = TIFFOpen(qPrintable(filename), "r");
    if (!tiffFile)
    {
        fprintf(stderr, "Error: Could not open the TIFF file \"%s\".\n", qPrintable(filename));
        return;
    }
    if (!TIFFSetDirectory(tiffFile, dirIndex))
    {
        fprintf(stderr, "Error: Could not select the directory %d in file \"%s\".\n", dirIndex, qPrintable(filename));
        TIFFClose(tiffFile);
        return;
    }
    uint32 w, h;
    TIFFGetField(tiffFile, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tiffFile, TIFFTAG_IMAGELENGTH, &h);
    unsigned int PW = w / ratio, PH = h / ratio;
    if ((PW == 0) || (PH == 0))
    {
        fprintf(stderr, "Error: Picture is too small (\"%s\").\n", qPrintable(filename));
        TIFFClose(tiffFile);
        return;
    }
    uint16 orientation;
    TIFFGetField(tiffFile, TIFFTAG_ORIENTATION, &orientation);
    if (orientation < 1)
        orientation = 1;
    QImage displayImg = QImage(PW, PH, QImage::Format_RGB32);
    if (displayImg.isNull())
    {
        fprintf(stderr, "Error: Not enough memory.\n");
        TIFFClose(tiffFile);
        return;
    }
    if (!renderTIFF(displayImg, tiffFile, PW, PH))
    {
        TIFFClose(tiffFile);
        return;
    }
    TIFFClose(tiffFile);
    emit renderingStatus(threadId, 100);
    result = QPixmap::fromImage(displayImg);
}

bool Reducer::renderTIFF(QImage &img, TIFF *tiffFile, unsigned int PW, unsigned int PH)
{
    uint16 bps, config, nsamples;
    TIFFGetField(tiffFile, TIFFTAG_BITSPERSAMPLE, &bps);
    if ((bps != 8) && (bps != 16))
    {
        fprintf(stderr, "Error: Unsupported number of bits per sample (%hu) in file \"%s\".\n", bps, qPrintable(filename));
        return false;
    }
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
                if (nsamples < 3)
                {
                    if (bps == 16)
                    {
                        uint16 *buffer = static_cast<uint16*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                        {
                            uint32 c = buffer[x] >> 8;
                            img.setPixel(x, y, c | (c << 8) | (c << 16) | (0xffLU << 24));
                        }
                    } else {
                        uint8 *buffer = static_cast<uint8*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                        {
                            uint32 c = buffer[x];
                            img.setPixel(x, y, c | (c << 8) | (c << 16) | (0xffLU << 24));
                        }
                    }
                } else {
                    if (bps == 16)
                    {
                        uint16 *buffer = static_cast<uint16*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                        {
                            int r = *(buffer++) >> 8;
                            int g = *(buffer++) >> 8;
                            int b = *(buffer++) >> 8;
                            img.setPixel(x, y, qRgb(r, g, b));
                        }
                    } else {
                        uint8 *buffer = static_cast<uint8*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                        {
                            int r = *(buffer++);
                            int g = *(buffer++);
                            int b = *(buffer++);
                            img.setPixel(x, y, qRgb(r, g, b));
                        }
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
                if (nsamples < 3)
                {
                    if (bps == 16)
                    {
                        uint16 *buffer = static_cast<uint16*>(buf);
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
                        uint8 *buffer = static_cast<uint8*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                        {
                            for (unsigned int sx = ratio; sx > 0; --sx)
                            {
                                *linePtr += *buffer;
                                buffer += nsamples;
                            }
                            ++linePtr;
                        }
                    }
                } else {
                    if (bps == 16)
                    {
                        uint16 *buffer = static_cast<uint16*>(buf);
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
                    } else {
                        uint8 *buffer = static_cast<uint8*>(buf);
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
            }
            if (nsamples < 3)
            {
                uint32 div = (1 << (bps - 8)) * ratio * ratio;
                for (unsigned int x = 0; x < PW; ++x)
                {
                    uint32 c = 0xff & (lineRAWData[x] / div);
                    img.setPixel(x, y, c | (c << 8) | (c << 16) | (0xffLU << 24));
                }
            } else {
                linePtr = lineRAWData;
                uint32 div = (1 << (bps - 8)) * ratio * ratio;
                for (unsigned int x = 0; x < PW; ++x, linePtr += 3)
                    img.setPixel(x, y, qRgb(linePtr[0] / div, linePtr[1] / div, linePtr[2] / div));
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
                    if (bps == 16)
                    {
                        uint16 *buffer = static_cast<uint16*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                        {
                            uint32 c = buffer[x] >> 8;
                            img.setPixel(x, y, c | (c << 8) | (c << 16) | (0xffLU << 24));
                        }
                    } else {
                        uint8 *buffer = static_cast<uint8*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                        {
                            uint32 c = buffer[x];
                            img.setPixel(x, y, c | (c << 8) | (c << 16) | (0xffLU << 24));
                        }
                    }
                } else {
                    TIFFReadScanline(tiffFile, buf, tiff_y++, 0);
                    if (bps == 16)
                    {
                        uint16 *buffer = static_cast<uint16*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                            img.setPixel(x, y, ((((uint32) buffer[x]) & 0xFF00) << 8) | (0xffLU << 24));
                    } else {
                        uint8 *buffer = static_cast<uint8*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                            img.setPixel(x, y, (((uint32) buffer[x]) << 16) | (0xffLU << 24));
                    }
                    TIFFReadScanline(tiffFile, buf, tiff_y++, 1);
                    if (bps == 16)
                    {
                        uint16 *buffer = static_cast<uint16*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                            img.setPixel(x, y, img.pixel(x, y) & (buffer[x] & 0xFF00));
                    } else {
                        uint8 *buffer = static_cast<uint8*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                            img.setPixel(x, y, img.pixel(x, y) & (((uint32) buffer[x]) << 8));
                    }
                    TIFFReadScanline(tiffFile, buf, tiff_y++, 2);
                    if (bps == 16)
                    {
                        uint16 *buffer = static_cast<uint16*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                            img.setPixel(x, y, img.pixel(x, y) & (buffer[x] >> 8));
                    } else {
                        uint8 *buffer = static_cast<uint8*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                            img.setPixel(x, y, img.pixel(x, y) & buffer[x]);
                    }
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
                    if (bps == 16)
                    {
                        uint16 *buffer = static_cast<uint16*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                        {
                            for (unsigned int sx = ratio; sx > 0; --sx)
                                *linePtr += *(buffer++);
                            ++linePtr;
                        }
                    } else {
                        uint8 *buffer = static_cast<uint8*>(buf);
                        for (unsigned int x = 0; x < PW; ++x)
                        {
                            for (unsigned int sx = ratio; sx > 0; --sx)
                                *linePtr += *(buffer++);
                            ++linePtr;
                        }
                    }
                }
                uint32 div = (1 << (bps - 8)) * ratio * ratio;
                for (unsigned int x = 0; x < PW; ++x)
                {
                    uint32 c = 0xff & (lineRAWData[x] / div);
                    img.setPixel(x, y, c | (c << 8) | (c << 16) | (0xffLU << 24));
                }
            } else {
                for (unsigned int sy = ratio; sy > 0; --sy)
                {
                    for (int sample = 2; sample >= 0; --sample)
                    {
                        linePtr = lineRAWData + sample;
                        TIFFReadScanline(tiffFile, buf, tiff_y++, sample);
                        if (bps == 16)
                        {
                            uint16 *buffer = static_cast<uint16*>(buf);
                            for (unsigned int x = 0; x < PW; ++x)
                            {
                                for (unsigned int sx = ratio; sx > 0; --sx)
                                    *linePtr += *(buffer++);
                                linePtr += 3;
                            }
                        } else {
                            uint8 *buffer = static_cast<uint8*>(buf);
                            for (unsigned int x = 0; x < PW; ++x)
                            {
                                for (unsigned int sx = ratio; sx > 0; --sx)
                                    *linePtr += *(buffer++);
                                linePtr += 3;
                            }
                        }
                    }
                }
                linePtr = lineRAWData;
                uint32 div = (1 << (bps - 8)) * ratio * ratio;
                for (unsigned int x = 0; x < PW; ++x, linePtr += 3)
                    img.setPixel(x, y, qRgb(linePtr[0] / div, linePtr[1] / div, linePtr[2] / div));
            }
        }
    }
    delete[] lineRAWData;
    _TIFFfree(buf);
    return true;
}

void Reducer::updateProgress(int &lastP, unsigned int &current, unsigned int &max)
{
    int newP = (current * 100) / max;
    if (newP > lastP)
    {
        emit renderingStatus(threadId, newP);
        lastP = newP;
    }
}
