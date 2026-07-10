#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QComboBox>
#include <QLabel>
#include <QWidget>

#include "navpage.h"

class SettingsPage : public NavPage
{
    Q_OBJECT
public:
    explicit SettingsPage(const QString& title, QWidget* parent = nullptr);
    ~SettingsPage();

signals:
    void hotkeyActivated();

protected:
    void changeEvent(QEvent *event) override;

private:
    Q_DISABLE_COPY_MOVE(SettingsPage)

    static QMap<Theme::ThemeMode, QString> _theme_files;
    HotkeyLineEdit* _hotkey_reader;
    QPushButton* _hotkey_clean;

    // 可翻译控件
    QLabel* _page_title;
    QLabel* _hotkey_desc;
    QLabel* _theme_toggle_desc;
    QLabel* _language_switch_desc;
    QComboBox* _language_list;

    QString& getThemeFiles(Theme::ThemeMode theme) override;
    void retranslateUi();
};

#endif // SETTINGSPAGE_H
