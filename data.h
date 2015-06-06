#ifndef DATA_H
#define DATA_H

#include <QObject>
#include <QList>
#include <QString>

struct TIFF_File
{
    QString filename;
    int dirIndex;
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
