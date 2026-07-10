#include "settingsagent.h"
#include "messagebox.h"
#include "translationmanager.h"
#include "../utils/logger.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QFile>
#include <QPushButton>
#include <QSettings>
#include <QTimer>
#include <QUrl>

SettingsAgent &SettingsAgent::instance()
{
    static SettingsAgent instance;
    return instance;
}

Theme::ThemeMode SettingsAgent::ThemeMode() const
{
    return static_cast<Theme::ThemeMode>(_config["ThemeMode"].toInt());
}

void SettingsAgent::setThemeMode(Theme::ThemeMode theme_mode)
{
    if (_config["ThemeMode"].toInt() != static_cast<int>(theme_mode)) {
        _config["ThemeMode"] = theme_mode;
        emit currentThemeChanged(theme_mode);
    }
}

Qt::WindowStates SettingsAgent::WindowState() const
{
    return static_cast<Qt::WindowStates>(_config["WindowState"].toInt());
}

void SettingsAgent::setWindowState(Qt::WindowStates window_state)
{
    _config["WindowState"] = window_state.toInt();
}

QString SettingsAgent::Hotkey() const
{
    return _config["Hotkey"].toString();
}

void SettingsAgent::setHotkey(const QString &hotkey)
{
    _config["Hotkey"] = hotkey;
}

QString SettingsAgent::Language() const
{
    return _config["Language"].toString();
}

void SettingsAgent::setLanguage(const QString& language)
{
    if (_config["Language"].toString() != language) {
        _config["Language"] = language;
        TranslationManager::instance().switchLanguage(language);
        emit currentLanguageChanged(language);
    }
}

int SettingsAgent::ClickType() const
{
    return _config["ClickType"].toInt();
}

void SettingsAgent::setClickType(const int type)
{
    _config["ClickType"] = type;
}

double SettingsAgent::IntervalTime() const
{
    return _config["IntervalTime"].toDouble();
}

void SettingsAgent::setIntervalTime(const double interval_time)
{
    _config["IntervalTime"] = interval_time;
}

bool SettingsAgent::EnableRandomInterval() const
{
    return _config["EnableRandomInterval"].toBool();
}

void SettingsAgent::setEnableRandomInterval(bool enable_random_interval)
{
    _config["EnableRandomInterval"] = enable_random_interval;
}

double SettingsAgent::RandomIntervalTime() const
{
    return _config["RandomIntervalTime"].toDouble();
}

void SettingsAgent::setRandomIntervalTime(const double random_interval_time)
{
    _config["RandomIntervalTime"] = random_interval_time;
}

bool SettingsAgent::EnableMemoryConfiguration() const
{
    return _config["EnableMemoryConfiguration"].toBool();
}

void SettingsAgent::setEnableMemoryConfiguration(bool memory_configuration)
{
    _config["EnableMemoryConfiguration"] = memory_configuration;
}

