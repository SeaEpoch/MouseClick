#include "hotkeylineedit.h"
#include "../utils/logger.h"

#include <QWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QKeySequence>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QHotkey>

HotkeyLineEdit::HotkeyLineEdit(QWidget* parent)
    : QLineEdit{parent},
      _hotkey(new QHotkey())
{
    setReadOnly(true);
    setPlaceholderText(tr("Please set a shortcut hotkey"));
    connect(_hotkey, &QHotkey::activated, this, &HotkeyLineEdit::hotkeyActivated);
}

HotkeyLineEdit::~HotkeyLineEdit()
{
    unregisterGlobalHotkey();

    delete _hotkey;
}

void HotkeyLineEdit::cleanHotKey()
{
    clear();
    unregisterGlobalHotkey();
}

const QString HotkeyLineEdit::getHotkey() const
{
    return _key_sequence;
}

void HotkeyLineEdit::setHotkey(const QString& key_sequence)
{
    _key_sequence = key_sequence;

    setText(_key_sequence);

    if (!_key_sequence.isEmpty()) {
        registerGlobalHotkey();
    }
}

void HotkeyLineEdit::mousePressEvent(QMouseEvent* event)
{
    if (!hasFocus()) {
        setFocus();
    }
    QLineEdit::mousePressEvent(event);
}

void HotkeyLineEdit::focusInEvent(QFocusEvent* event)
{
    // 清空之前的内容
    cleanHotKey();
    // 设置焦点时监听键盘事件
    grabKeyboard();
    QLineEdit::focusInEvent(event);
}

void HotkeyLineEdit::focusOutEvent(QFocusEvent* event)
{
    // 失去焦点时取消监听
    releaseKeyboard();
    QLineEdit::focusOutEvent(event);
}

void HotkeyLineEdit::keyPressEvent(QKeyEvent* event)
{
    int key = event->key(); // 获取按键编码
    Qt::KeyboardModifiers modifiers = event->modifiers();

    if (key != Qt::Key_unknown && key != Qt::Key_Control && key != Qt::Key_Shift &&
        key != Qt::Key_Alt && key != Qt::Key_Meta) {
        _key_sequence = QKeySequence(modifiers | key).toString();
    } else {
        _key_sequence = QKeySequence(modifiers).toString();
    }

    setText(_key_sequence);

    _pressed_keys.insert(key);

    QLineEdit::keyPressEvent(event);
}

void HotkeyLineEdit::keyReleaseEvent(QKeyEvent* event)
{
    int key = event->key();

    _pressed_keys.remove(key);

    if (_pressed_keys.isEmpty()) {
        clearFocus();

        registerGlobalHotkey();
    }

    QLineEdit::keyReleaseEvent(event);
}

void HotkeyLineEdit::registerGlobalHotkey()
{
#ifdef QT_DEBUG
    LOG_DEBUG(QString("Register Global Hotkey: %1").arg(_key_sequence));
#endif

    unregisterGlobalHotkey(); // 先注销之前的快捷键

    QStringList keys = _key_sequence.split('+');
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    int key = 0;

    for (const QString& k : keys) {
        if (k == "Ctrl") {
            modifiers |= Qt::ControlModifier;
        } else if (k == "Shift") {
            modifiers |= Qt::ShiftModifier;
        } else if (k == "Alt") {
            modifiers |= Qt::AltModifier;
        } else if (k == "Meta") {
            modifiers |= Qt::MetaModifier;
        } else {
            QKeySequence keySequence(k, QKeySequence::PortableText);
            if (!keySequence.isEmpty()) {
                key = keySequence[0].key();
            }
        }
    }

    if (key != 0) {
        if (!_hotkey->setShortcut(QKeySequence(modifiers | key), true)) {
            // 设置失败
            QMessageBox msgBox(QMessageBox::Critical, tr("ERROR"), tr("Failed to register global hotkey."), QMessageBox::NoButton, this);
            QPushButton* okButton = msgBox.addButton(tr("OK"), QMessageBox::ActionRole);
            okButton->setStyleSheet("QPushButton{width: 60px;}");
            msgBox.exec();

            // 清空
            _key_sequence.clear();
            clear();
        } else {
            emit currentHotkeyChanged(_key_sequence);   // 发送信号，快捷键已更改
        }
    }
}

void HotkeyLineEdit::unregisterGlobalHotkey()
{
    _hotkey->setShortcut(QKeySequence(), true);
}
