#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "../util/settings.h"

OptionsDialog::OptionsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::OptionsDialog), hasChanges(false)
{
    ui->setupUi(this);
    setModal(true);
    setAttribute(Qt::WA_DeleteOnClose);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::loadSettings()
{
    ui->nbThreads->setValue(Settings::getNumberOfThreads());
}

void OptionsDialog::saveChanges()
{
    Settings::setNumberOfThreads(ui->nbThreads->value());
}

void OptionsDialog::on_cancelButton_pressed()
{
    close();
}

void OptionsDialog::on_validate_pressed()
{
    if (hasChanges)
        saveChanges();
    close();
}

void OptionsDialog::on_nbThreads_valueChanged(int threads)
{
    Q_UNUSED(threads)
    hasChanges = true;
}
