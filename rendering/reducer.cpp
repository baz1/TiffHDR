#include "reducer.h"

#include <QDebug>

Reducer::Reducer(int ratio, int threadId, QObject *parent) : QThread(parent), threadId(threadId), ratio(ratio), taskId(-1)
{
}

void Reducer::setTask(int taskId, QString filename, int dirIndex)
{
    result = QPixmap();
    this->taskId = taskId;
    this->filename = filename;
    this->dirIndex = dirIndex;
}

void Reducer::run()
{
    /* Simulating display */
    qsrand(threadId);
    int stepms = 20 + (qrand() % 50);
    for (int i = 1; i <= 100; ++i)
    {
        msleep(stepms);
        emit renderingStatus(threadId, i);
    }
    // TODO: actual calculation
}
