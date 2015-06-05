#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    Q_UNUSED(filenames) // TODO
}
