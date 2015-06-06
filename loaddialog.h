#ifndef LOADDIALOG_H
#define LOADDIALOG_H

#include <QDialog>
#include <QStringList>

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
    void addFile(QString filename);
public:
    static QList<TIFF_File> loadTiffFiles(QWidget *parent = 0, QStringList startList = QStringList());
private slots:
    void on_addTIFF_pressed();
    void on_validate_pressed();
    void on_cancel_pressed();
private:
    Ui::LoadDialog *ui;
    QList<TIFF_File> files;
    bool isAccepted;
};

#endif // LOADDIALOG_H
