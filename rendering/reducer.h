#ifndef REDUCER_H
#define REDUCER_H

#include <QThread>
#include <QPixmap>

class Reducer : public QThread
{
    Q_OBJECT
public:
    explicit Reducer(int ratio, int threadId = 0, QObject *parent = 0);
    void setTask(int taskId, QString filename, int dirIndex);
    inline int getTaskId() const;
    inline QPixmap getPixmap() const;
protected:
    void run();
signals:
    void renderingStatus(int threadId, int value);
private:
    void updateProgress(int &lastP, unsigned int &current, unsigned int &max);
private:
    int threadId, ratio;
    int taskId;
    QString filename;
    int dirIndex;
    QPixmap result;
};

inline int Reducer::getTaskId() const
{
    return taskId;
}

inline QPixmap Reducer::getPixmap() const
{
    return result;
}

#endif // REDUCER_H
