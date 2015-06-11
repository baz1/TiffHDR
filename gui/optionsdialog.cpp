#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "../util/settings.h"

OptionsDialog::OptionsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::OptionsDialog), hasChanges(false)
{
    ui->setupUi(this);
    ui->defratio->setMaximum(SETTINGS_MAX_RATIO);
    loadSettings();
    setModal(true);
    setAttribute(Qt::WA_DeleteOnClose);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::loadSettings()
{
    ui->defratio->setValue(Settings::getDefRatio());
    ui->defratiomem->setChecked(Settings::getDefRatioMem());
    ui->nbThreads->setValue(Settings::getNumberOfThreads());
}

void OptionsDialog::saveChanges()
{
    Settings::setNumberOfThreads(ui->nbThreads->value());
    Settings::setDefRatio(ui->defratio->value());
    Settings::setDefRatioMem(ui->defratiomem->isChecked());
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

void OptionsDialog::on_defratiomem_stateChanged(int state)
{
    Q_UNUSED(state)
    hasChanges = true;
}

void OptionsDialog::on_defratio_valueChanged(int ratio)
{
    Q_UNUSED(ratio)
    hasChanges = true;
}
