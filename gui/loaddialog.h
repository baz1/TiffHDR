#ifndef LOADDIALOG_H
#define LOADDIALOG_H

#include <QDialog>
#include <QStringList>

#include "../rendering/data.h"

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
public:
    static void setDebugMode(bool isEnabled);
    static QList<PhotoItem> loadTiffFiles(QWidget *parent = 0, QStringList startList = QStringList());
private:
    void addFile(const QString filename);
    bool tryAppending(PhotoItem item);
private slots:
    void on_addTIFF_pressed();
    void on_validate_pressed();
    void on_cancel_pressed();
    void on_ratioSlider_valueChanged(int value);
private:
    static bool debug;
    Ui::LoadDialog *ui;
    QList<PhotoItem> files;
    bool isAccepted;
};

#endif // LOADDIALOG_H
