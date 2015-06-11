#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "loaddialog.h"
#include "optionsdialog.h"

MainWindow::MainWindow(bool debug, QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), debug(debug)
{
    ui->setupUi(this);
    LoadDialog::setDebugMode(debug);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startTIFFLoading(QStringList filenames)
{
    QList<PhotoItem> tiffFiles = LoadDialog::loadTiffFiles(this, filenames);
    Q_UNUSED(tiffFiles) // TODO
}

void MainWindow::on_loadTIFF_triggered()
{
    startTIFFLoading();
}

void MainWindow::on_actionOptions_triggered()
{
    OptionsDialog *dlg = new OptionsDialog(this);
    dlg->show();
}
