#include "clicker.h"

#include <QRandomGenerator>
#include <QThread>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

Clicker::Clicker(QObject* parent)
    : QObject{parent},
      _run(false),
      _btn_type(Qt::LeftButton),
      _interval(10),
      _random_interval_flag(false),
      _max_random_interval(20),
      _random_offset_flag(false),
      _max_random_offset(0)
{}

Clicker::~Clicker()
{
}

Clicker& Clicker::initParameters(Qt::MouseButton btnType,
                                 int interval,
                                 bool randomIntervalFlag,
                                 int maxRandomInterval,
                                 bool randomOffsetFlag,
                                 int maxRandomOffset)
{
    _btn_type = btnType;
    _interval = interval;
    _random_interval_flag = randomIntervalFlag;
    _max_random_interval = maxRandomInterval;
    _random_offset_flag = randomOffsetFlag;
    _max_random_offset = maxRandomOffset;

    return (*this);
}

void Clicker::start()
{
    _run = true;

    switch(_btn_type) {
        case Qt::LeftButton: {
            if (_random_interval_flag) {
                leftRandomClick();
            } else {
                leftClick();
            }
        }
        break;
        case Qt::RightButton: {
            if (_random_interval_flag) {
                rightRandomClick();
            } else {
                rightClick();
            }
        }
        break;
        case Qt::MiddleButton: {
            if (_random_interval_flag) {
                middleRandomClick();
            } else {
                middleClick();
            }
        }
        break;
        default:
            break;
    }
}

void Clicker::stop()
{
    _run = false;

    _sleep_timer.wakeAll();
}

void Clicker::msleep(int ms)
{
    _interrupt_sleep_immediately.lock();

    if (_run) {
        _sleep_timer.wait(&_interrupt_sleep_immediately, ms);
    }

    _interrupt_sleep_immediately.unlock();
}

#if defined(Q_OS_WIN)
POINT Clicker::randomClickPosition(const POINT& base_pos) const
{
    if (!_random_offset_flag || _max_random_offset <= 0) {
        return base_pos;
    }

    int dx = 0;
    int dy = 0;
    const int max_offset_squared = _max_random_offset * _max_random_offset;
    do {
        dx = QRandomGenerator::global()->bounded(-_max_random_offset, _max_random_offset + 1);
        dy = QRandomGenerator::global()->bounded(-_max_random_offset, _max_random_offset + 1);
    } while (dx * dx + dy * dy > max_offset_squared);

    POINT click_pos {base_pos.x + dx, base_pos.y + dy};

    const int min_x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    const int min_y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    const int max_x = min_x + GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1;
    const int max_y = min_y + GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1;

    click_pos.x = qBound(min_x, click_pos.x, max_x);
    click_pos.y = qBound(min_y, click_pos.y, max_y);

    return click_pos;
}

void Clicker::clickAtPosition(DWORD down_event, DWORD up_event, const POINT& base_pos, const POINT& click_pos) const
{
    SetCursorPos(click_pos.x, click_pos.y);
    mouse_event(down_event | up_event, click_pos.x, click_pos.y, 0, 0);

    if (click_pos.x != base_pos.x || click_pos.y != base_pos.y) {
        SetCursorPos(base_pos.x, base_pos.y);
    }
}

void Clicker::leftClick()
{
    POINT mouse_pos;

    while (_run) {
        GetCursorPos(&mouse_pos);
        clickAtPosition(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP, mouse_pos, randomClickPosition(mouse_pos));
        msleep(_interval);
    }
}

void Clicker::rightClick()
{
    POINT mouse_pos;

    while (_run) {
        GetCursorPos(&mouse_pos);
        clickAtPosition(MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP, mouse_pos, randomClickPosition(mouse_pos));
        msleep(_interval);
    }
}

void Clicker::middleClick()
{
    POINT mouse_pos;

    while (_run) {
        GetCursorPos(&mouse_pos);
        clickAtPosition(MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP, mouse_pos, randomClickPosition(mouse_pos));
        msleep(_interval);
    }
}

void Clicker::leftRandomClick()
{
    POINT mouse_pos;

    while (_run) {
        GetCursorPos(&mouse_pos);
        clickAtPosition(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP, mouse_pos, randomClickPosition(mouse_pos));
        msleep(QRandomGenerator::global()->bounded(0, _max_random_interval));
    }
}

void Clicker::rightRandomClick()
{
    POINT mouse_pos;

    while (_run) {
        GetCursorPos(&mouse_pos);
        clickAtPosition(MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP, mouse_pos, randomClickPosition(mouse_pos));
        msleep(QRandomGenerator::global()->bounded(0, _max_random_interval));
    }
}

void Clicker::middleRandomClick()
{
    POINT mouse_pos;

    while (_run) {
        GetCursorPos(&mouse_pos);
        clickAtPosition(MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP, mouse_pos, randomClickPosition(mouse_pos));
        msleep(QRandomGenerator::global()->bounded(0, _max_random_interval));
    }
}
#endif
