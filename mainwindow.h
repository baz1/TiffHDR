#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(bool debug = false, QWidget *parent = 0);
    ~MainWindow();
    void startTIFFLoading(QStringList filenames);
private:
    Ui::MainWindow *ui;
    bool debug;
};

#endif // MAINWINDOW_H
