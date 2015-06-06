#include "loaddialog.h"
#include "ui_loaddialog.h"

#include <QMessageBox>
#include <QFileDialog>

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
    Q_UNUSED(filename) // TODO
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
