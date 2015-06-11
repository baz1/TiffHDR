#include "settings.h"

#include <QCoreApplication>

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
