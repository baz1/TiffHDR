#include "loaddialog.h"
#include "ui_loaddialog.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

#include <stdio.h>
#include <tiffio.h>

LoadDialog::LoadDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoadDialog), isAccepted(false)
{
    ui->setupUi(this);
    setModal(true);
}

LoadDialog::~LoadDialog()
{
    delete ui;
}

void LoadDialog::addFile(QString filename)
{
    TIFF *tiffFile = TIFFOpen(qPrintable(filename), "r");
    if (!tiffFile)
    {
        QMessageBox::warning(this, tr("Error:"), tr("Unable to load TIFF file %1.").arg(filename));
        return;
    }
    /* Checking the number of directories */
    QStringList descriptions;
    QList<TIFF_File> photos;
    int dir = 0;
    while (true)
    {
        uint16 config;
        TIFFGetField(tiffFile, TIFFTAG_PLANARCONFIG, &config);
        if ((config != PLANARCONFIG_CONTIG) && (config != PLANARCONFIG_SEPARATE))
        {
            if (debug)
            {
                fprintf(stderr, "Warning: Unknown planar configuration in directory %d of \"%s\" (%d):\n",
                        dir, qPrintable(filename), config);
                TIFFPrintDirectory(tiffFile, stderr, TIFFPRINT_NONE);
            }
            continue;
        }
        uint16 bps;
        TIFFGetField(tiffFile, TIFFTAG_BITSPERSAMPLE, &bps);
        if ((bps != 8) && (bps != 16) && (bps != 32))
        {
            if (debug)
            {
                fprintf(stderr, "Warning: Unsupported number of bits per sample in directory %d of \"%s\" (%d):\n",
                        dir, qPrintable(filename), bps);
                TIFFPrintDirectory(tiffFile, stderr, TIFFPRINT_NONE);
            }
            continue;
        }
        TIFF_File photo;
        uint16 nsamples;
        TIFFGetField(tiffFile, TIFFTAG_IMAGEWIDTH, &photo.width);
        TIFFGetField(tiffFile, TIFFTAG_IMAGELENGTH, &photo.height);
        TIFFGetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL, &nsamples);
        descriptions.append(tr("Dir. %1: %2x%3 image with %4 bits per sample and %5 samples")
                            .arg(dir).arg(photo.width).arg(photo.height).arg(bps).arg(nsamples));
        photos.append(photo);
        if (!TIFFReadDirectory(tiffFile))
            break;
        ++dir;
    }
    TIFFClose(tiffFile);
    if (photos.isEmpty())
    {
        QMessageBox::warning(this, tr("Error:"), tr("Did not find any compatible directory in the TIFF file:\n%1").arg(filename));
        return;
    }
    if (photos.length() == 1)
    {
        files.append(photos.first());
        // TODO update GUI
        return;
    }
    bool ok;
    int index = QInputDialog::getItem(this, tr("Directory selection"), tr("Please select the directory that corresponds to your photo:"),
                                      descriptions, 0, false, &ok);
    if (!ok)
        return;
    files.append(photos.at(index));
    // TODO update GUI
}

void LoadDialog::setDebugMode(bool isEnabled)
{
    debug = isEnabled;
}

QList<TIFF_File> LoadDialog::loadTiffFiles(QWidget *parent, QStringList startList)
{
    LoadDialog dlg(parent);
    dlg.show();
    foreach (QString filename, startList)
        dlg.addFile(filename);
    dlg.exec();
    if (!dlg.isAccepted)
        dlg.files.clear();
    return dlg.files;
}

void LoadDialog::on_addTIFF_pressed()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Select your TIFF photos"), QString(), tr("TIFF images (*.tif *.tiff)"));
    foreach (QString filename, filenames)
        addFile(filename);
}

void LoadDialog::on_validate_pressed()
{
    Q_ASSERT(files.length() >= 2);
    isAccepted = true;
    close();
}

void LoadDialog::on_cancel_pressed()
{
    close();
}
