#ifndef MOUSECLICKPAGE_H
#define MOUSECLICKPAGE_H

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QRadioButton>
#include <QWidget>

#include "navpage.h"
#include "settingspage.h"

class MouseClickPage : public NavPage
{
    Q_OBJECT
public:
    explicit MouseClickPage(const QString& title, SettingsPage& settings_page, QWidget* parent = nullptr);
    ~MouseClickPage();

protected:
    void changeEvent(QEvent *event) override;

private:
    Q_DISABLE_COPY(MouseClickPage)

    static QMap<Theme::ThemeMode, QString> _theme_files;

    // 可翻译控件
    QLabel* _page_title;
    QLabel* _click_type_desc;
    QComboBox* _click_type_list;
    QLabel* _interval_time_desc;
    QLabel* _random_interval_toggle_desc;
    QLabel* _random_interval_time_desc;
    QLabel* _memory_configuration_toggle_desc;

    QString& getThemeFiles(Theme::ThemeMode theme) override;
    void retranslateUi();
};

#endif // MOUSECLICKPAGE_H
