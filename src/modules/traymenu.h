#ifndef TRAYMENU_H
#define TRAYMENU_H

#include <QMenu>

class TrayMenu : public QMenu
{
    Q_OBJECT
public:
    explicit TrayMenu(QWidget* parent = nullptr);

    void setDarkMode(bool dark);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    bool _is_dark = false;
};

#endif // TRAYMENU_H
