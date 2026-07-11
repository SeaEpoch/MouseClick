#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "modules/shared.h"

#include <QMainWindow>
#include <QMenu>
#include <QPushButton>
#include <QSystemTrayIcon>

namespace QWK
{
    class WidgetWindowAgent;
}

template <class Key, class T> class QMap;

class QStackedWidget;
class SettingsPage;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    bool event(QEvent *event) override;
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

signals:
    void windowStateChanged(Qt::WindowStates newState);

private:
    Q_DISABLE_COPY_MOVE(MainWindow)

    QWK::WidgetWindowAgent* _window_agent;
    static QMap<Theme::ThemeMode, QString> _theme_files;

    SettingsPage* _settings_page = nullptr;
    QStackedWidget* _navigation_pages = nullptr;

    // 系统托盘
    QSystemTrayIcon* _tray_icon = nullptr;
    QMenu* _tray_menu = nullptr;
    QAction* _tray_open_action = nullptr;
    QAction* _tray_website_action = nullptr;
    QAction* _tray_exit_action = nullptr;
    bool _force_quit = false;
    bool _was_maximized_before_tray = false;
    bool _was_hidden_before_clicker = false;

    // 导航按钮（语言切换需要重新设置文本）
    QPushButton* _nav_mouse_click;
    QPushButton* _nav_beautify_cursor;
    QPushButton* _nav_settings;

    void windowInit(const QString& title, const QIcon& icon);
    void loadThemeStyelSheet(Theme::ThemeMode theme);
    void UIWidgetInit();
    void connectInit();
    void setupSystemTray();
    void retranslateUi();
};

#endif // MAINWINDOW_H
