#include "loaddialog.h"
#include "ui_loaddialog.h"

#include "../util/loadingdialog.h"
#include "../util/settings.h"
#include "../rendering/reducer.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDir>

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

QList<PhotoItem> LoadDialog::loadTiffFiles(QWidget *parent, QStringList startList)
{
    LoadDialog *dlg = new LoadDialog(parent);
    dlg->show();
    foreach (const QString &filename, startList)
        dlg->addFile(filename);
    dlg->exec();
    QList<PhotoItem> result = dlg->files;
    if (!dlg->isAccepted)
        result.clear();
    int ratioValue = dlg->ui->ratioSlider->value();
    delete dlg;
    if (!result.isEmpty())
    {
        /* Getting number of threads */
        int nThreads = Settings::getThreads();
        if (debug)
            fprintf(stderr, "Debug: Trying to render display pixmaps with %d (>= 1) threads.\n", nThreads);
        /* Initialization */
        LoadingDialog ldlg(parent, nThreads);
        ldlg.show();
        Reducer **reducers = new Reducer*[nThreads];
        QEventLoop pause;
        for (int j = nThreads; --j >= 0;)
        {
            reducers[j] = new Reducer(ratioValue, j);
            connect(reducers[j], SIGNAL(renderingStatus(int,int)), &ldlg, SLOT(setSubStep(int,int)));
            connect(reducers[j], SIGNAL(finished()), &pause, SLOT(quit()));
        }
        int fromP = 0;
        /* Multithreaded rendering of the pixmaps */
        for (int i = 0; i < result.length(); ++i)
        {
            int toP = ((i + 1) * 100) / result.length();
            /* Getting a free thread */
            int j = nThreads;
            while (true)
            {
                if (!reducers[--j]->isRunning())
                    break;
                if (j <= 0)
                {
                    pause.exec();
                    j = nThreads;
                }
            }
            /* Saving the result of thread j if there is one */
            if (reducers[j]->getTaskId() >= 0)
                result[reducers[j]->getTaskId()].display = reducers[j]->getPixmap();
            /* Rendering file i on thread j */
            //: Small caption for the processing of the TIFF file number %1 for display inside the application
            ldlg.newStep(j, tr("TIFF n.%1...").arg(i), toP - fromP);
            fromP = toP;
            reducers[j]->setTask(i, result.at(i).filename, result.at(i).dirIndex);
            reducers[j]->start();
        }
        /* Waiting for all threads to finish */
        for (int j = nThreads; --j >= 0;)
        {
            while (reducers[j]->isRunning())
                pause.exec();
            /* Saving the result of thread j if there is one (we may overwrite which is not a big deal) */
            if (reducers[j]->getTaskId() >= 0)
                result[reducers[j]->getTaskId()].display = reducers[j]->getPixmap();
        }
        /* Deleting all the remaining objects */
        for (int j = nThreads; --j >= 0;)
            delete reducers[j];
        delete[] reducers;
        ldlg.close();
    }
    return result;
}

QString getExtension(QString filename)
{
    int index = filename.indexOf(QChar('.'));
    if (index < 0)
        return QString();
    return filename.mid(index + 1).toLower();
}

void LoadDialog::addFile(const QString filename)
{
    QString extension = getExtension(filename);
    if ((extension == "tif") || (extension == "tiff"))
    {
        TIFF *tiffFile = TIFFOpen(qPrintable(filename), "r");
        if (!tiffFile)
        {
            QMessageBox::warning(this, tr("Error:"), tr("Unable to load TIFF file %1.").arg(filename));
            return;
        }
        QStringList descriptions;
        QList<PhotoItem> photos;
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
            if ((bps != 8) && (bps != 16))
            {
                if (debug)
                {
                    fprintf(stderr, "Warning: Unsupported number of bits per sample in directory %d of \"%s\" (%d):\n",
                            dir, qPrintable(filename), bps);
                    TIFFPrintDirectory(tiffFile, stderr, TIFFPRINT_NONE);
                }
                continue;
            }
            PhotoItem photo;
            uint16 nsamples;
            TIFFGetField(tiffFile, TIFFTAG_IMAGEWIDTH, &photo.width);
            TIFFGetField(tiffFile, TIFFTAG_IMAGELENGTH, &photo.height);
            TIFFGetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL, &nsamples);
            photo.dirIndex = dir;
            photo.filename = filename;
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
            if (!tryAppending(photos.first()))
                return;
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
        if (!tryAppending(photos.at(index)))
            return;
        //: Formatting for the list of TIFF files to load; %1 is the filename while %2 is the directory number
        ui->tiffList->addItem(tr("%1 (dir. %2)").arg(filename).arg(photos.at(index).dirIndex));
        if (files.length() >= 2)
            ui->validate->setEnabled(true);
        ui->removeTIFF->setEnabled(true);
    } else {
        PhotoItem photo;
        photo.filename = filename;
        QImage img(filename);
        if (img.isNull())
        {
            QMessageBox::warning(this, tr("Error:"), tr("Unrecognized image: \"%1\".").arg(filename));
            return;
        }
        photo.width = img.width();
        photo.height = img.height();
        photo.dirIndex = -1;
        if (!tryAppending(photo))
            return;
        //: Formatting for the list of files to load; %1 is the filename
        ui->tiffList->addItem(tr("%1").arg(filename));
        if (files.length() >= 2)
            ui->validate->setEnabled(true);
        ui->removeTIFF->setEnabled(true);
    }
}

bool LoadDialog::tryAppending(PhotoItem item)
{
    foreach (const PhotoItem &file, files)
    {
        if (QDir(file.filename).canonicalPath() == QDir(item.filename).canonicalPath())
        {
            QMessageBox::warning(this, tr("Error:"), tr("This photo has already been added to the list."));
            return false;
        }
    }
    files.append(item);
    return true;
}

void LoadDialog::on_addTIFF_pressed()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Select your photos"), QString(),
                                                          tr("TIFF images (*.tif *.tiff);;Other image files (*.bmp *.gif *.jpg *jpeg *.png)"));
    foreach (const QString &filename, filenames)
        addFile(filename);
}

void LoadDialog::on_validate_pressed()
{
    Q_ASSERT(files.length() >= 2);
    quint32 w = files.at(0).width, h = files.at(0).height;
    for (int i = files.length() - 1; i > 0; --i)
    {
        if ((files.at(i).width != w) || (files.at(i).height != h))
        {
            QMessageBox::warning(this, tr("Error:"), tr("All the photos do not have the same size."));
            return;
        }
    }
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
