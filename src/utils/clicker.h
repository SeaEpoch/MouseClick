#ifndef CLICKER_H
#define CLICKER_H

#include <atomic>
#include <QMutex>
#include <QObject>
#include <QWaitCondition>

class Clicker : public QObject
{
    Q_OBJECT
public:
    explicit Clicker(QObject* parent = nullptr);
    ~Clicker();
    Clicker& initParameters(Qt::MouseButton btnType,
                            int interval,
                            bool randomIntervalFlag,
                            int maxRandomInterval);

public Q_SLOTS:
    void start();
    void stop();

private:
    std::atomic<bool> _run;
    Qt::MouseButton _btn_type;
    int _interval;
    bool _random_interval_flag;
    int _max_random_interval;

    QMutex _interrupt_sleep_immediately;
    QWaitCondition _sleep_timer;
    void msleep(int ms);
#if defined(Q_OS_WIN)
    void clickLoop(unsigned long flags);
#endif
};

#endif // CLICKER_H
