#include "loadingdialog.h"
#include "ui_loadingdialog.h"

LoadingDialog::LoadingDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::LoadingDialog), currentFrom(0), currentMult(100)
{
    ui->setupUi(this);
    setModal(true);
}

LoadingDialog::~LoadingDialog()
{
    delete ui;
}

void LoadingDialog::setStep(QString description, int from, int to)
{
    Q_ASSERT((from >= 0) && (to >= from) && (to <= 100));
    ui->mainPrgss->setValue(from);
    ui->label->setText(description);
    ui->subPrgss->setValue(0);
    currentFrom = from;
    currentMult = to - from;
}

void LoadingDialog::setSubStep(int value)
{
    ui->subPrgss->setValue(value);
    ui->mainPrgss->setValue(currentFrom + ((value * currentMult) / 100));
}
