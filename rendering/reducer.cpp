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

class RenderInverted
{
public:
    static inline void setPixel(QRgb *img, const unsigned int x, const unsigned int y, const QRgb rgb, const unsigned int &PRL)
    {
        img[x * PRL + y] = rgb;
    }

    static inline QRgb getPixel(QRgb *img, const unsigned int x, const unsigned int y, const unsigned int &PRL)
    {
        return img[x * PRL + y];
    }
};

class RenderNormal
{
public:
    static inline void setPixel(QRgb *img, const unsigned int x, const unsigned int y, const QRgb rgb, const unsigned int &PRL)
    {
        img[y * PRL + x] = rgb;
    }

    static inline QRgb getPixel(QRgb *img, const unsigned int x, const unsigned int y, const unsigned int &PRL)
    {
        return img[y * PRL + x];
    }
};

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
    if ((orientation < 1) || (orientation > 8))
        orientation = 1;
    --orientation;
    QImage displayImg = QImage(PW, PH, QImage::Format_RGB32);
    if (displayImg.isNull())
    {
        fprintf(stderr, "Error: Not enough memory.\n");
        TIFFClose(tiffFile);
        return;
    }
    bool success;
    if (orientation & 4)
        success = renderTIFF<RenderInverted>(reinterpret_cast<QRgb*>(displayImg.bits()), tiffFile, PW, PH, PW);
    else
        success = renderTIFF<RenderNormal>(reinterpret_cast<QRgb*>(displayImg.bits()), tiffFile, PW, PH, PW);
    TIFFClose(tiffFile);
    if (!success)
        return;
    switch (orientation & 3)
    {
    case 0:
        break;
    case 1:
        displayImg = displayImg.mirrored(true, false);
        break;
    case 2:
        displayImg = displayImg.mirrored(true, true);
        break;
    case 3:
        displayImg = displayImg.mirrored(false, true);
        break;
    }
    emit renderingStatus(threadId, 100);
    result = QPixmap::fromImage(displayImg);
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
