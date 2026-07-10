#ifndef BEAUTIFYCURSORPAGE_H
#define BEAUTIFYCURSORPAGE_H

#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QWidget>

#include "navpage.h"

class BeautifyCursorPage : public NavPage
{
    Q_OBJECT
public:
    explicit BeautifyCursorPage(const QString &title, QWidget *parent = nullptr);
    ~BeautifyCursorPage();

private:
    Q_DISABLE_COPY(BeautifyCursorPage)

    static QMap<Theme::ThemeMode, QString> _theme_files;
    QMap<QString, QJsonObject> _cursors;

    QString &getThemeFiles(Theme::ThemeMode theme) override;
    void loadCursorInfo(const QString &path);
    QJsonObject getConfigJsonObj(const QString &file_path);
};

class CursorListItem : public QWidget
{
    Q_OBJECT
public:
    explicit CursorListItem(const QString &image_path,
                            const QString &author_name,
                            const QString &description,
                            const QString &source_url,
                            QWidget *parent = nullptr);

    QPushButton* installButton() const { return _install_btn; }
    QPushButton* applyButton() const { return _apply_btn; }
    QPushButton* uninstallButton() const { return _uninstall_btn; }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QPushButton* _install_btn;
    QPushButton* _apply_btn;
    QPushButton* _uninstall_btn;
    QLabel* _author_label;
    QString _source_url;
    QTimer* _hover_timer;

    void openSourceUrl();
};

#endif // BEAUTIFYCURSORPAGE_H
