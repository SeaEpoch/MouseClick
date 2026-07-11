#include "traymenu.h"

#include <QPainter>
#include <QPainterPath>

TrayMenu::TrayMenu(QWidget* parent)
    : QMenu(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // QSS 仅用于设置 padding / margin 等几何属性（actionGeometry 依赖），
    // 背景、文字颜色、选中状态由 paintEvent 自绘，保证圆角与主题一致。
    setStyleSheet(QStringLiteral("TrayMenu { padding: 4px 2px; }"
                                 "TrayMenu::item { padding: 4px 12px; }"
                                 "TrayMenu::separator { height: 1px; margin: 4px 8px; }"));
}

void TrayMenu::setDarkMode(bool dark)
{
    _is_dark = dark;
    update();
}

void TrayMenu::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // === 颜色定义 ===
    const QColor bgColor      = _is_dark ? QColor("#333333") : QColor("#ffffff");
    const QColor borderColor  = _is_dark ? QColor("#555555") : QColor("#e0e0e0");
    const QColor textColor    = _is_dark ? QColor("#fafafa") : QColor("#1e1e1e");
    const QColor sepColor     = _is_dark ? QColor("#555555") : QColor("#e0e0e0");
    const QColor selBg        = QColor("#3AA0F5");
    const QColor selText      = QColor("#fafafa");

    const int menuRadius = 6;
    const int itemRadius = 4;
    const int itemPadHorizontal = 12; // item 左右内边距（与 QSS padding: 4px 12px 一致）
    const int separatorMargin = 8;    // 分隔线左右边距

    // === 背景 + 边框 ===
    p.setBrush(bgColor);
    p.setPen(QPen(borderColor, 1));
    p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), menuRadius, menuRadius);

    // === 裁剪 ===
    QPainterPath clip;
    clip.addRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), menuRadius - 1, menuRadius - 1);
    p.setClipPath(clip);

    // === 遍历 action ===
    const QList<QAction*> acts = actions();
    for (int i = 0; i < acts.count(); ++i) {
        QAction* action = acts.at(i);
        QRect r = actionGeometry(action);

        if (action->isSeparator()) {
            int y = r.center().y();
            p.setPen(QPen(sepColor, 1));
            p.drawLine(r.left() + separatorMargin, y, r.right() - separatorMargin, y);
            continue;
        }

        bool selected = (action == activeAction()) && action->isEnabled();
        QColor currentText = selected ? selText : textColor;

        if (selected) {
            p.setPen(Qt::NoPen);
            p.setBrush(selBg);
            QRect selRect = r.adjusted(3, 1, -3, -1);
            p.drawRoundedRect(QRectF(selRect), itemRadius, itemRadius);
        }

        p.setPen(currentText);
        QFont f = font();
        f.setPixelSize(12);
        p.setFont(f);

        QRect textRect = r.adjusted(itemPadHorizontal, 0, -itemPadHorizontal, 0);
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, action->text());
    }

    p.end();
}
