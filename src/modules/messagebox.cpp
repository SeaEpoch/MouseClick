#include "messagebox.h"

#include <QFile>
#include <QMap>

MessageBox::MessageBox(QWidget* parent)
    : QMessageBox{parent}
{
    QFile style_file(":/qss/modules/messagebox.qss");

    if (style_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        _base_styleSheet = QString::fromUtf8(style_file.readAll());
        setStyleSheet(_base_styleSheet);
    }
}

MessageBox::~MessageBox() {}

void MessageBox::setIcon(Icon icon)
{
    static const QMap<Icon, QString> iconColors = {
        {QMessageBox::Information, "#0099CC"},
        {QMessageBox::Warning,     "#E6A23C"},
        {QMessageBox::Critical,    "#F56C6C"},
        {QMessageBox::Question,    "#409EFF"},
    };
    setStyleSheet(_base_styleSheet +
        QString(" QMessageBox QLabel#qt_msgbox_label { color: %1; }")
            .arg(iconColors.value(icon)));
    QMessageBox::setIcon(icon);
}
