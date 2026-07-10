#ifndef SETTINGSAGENT_H
#define SETTINGSAGENT_H

#include "shared.h"
#include <QFile>
#include <QMap>
#include <QObject>
#include <QSettings>
#include <QTranslator>

class SettingsAgent : public QObject
{
    Q_OBJECT
public:
    // 接口
    static SettingsAgent& instance();

    void initTranslator();

    /***** 相关的 getter 与 settter *****/
    Theme::ThemeMode ThemeMode() const;
    void setThemeMode(Theme::ThemeMode& theme_mode);

    Qt::WindowStates WindowState() const;
    void setWindowState(Qt::WindowStates window_state);

    QString Hotkey() const;
    void setHotkey(const QString& hotkey);

    QString Language() const;
    void setLanguage(const QString& language);

    // ------------- //

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
    /***********************************/

signals:
    void currentThemeChanged(Theme::ThemeMode current_theme);
    void currentLanguageChanged(const QString current_language);

private:
    explicit SettingsAgent(QObject *parent = nullptr);
    ~SettingsAgent();
    Q_DISABLE_COPY(SettingsAgent);

    // 所有配置以键值对形式存储
    QMap<QString, QVariant> _config;

    // 翻译器（运行时语言热切换）
    QTranslator* _translator;
    bool _translator_loaded;
    void loadLanguageFile(const QString& language);

    // 配置文件完整文件路径
    QString _settings_file_path;

    // 支持语言列表
    const QStringList _language_support_list = {
        "en_US",
        "zh_CN",
        "zh_TW"
    };

    // 默认值
    const Theme::ThemeMode _DEFAULT_THEMEMODE = Theme::Light;   // 默认主题设置
    const Qt::WindowStates _DEFAULT_WINDOWSTATE = Qt::WindowNoState;
    const QString _DEFAULT_LANGUAGE = "en_US";                  // 默认语言设置
    const QString _DEFAULT_HOTKEY = "Ctrl+F2";
    const int _DEFAULT_CLICKTYPE = 0;
    const double _DEFAULT_INTERVALTIME = 0.01;
    const bool _DEFAULT_RANDOMINTERVAL = false;
    const double _DEFAULT_RANDOMINTERVALTIME = 0.02;
    const bool _DEFAULT_MEMORYCONFIGURATION = false;
};

#endif // SETTINGSAGENT_H
