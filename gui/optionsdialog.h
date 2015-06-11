#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>

namespace Ui
{
    class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit OptionsDialog(QWidget *parent = 0);
    ~OptionsDialog();
private:
    void loadSettings();
    void saveChanges();
private slots:
    void on_cancelButton_pressed();
    void on_validate_pressed();
    void on_nbThreads_valueChanged(int threads);
    void on_defratiomem_stateChanged(int state);
    void on_defratio_valueChanged(int ratio);
private:
    Ui::OptionsDialog *ui;
    bool hasChanges;
};

#endif // OPTIONSDIALOG_H
