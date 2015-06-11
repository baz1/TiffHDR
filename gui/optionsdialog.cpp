#include "optionsdialog.h"
#include "ui_optionsdialog.h"

OptionsDialog::OptionsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    setModal(true);
    setAttribute(Qt::WA_DeleteOnClose);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}
