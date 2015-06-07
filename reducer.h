#ifndef REDUCER_H
#define REDUCER_H

#include <QThread>
#include <QPixmap>

class Reducer : public QThread
{
    Q_OBJECT
public:
    explicit Reducer(int ratio, QObject *parent = 0);
    void setFilename(QString filename);
    inline QPixmap getPixmap() const;
protected:
    void run();
signals:
    void renderingStatus(int value);
private:
    int ratio;
    QString filename;
    QPixmap result;
};

inline QPixmap Reducer::getPixmap() const
{
    return result;
}

#endif // REDUCER_H
