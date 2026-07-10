#include "clicker.h"

#include <map>
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
      _max_random_interval(20)
{}

Clicker::~Clicker()
{
}

Clicker& Clicker::initParameters(Qt::MouseButton btnType,
                                 int interval,
                                 bool randomIntervalFlag,
                                 int maxRandomInterval)
{
    _btn_type = btnType;
    _interval = interval;
    _random_interval_flag = randomIntervalFlag;
    _max_random_interval = maxRandomInterval;

    return (*this);
}

void Clicker::start()
{
    _run = true;

    static const std::map<Qt::MouseButton, DWORD> buttonFlags = {
        {Qt::LeftButton,   MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP},
        {Qt::RightButton,  MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP},
        {Qt::MiddleButton, MOUSEEVENTF_MIDDLEDOWN | MOUSEEVENTF_MIDDLEUP},
    };

    auto it = buttonFlags.find(_btn_type);
    if (it == buttonFlags.end()) {
        return;
    }

    clickLoop(it->second);
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
void Clicker::clickLoop(DWORD flags)
{
    POINT mouse_pos;

    while (_run) {
        GetCursorPos(&mouse_pos);
        mouse_event(flags, mouse_pos.x, mouse_pos.y, 0, 0);
        msleep(_random_interval_flag
                   ? QRandomGenerator::global()->bounded(0, _max_random_interval)
                   : _interval);
    }
}
#endif
