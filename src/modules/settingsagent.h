#ifndef SETTINGSAGENT_H
#define SETTINGSAGENT_H

#include "shared.h"
#include <QFile>
#include <QMap>
#include <QObject>
#include <QSettings>

class SettingsAgent : public QObject
{
    Q_OBJECT
public:
    static SettingsAgent& instance();

    Theme::ThemeMode ThemeMode() const;
    void setThemeMode(Theme::ThemeMode theme_mode);

    Qt::WindowStates WindowState() const;
    void setWindowState(Qt::WindowStates window_state);

    QString Hotkey() const;
    void setHotkey(const QString& hotkey);

    QString Language() const;
    void setLanguage(const QString& language);

    int ClickType() const;
    void setClickType(const int type);

    double IntervalTime() const;
    void setIntervalTime(const double interval_time);

    bool EnableRandomInterval() const;
    void setEnableRandomInterval(bool enable_random_interval);

    double RandomIntervalTime() const;
    void setRandomIntervalTime(const double random_interval_time);

    bool EnableMemoryConfiguration() const;
    void setEnableMemoryConfiguration(bool memory_configuration);

    QString CloseButtonBehavior() const;
    void setCloseButtonBehavior(const QString& behavior);

signals:
    void currentThemeChanged(Theme::ThemeMode current_theme);
    void currentLanguageChanged(const QString current_language);

private:
    explicit SettingsAgent(QObject *parent = nullptr);
    ~SettingsAgent();
    Q_DISABLE_COPY_MOVE(SettingsAgent);

    QMap<QString, QVariant> _config;
    QString _settings_file_path;

    const QStringList _language_support_list = {
        "en_US",
        "zh_CN",
        "zh_TW"
    };

    const Theme::ThemeMode _DEFAULT_THEMEMODE = Theme::Light;
    const Qt::WindowStates _DEFAULT_WINDOWSTATE = Qt::WindowNoState;
    const QString _DEFAULT_LANGUAGE = "en_US";
    const QString _DEFAULT_HOTKEY = "Ctrl+F2";
    const int _DEFAULT_CLICKTYPE = 0;
    const double _DEFAULT_INTERVALTIME = 0.01;
    const bool _DEFAULT_RANDOMINTERVAL = false;
    const double _DEFAULT_RANDOMINTERVALTIME = 0.02;
    const bool _DEFAULT_MEMORYCONFIGURATION = false;
    const QString _DEFAULT_CLOSEBUTTONBEHAVIOR = "exit";
};

#endif // SETTINGSAGENT_H
