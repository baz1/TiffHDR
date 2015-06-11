#include "settings.h"

#include <QCoreApplication>

#define SETTINGS_THREADS "Threads"
#define SETTINGS_THREADS_DEFAULT 0

static QSettings Settings::obj;

void Settings::init(QObject *parent)
{
    if (!obj)
        obj = new QSettings(QCoreApplication::applicationDirPath() + QStringLiteral("/settings.ini"),
                            QSettings::IniFormat, parent);
}

void Settings::terminate()
{
    if (obj)
    {
        delete obj;
        obj = NULL;
    }
}

int Settings::getNumberOfThreads()
{
    return Settings::value(QStringLiteral(SETTINGS_THREADS), SETTINGS_THREADS_DEFAULT).toInt();
}

void Settings::setNumberOfThreads(int value)
{
    Q_ASSERT(value >= 0);
    Settings::setValue(QStringLiteral(SETTINGS_THREADS), value);
}

int Settings::getThreads()
{
    int nThreads = getNumberOfThreads();
    if (nThreads > 0)
    {
        /* Do not waste threads: */
        int cmpTemp = QThread::idealThreadCount();
        if ((cmpTemp > 0) && (cmpTemp < nThreads))
            nThreads = cmpTemp;
    } else {
        nThreads = QThread::idealThreadCount();
        if (nThreads < 1)
            nThreads = 1;
    }
}
