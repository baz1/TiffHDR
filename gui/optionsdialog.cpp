#include "optionsdialog.h"
#include "ui_optionsdialog.h"

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
