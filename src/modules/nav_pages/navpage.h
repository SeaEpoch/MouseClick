#ifndef NAVPAGE_H
#define NAVPAGE_H

#include <memory>
#include <QPushButton>
#include <QWidget>

#include "../shared.h"
#include "../hotkeylineedit.h"
#include "../../utils/clicker.h"

class NavPage : public QWidget
{
    Q_OBJECT
public:
    explicit NavPage(QWidget* parent = nullptr);
    ~NavPage();

    static Clicker* clicker();
    static QThread* clickerThread();

Q_SIGNALS:
    void ThemeChanged();

protected:
    static QMap<Theme::ThemeMode, QString> _theme_files;

    virtual QString& getThemeFiles(Theme::ThemeMode theme) = 0;
    void LoadThemeStyleSheet(Theme::ThemeMode theme);

private:
    static std::unique_ptr<Clicker> _clicker;
    static std::unique_ptr<QThread> _clicker_thread;
    static bool _is_thread_initialized;
};

#endif // NAVPAGE_H
