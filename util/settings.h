#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

#define SETTINGS_MAX_RATIO 40

class Settings
{
public:
    static void init(QObject *parent = NULL);
    static void terminate();
    static inline bool contains(const QString &key);
    static inline void setValue(const QString &key, const QVariant &value);
    static inline QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
public: /* Application dependant settings: */
    static int getDefRatio();
    static void setDefRatio(int value);
    static bool getDefRatioMem();
    static void setDefRatioMem(bool value);
    static void workWithRatio(int value);
    static int getNumberOfThreads();
    static void setNumberOfThreads(int value);
    static int getThreads();
private:
    static QSettings *obj;
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

inline QVariant Settings::value(const QString &key, const QVariant &defaultValue)
{
    if (!obj)
        return defaultValue;
    return obj->value(key, defaultValue);
}

#endif // SETTINGS_H
