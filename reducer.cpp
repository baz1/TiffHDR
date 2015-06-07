#include "reducer.h"

Reducer::Reducer(int ratio, QObject *parent) : QThread(parent), ratio(ratio)
{
}

void Reducer::setFilename(QString filename)
{
    result = QPixmap();
    this->filename = filename;
}

void Reducer::run()
{
    // TODO
}
