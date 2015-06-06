#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "loaddialog.h"

MainWindow::MainWindow(bool debug, QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), debug(debug)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startTIFFLoading(QStringList filenames)
{
    QList<TIFF_File> tiffFiles = LoadDialog::loadTiffFiles(this, filenames);
    Q_UNUSED(tiffFiles) // TODO
}

void MainWindow::on_loadTIFF_triggered()
{
    startTIFFLoading();
}
