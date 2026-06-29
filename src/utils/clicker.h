#ifndef CLICKER_H
#define CLICKER_H

#include <QMutex>
#include <QObject>
#include <QWaitCondition>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

class Clicker : public QObject
{
    Q_OBJECT
public:
    explicit Clicker(QObject* parent = nullptr);
    ~Clicker();
    Clicker& initParameters(Qt::MouseButton btnType,
                            int interval,
                            bool randomIntervalFlag,
                            int maxRandomInterval,
                            bool randomOffsetFlag,
                            int maxRandomOffset);

public Q_SLOTS:
    void start();
    void stop();

private:
    bool _run;
    Qt::MouseButton _btn_type;
    int _interval;
    bool _random_interval_flag;
    int _max_random_interval;
    bool _random_offset_flag;
    int _max_random_offset;

    QMutex _interrupt_sleep_immediately;
    QWaitCondition _sleep_timer;
    void msleep(int ms);

    void leftClick();
    void rightClick();
    void middleClick();

    void leftRandomClick();
    void rightRandomClick();
    void middleRandomClick();

#if defined(Q_OS_WIN)
    POINT randomClickPosition(const POINT& base_pos) const;
    void clickAtPosition(DWORD down_event, DWORD up_event, const POINT& base_pos, const POINT& click_pos) const;
#endif
};

#endif // CLICKER_H
