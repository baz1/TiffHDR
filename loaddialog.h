#ifndef LOADDIALOG_H
#define LOADDIALOG_H

#include <QDialog>

#include "data.h"

namespace Ui
{
    class LoadDialog;
}

class LoadDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoadDialog(QWidget *parent = 0);
    ~LoadDialog();
private slots:
    void on_validation_rejected();
private:
    Ui::LoadDialog *ui;
    QList<TIFF_File> files;
};

#endif // LOADDIALOG_H
