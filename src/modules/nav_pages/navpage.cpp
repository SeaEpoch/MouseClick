#include "navpage.h"
#include "../shared.h"
#include "../settingsagent.h"

#include <QFile>
#include <QThread>

QMap<Theme::ThemeMode, QString> NavPage::_theme_files {};

std::unique_ptr<Clicker> NavPage::_clicker = std::make_unique<Clicker>();
std::unique_ptr<QThread> NavPage::_clicker_thread = std::make_unique<QThread>();
bool NavPage::_is_thread_initialized = false;

Clicker* NavPage::clicker()
{
    return _clicker.get();
}

QThread* NavPage::clickerThread()
{
    return _clicker_thread.get();
}

NavPage::NavPage(QWidget* parent)
    : QWidget{parent}
{
    disconnect(&SettingsAgent::instance(), &SettingsAgent::currentThemeChanged,
                this, &NavPage::LoadThemeStyleSheet);
    connect(&SettingsAgent::instance(), &SettingsAgent::currentThemeChanged,
            this, &NavPage::LoadThemeStyleSheet);

    if (!_is_thread_initialized) {
        _clicker->moveToThread(_clicker_thread.get());
        connect(_clicker_thread.get(), &QThread::started,
                _clicker.get(), &Clicker::start);
        _is_thread_initialized = true;
    }
}

NavPage::~NavPage()
{
}

void NavPage::LoadThemeStyleSheet(Theme::ThemeMode theme)
{
    QFile style_file(getThemeFiles(theme));
    if (style_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setStyleSheet(QString::fromUtf8(style_file.readAll()));
    }
}
