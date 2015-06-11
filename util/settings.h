#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings
{
public:
    static void init(QObject *parent = NULL);
    static void terminate();
    static inline bool contains(const QString &key);
    static inline void setValue(const QString &key, const QVariant &value);
    static inline QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
public: /* Application dependant settings: */
    static int getNumberOfThreads();
    static void setNumberOfThreads(int value);
    static int getThreads();
private:
    static QSettings *obj = NULL;
};

inline bool Settings::contains(const QString &key)
{
    return obj->contains(key);
}

inline void Settings::setValue(const QString &key, const QVariant &value)
{
    if (obj)
        obj->setValue(key, value);
}

inline QVariant Settings::value(const QString &key, const QVariant &defaultValue = QVariant())
{
    if (!obj)
        return defaultValue;
    return obj->value(key, defaultValue);
}

#endif // SETTINGS_H