SettingsAgent::SettingsAgent(QObject *parent)
    : QObject{parent}
{
    _settings_file_path = QCoreApplication::applicationDirPath() + "/config.ini";
    QFile settings_file(_settings_file_path);
    if (!settings_file.exists()) {
        MessageBox file_missing_msg;
        file_missing_msg.setWindowIcon(QIcon(":/svg/favicon.svg"));
        file_missing_msg.setIcon(QMessageBox::Warning);
        file_missing_msg.setText(tr("WARNING"));
        file_missing_msg.setInformativeText(tr("The configuration file 'condig.ini' was not found. The program may have been modified. It is recommended to reinstall the program, which may resolve this issue."));

        QPushButton* ignore_btn = file_missing_msg.addButton(tr("Ignore"), QMessageBox::NoRole);
        QPushButton* reinstall_btn = file_missing_msg.addButton(tr("Reinstall"), QMessageBox::YesRole);
        file_missing_msg.setDefaultButton(ignore_btn);

        file_missing_msg.exec();

        if (file_missing_msg.clickedButton() == reinstall_btn) {
            QDesktopServices::openUrl(QUrl("https://github.com/SeaEpoch/MouseClick"));
        } else if (file_missing_msg.clickedButton() == ignore_btn) {
            // nothing to do
        } else {
            // nothing to do
        }
    }

    QSettings settings(_settings_file_path, QSettings::IniFormat);

    // 读取 MainWindow 配置
    settings.beginGroup("MainWindow");
    _config["ThemeMode"] = settings.value("ThemeMode", _DEFAULT_THEMEMODE);
    _config["WindowState"] = settings.value("WindowState", static_cast<int>(_DEFAULT_WINDOWSTATE));
    settings.endGroup();

    // 读取 Settings 配置
    settings.beginGroup("Settings");
    _config["Language"] = settings.value("Language", _DEFAULT_LANGUAGE);
    _config["Hotkey"] = settings.value("Hotkey", _DEFAULT_HOTKEY);
    settings.endGroup();

    // 读取 Configuration 配置
    settings.beginGroup("Configuration");
    _config["ClickType"] = settings.value("ClickType", _DEFAULT_CLICKTYPE);
    _config["IntervalTime"] = settings.value("IntervalTime", _DEFAULT_INTERVALTIME);
    _config["EnableRandomInterval"] = settings.value("EnableRandomInterval", _DEFAULT_RANDOMINTERVAL);
    _config["RandomIntervalTime"] = settings.value("RandomIntervalTime", _DEFAULT_RANDOMINTERVALTIME);
    _config["EnableMemoryConfiguration"] = settings.value("EnableMemoryConfiguration", _DEFAULT_MEMORYCONFIGURATION);
    settings.endGroup();

    // 解决可能出现的 @Invalid() 值问题
    auto ensureValid = [this](const QString& key, const QVariant& defaultValue) {
        if (!_config[key].isValid() || _config[key].metaType().id() == QMetaType::UnknownType) {
            _config[key] = defaultValue;
        }
    };
    ensureValid("ThemeMode",                _DEFAULT_THEMEMODE);
    ensureValid("WindowState",              static_cast<int>(_DEFAULT_WINDOWSTATE));
    ensureValid("Language",                 _DEFAULT_LANGUAGE);
    ensureValid("Hotkey",                   _DEFAULT_HOTKEY);
    ensureValid("ClickType",                _DEFAULT_CLICKTYPE);
    ensureValid("IntervalTime",             _DEFAULT_INTERVALTIME);
    ensureValid("EnableRandomInterval",     _DEFAULT_RANDOMINTERVAL);
    ensureValid("RandomIntervalTime",       _DEFAULT_RANDOMINTERVALTIME);
    ensureValid("EnableMemoryConfiguration", _DEFAULT_MEMORYCONFIGURATION);

    // 主题的特殊检查
    if (!Theme::isValidThemeMode(_config["ThemeMode"].toInt())) {
        _config["ThemeMode"] = _DEFAULT_THEMEMODE;
        QTimer::singleShot(0, this, [this](){   // 延迟发送，确保在对象初始化完成后发送信号
            emit currentThemeChanged(_DEFAULT_THEMEMODE);
        });
    }

    // 语言的特殊检查（必须确保可用）
    const QString language = _config["Language"].toString();
    if (!_language_support_list.contains(language)) {
        _config["Language"] = _DEFAULT_LANGUAGE;
        QTimer::singleShot(0, this, [this]() {  // 延迟发送，确保在对象初始化完成后发送信号
            emit currentLanguageChanged(_DEFAULT_LANGUAGE);
        });
    }

    // 不需要记忆点击配置
    if (!_config["EnableMemoryConfiguration"].toBool()) {
        _config["ClickType"] = _DEFAULT_CLICKTYPE;
        _config["IntervalTime"] = _DEFAULT_INTERVALTIME;
        _config["EnableRandomInterval"] = _DEFAULT_RANDOMINTERVAL;
        _config["RandomIntervalTime"] = _DEFAULT_RANDOMINTERVALTIME;
    }
}

SettingsAgent::~SettingsAgent()
{
    QSettings settings(_settings_file_path, QSettings::IniFormat);

    settings.beginGroup("MainWindow");
    settings.setValue("ThemeMode", _config["ThemeMode"]);
    settings.setValue("WindowState", _config["WindowState"]);
    settings.endGroup();

    settings.beginGroup("Settings");
    settings.setValue("Language", _config["Language"]);
    settings.setValue("Hotkey", _config["Hotkey"]);
    settings.endGroup();

    settings.beginGroup("Configuration");
    settings.setValue("ClickType", _config["ClickType"]);
    settings.setValue("IntervalTime", _config["IntervalTime"]);
    settings.setValue("EnableRandomInterval", _config["EnableRandomInterval"]);
    settings.setValue("RandomIntervalTime", _config["RandomIntervalTime"]);
    settings.setValue("EnableMemoryConfiguration", _config["EnableMemoryConfiguration"]);
    settings.endGroup();
}
