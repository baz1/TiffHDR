#include "settings.h"

#include <QCoreApplication>
#include <QThread>

#define SETTINGS_FILENAME "/settings.ini"

#define SETTINGS_THREADS "Threads"
#define SETTINGS_THREADS_DEFAULT 0

#define SETTINGS_DEFRATIO "DefRatio"
#define SETTINGS_DEFRATIO_DEFAULT 1

#define SETTINGS_DEFRATIO_MEM "DefRatioMem"
#define SETTINGS_DEFRATIO_MEM_DEFAULT true

QSettings *Settings::obj = NULL;

void Settings::init(QObject *parent)
{
    if (!obj)
        obj = new QSettings(QCoreApplication::applicationDirPath() + QStringLiteral(SETTINGS_FILENAME),
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

int Settings::getDefRatio()
{
    int defratio = Settings::value(QStringLiteral(SETTINGS_DEFRATIO), SETTINGS_DEFRATIO_DEFAULT).toInt();
    if (defratio < 1)
        defratio = SETTINGS_DEFRATIO_DEFAULT;
    else if (defratio > SETTINGS_MAX_RATIO)
        defratio = SETTINGS_MAX_RATIO;
    return defratio;
}

void Settings::setDefRatio(int value)
{
    Q_ASSERT((value > 0) && (value <= 256));
    Settings::setValue(QStringLiteral(SETTINGS_DEFRATIO), value);
}

bool Settings::getDefRatioMem()
{
    return Settings::value(QStringLiteral(SETTINGS_DEFRATIO_MEM), SETTINGS_DEFRATIO_MEM_DEFAULT).toBool();
}

void Settings::setDefRatioMem(bool value)
{
    Settings::setValue(QStringLiteral(SETTINGS_DEFRATIO_MEM), value);
}

void Settings::workWithRatio(int value)
{
    if (Settings::getDefRatioMem())
        Settings::setDefRatio(value);
}

int Settings::getNumberOfThreads()
{
    int threads = Settings::value(QStringLiteral(SETTINGS_THREADS), SETTINGS_THREADS_DEFAULT).toInt();
    if (threads < 0)
        threads = SETTINGS_THREADS_DEFAULT;
    return SETTINGS_THREADS_DEFAULT;
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
    return nThreads;
}
