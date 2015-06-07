#include "loaddialog.h"
#include "ui_loaddialog.h"

#include "loadingdialog.h"
#include "reducer.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

#include <stdio.h>
#include <tiffio.h>

bool LoadDialog::debug;

LoadDialog::LoadDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoadDialog), isAccepted(false)
{
    ui->setupUi(this);
    setModal(true);
}

LoadDialog::~LoadDialog()
{
    delete ui;
}

void LoadDialog::setDebugMode(bool isEnabled)
{
    debug = isEnabled;
}

QList<TIFF_File> LoadDialog::loadTiffFiles(QWidget *parent, QStringList startList)
{
    LoadDialog dlg(parent);
    dlg.show();
    foreach (const QString &filename, startList)
        dlg.addFile(filename);
    dlg.exec();
    QList<TIFF_File> result;
    if (dlg.isAccepted)
    {
        result = dlg.files;
        LoadingDialog ldlg(parent);
        ldlg.show();
        Reducer reducer(dlg.ui->ratioSlider->value());
        connect(&reducer, SIGNAL(renderingStatus(int)), &ldlg, SLOT(setSubStep(int)));
        int fromP = 0;
        for (int i = 0; i < result.length(); ++i)
        {
            int toP = ((i + 1) * 100) / result.length();
            ldlg.setStep(tr("Converting TIFF n.%1 for display..."), fromP, toP);
            reducer.setFilename(result.at(i).filename);
            reducer.start();
            reducer.wait();
            result[i].display = reducer.getPixmap();
            fromP = toP;
        }
        ldlg.close();
    }
    return result;
}

void LoadDialog::addFile(const QString filename)
{
    TIFF *tiffFile = TIFFOpen(qPrintable(filename), "r");
    if (!tiffFile)
    {
        QMessageBox::warning(this, tr("Error:"), tr("Unable to load TIFF file %1.").arg(filename));
        return;
    }
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
        photo.dirIndex = dir;
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
        //: Formatting for the list of TIFF files to load; %1 is the filename while %2 is the directory number
        ui->tiffList->addItem(tr("%1 (dir. %2)").arg(filename).arg(photos.first().dirIndex));
        if (files.length() >= 2)
            ui->validate->setEnabled(true);
        ui->removeTIFF->setEnabled(true);
        return;
    }
    bool ok;
    QString chosen = QInputDialog::getItem(this, tr("Directory selection"), tr("Please select the directory that corresponds to your photo:"),
                                           descriptions, 0, false, &ok);
    if (!ok)
        return;
    int index = descriptions.indexOf(chosen);
    files.append(photos.at(index));
    //: Formatting for the list of TIFF files to load; %1 is the filename while %2 is the directory number
    ui->tiffList->addItem(tr("%1 (dir. %2)").arg(filename).arg(photos.at(index).dirIndex));
    if (files.length() >= 2)
        ui->validate->setEnabled(true);
    ui->removeTIFF->setEnabled(true);
}

void LoadDialog::on_addTIFF_pressed()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Select your TIFF photos"), QString(), tr("TIFF images (*.tif *.tiff)"));
    foreach (const QString &filename, filenames)
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

void LoadDialog::on_ratioSlider_valueChanged(int value)
{
    ui->ratioLabel->setText(tr("Ratio: 1/%1").arg(value));
}
