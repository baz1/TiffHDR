#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H

#include <QDialog>

namespace Ui
{
    class LoadingDialog;
}

class LoadingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoadingDialog(QWidget *parent = 0);
    ~LoadingDialog();
public slots:
    void setStep(QString description, int from, int to);
    void setSubStep(int value);
private:
    Ui::LoadingDialog *ui;
    int currentFrom, currentMult;
};

#endif // LOADINGDIALOG_H
