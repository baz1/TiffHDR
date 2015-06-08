#include "loadingdialog.h"
#include "ui_loadingdialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>

#define MAX_SEPARATION 64

LoadingDialog::LoadingDialog(QWidget *parent, int nThreads)
    : QDialog(parent), ui(new Ui::LoadingDialog), nThreads(nThreads), currentPos(0)
{
    Q_ASSERT((nThreads >= 1) && (nThreads <= MAX_SEPARATION));
    ui->setupUi(this);
    setModal(true);
    labels = new QLabel*[nThreads];
    subPrgss = new QProgressBar*[nThreads];
    currentMult = new int[nThreads];
    for (int i = nThreads; --i >= 0;)
    {
        QVBoxLayout *layout = new QVBoxLayout();
        labels[i] = new QLabel(tr("Loading..."), this);
        layout->addWidget(labels[i]);
        subPrgss[i] = new QProgressBar(this);
        subPrgss[i]->setValue(0);
        layout->addWidget(subPrgss[i]);
        ui->horizontalLayout->addLayout(layout);
        currentMult[i] = 0;
    }
#ifndef QT_NO_DEBUG
    currentTo = 0;
#endif
    update();
}

LoadingDialog::~LoadingDialog()
{
    delete[] currentMult;
    delete[] subPrgss;
    delete[] labels;
    delete ui;
}

void LoadingDialog::newStep(QString description, int mainValue)
{
#ifndef QT_NO_DEBUG
    Q_ASSERT(nThreads == 1);
    Q_ASSERT((mainValue >= 0) && (currentTo + mainValue <= 100));
    currentTo += mainValue;
#endif
    int remainingP = 100 - subPrgss[0]->value();
    if (remainingP > 0)
    {
        currentPos += currentMult[0] * remainingP;
        ui->mainPrgss->setValue(currentPos / 100);
    }
    currentMult[0] = mainValue;
    subPrgss[0]->setValue(0);
    labels[0]->setText(description);
}

void LoadingDialog::newStep(int threadId, QString description, int mainValue)
{
#ifndef QT_NO_DEBUG
    Q_ASSERT((threadId >= 0) && (threadId < nThreads));
    Q_ASSERT((mainValue >= 0) && (currentTo + mainValue <= 100));
    currentTo += mainValue;
#endif
    int remainingP = 100 - subPrgss[threadId]->value();
    if (remainingP > 0)
    {
        currentPos += currentMult[threadId] * remainingP;
        ui->mainPrgss->setValue(currentPos / 100);
    }
    currentMult[threadId] = mainValue;
    subPrgss[threadId]->setValue(0);
    labels[threadId]->setText(description);
}

void LoadingDialog::setSubStep(int value)
{
    Q_ASSERT(nThreads == 1);
    Q_ASSERT((value >= 0) && (value <= 100));
    int add = value - subPrgss[0]->value();
    if (!add)
        return;
    subPrgss[0]->setValue(value);
    currentPos += currentMult[0] * add;
    ui->mainPrgss->setValue(currentPos / 100);
}

void LoadingDialog::setSubStep(int threadId, int value)
{
    Q_ASSERT((threadId >= 0) && (threadId < nThreads));
    Q_ASSERT((value >= 0) && (value <= 100));
    int add = value - subPrgss[threadId]->value();
    if (!add)
        return;
    subPrgss[threadId]->setValue(value);
    currentPos += currentMult[threadId] * add;
    ui->mainPrgss->setValue(currentPos / 100);
}
