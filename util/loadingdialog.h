#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H

#include <QDialog>

class QLabel;
class QProgressBar;
class QVBoxLayout;

namespace Ui
{
    class LoadingDialog;
}

class LoadingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoadingDialog(QWidget *parent = 0, int nThreads = 1);
    ~LoadingDialog();
public slots:
    void newStep(QString description, int mainValue);
    void newStep(int threadId, QString description, int mainValue);
    void setSubStep(int value);
    void setSubStep(int threadId, int value);
private:
    Ui::LoadingDialog *ui;
    int nThreads;
    QLabel **labels;
    QProgressBar **subPrgss;
    int currentPos, *currentMult;
#ifndef QT_NO_DEBUG
    int currentTo;
#endif
};

#endif // LOADINGDIALOG_H
