#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <tiffio.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    TIFF* tif = TIFFOpen("foo.tif", "r");
    if (tif)
    {
        TIFFClose(tif);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
