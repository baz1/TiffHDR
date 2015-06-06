#ifndef DATA_H
#define DATA_H

#include <QObject>
#include <QList>
#include <QString>
#include <QPixmap>

struct TIFF_File
{
    QString filename;
    int dirIndex;
    quint32 width, height;
    QPixmap display;
    double ev_coeff;
};

class Data : public QObject
{
    Q_OBJECT
public:
    explicit Data(QObject *parent = 0);
signals:
public slots:
private:
    QList<TIFF_File> photos;
    bool configModified;
};

#endif // DATA_H
