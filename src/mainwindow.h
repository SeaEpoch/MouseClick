#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "modules/shared.h"

#include <QMainWindow>
#include <QPushButton>

namespace QWK
{
    class WidgetWindowAgent;
}

template <class Key, class T> class QMap;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    bool event(QEvent *event) override;
    void changeEvent(QEvent *event) override;

signals:
    void windowStateChanged(Qt::WindowStates newState);

private:
    Q_DISABLE_COPY_MOVE(MainWindow)

    QWK::WidgetWindowAgent* _window_agent;
    static QMap<Theme::ThemeMode, QString> _theme_files;

    // 导航按钮（语言切换需要重新设置文本）
    QPushButton* _nav_mouse_click;
    QPushButton* _nav_beautify_cursor;
    QPushButton* _nav_settings;

    void windowInit(const QString& title, const QIcon& icon);
    void loadThemeStyelSheet(Theme::ThemeMode theme);
    void UIWidgetInit();
    void connectInit();
    void retranslateUi();
};

#endif // MAINWINDOW_H
