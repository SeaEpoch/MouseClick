#include "mouseclickpage.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QFile>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QThread>

#include "../../utils/clicker.h"
#include "../settingsagent.h"

QMap<Theme::ThemeMode, QString> MouseClickPage::_theme_files {
    {Theme::Light, (":/qss/modules/light-mouseclickpage.qss")},
    {Theme::Dark, (":/qss/modules/dark-mouseclickpage.qss")}
};

MouseClickPage::MouseClickPage(const QString& title, SettingsPage& settings_page, QWidget* parent)
    : NavPage{parent}
{
    SettingsAgent& app_settings = SettingsAgent::instance();

    LoadThemeStyleSheet(app_settings.ThemeMode());

    QVBoxLayout* central_layout = new QVBoxLayout(this);
    central_layout->setSpacing(0);
    central_layout->setContentsMargins(QMargins());

    _page_title = new QLabel(this);
    _page_title->setObjectName(QStringLiteral("page-title"));
    _page_title->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _page_title->setFocusPolicy(Qt::NoFocus);
    _page_title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _page_title->setText(title);
    _page_title->setMaximumHeight(36);

    QWidget* page_content = new QWidget(this);
    page_content->setObjectName(QStringLiteral("page-content"));
    page_content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QVBoxLayout* page_content_layout = new QVBoxLayout(page_content);
    page_content_layout->setSpacing(12);
    page_content_layout->setContentsMargins(QMargins(0, 8, 0, 8));

    const int pageContentUniformHeight = 32;    // 统一控件高度

    /********************/

    QWidget* click_type_content = new QWidget(page_content);
    click_type_content->setFixedHeight(pageContentUniformHeight);

    QHBoxLayout* click_type_content_layout = new QHBoxLayout(click_type_content);
    click_type_content_layout->setSpacing(0);
    click_type_content_layout->setContentsMargins(QMargins());

    _click_type_desc = new QLabel(click_type_content);
    _click_type_desc->setObjectName(QStringLiteral("click-type-desc"));
    _click_type_desc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _click_type_desc->setFocusPolicy(Qt::NoFocus);
    _click_type_desc->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _click_type_desc->setText(tr("Click Type"));

    _click_type_list = new QComboBox(click_type_content);
    _click_type_list->setObjectName(QStringLiteral("click-type-list"));
    _click_type_list->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _click_type_list->addItem(QIcon(":/svg/mouse-left.svg"), tr("Left Mouse Button"));
    _click_type_list->addItem(QIcon(":/svg/mouse-right.svg"), tr("Right Mouse Button"));
    _click_type_list->addItem(QIcon(":/svg/mouse-middle.svg"), tr("Middle Mouse Button"));
    _click_type_list->setCurrentIndex(app_settings.ClickType());

    click_type_content_layout->addWidget(_click_type_desc);
    click_type_content_layout->addWidget(_click_type_list);
    click_type_content->setLayout(click_type_content_layout);

    /********************/

    QWidget* interval_time_content = new QWidget(page_content);
    interval_time_content->setFixedHeight(pageContentUniformHeight);

    QHBoxLayout* interval_time_content_layout = new QHBoxLayout(interval_time_content);
    interval_time_content_layout->setSpacing(0);
    interval_time_content_layout->setContentsMargins(QMargins());

    _interval_time_desc = new QLabel(interval_time_content);
    _interval_time_desc->setObjectName(QStringLiteral("interval-time-desc"));
    _interval_time_desc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _interval_time_desc->setFocusPolicy(Qt::NoFocus);
    _interval_time_desc->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _interval_time_desc->setText(tr("Interval Time"));

    QDoubleSpinBox* interval_time = new QDoubleSpinBox(interval_time_content);
    interval_time->setObjectName(QStringLiteral("interval-time"));
    interval_time->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    interval_time->setDecimals(2);
    interval_time->setMaximum(100.00);
    interval_time->setMinimum(0.01);
    interval_time->setSingleStep(0.01);
    interval_time->setValue(app_settings.IntervalTime());

    interval_time_content_layout->addWidget(_interval_time_desc);
    interval_time_content_layout->addWidget(interval_time);
    interval_time_content->setLayout(interval_time_content_layout);

    /********************/

    QWidget* random_interval_toggle_content = new QWidget(page_content);
    random_interval_toggle_content->setFixedHeight(pageContentUniformHeight);

    QHBoxLayout* random_interval_toggle_content_layout = new QHBoxLayout(random_interval_toggle_content);
    random_interval_toggle_content_layout->setSpacing(0);
    random_interval_toggle_content_layout->setContentsMargins(QMargins());

    _random_interval_toggle_desc = new QLabel(random_interval_toggle_content);
    _random_interval_toggle_desc->setObjectName(QStringLiteral("random-interval-toggle-desc"));
    _random_interval_toggle_desc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _random_interval_toggle_desc->setFocusPolicy(Qt::NoFocus);
    _random_interval_toggle_desc->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _random_interval_toggle_desc->setText(tr("Random Start Interval"));

    QRadioButton* random_interval_toggle_btn = new QRadioButton(random_interval_toggle_content);
    random_interval_toggle_btn->setObjectName(QStringLiteral("random-interval-toggle-btn"));
    random_interval_toggle_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    random_interval_toggle_btn->setChecked(app_settings.EnableRandomInterval());

    random_interval_toggle_content_layout->addWidget(_random_interval_toggle_desc);
    random_interval_toggle_content_layout->addWidget(random_interval_toggle_btn);
    random_interval_toggle_content->setLayout(random_interval_toggle_content_layout);

    /********************/

    QWidget* random_interval_time_content = new QWidget(page_content);
    random_interval_time_content->setFixedHeight(pageContentUniformHeight);

    QHBoxLayout* random_interval_time_content_layout = new QHBoxLayout(random_interval_time_content);
    random_interval_time_content_layout->setSpacing(0);
    random_interval_time_content_layout->setContentsMargins(QMargins());

    _random_interval_time_desc = new QLabel(random_interval_time_content);
    _random_interval_time_desc->setObjectName(QStringLiteral("random-interval-time-desc"));
    _random_interval_time_desc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _random_interval_time_desc->setFocusPolicy(Qt::NoFocus);
    _random_interval_time_desc->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _random_interval_time_desc->setText(tr("Set Max Random Interval"));

    QDoubleSpinBox* random_interval_time = new QDoubleSpinBox(random_interval_time_content);
    random_interval_time->setObjectName(QStringLiteral("random-interval-time"));
    random_interval_time->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    random_interval_time->setDecimals(2);
    random_interval_time->setMaximum(100.00);
    random_interval_time->setMinimum(0.02);
    random_interval_time->setSingleStep(0.01);
    random_interval_time->setValue(app_settings.RandomIntervalTime());
    random_interval_time->setEnabled(false);

    random_interval_time_content_layout->addWidget(_random_interval_time_desc);
    random_interval_time_content_layout->addWidget(random_interval_time);
    random_interval_time_content->setLayout(random_interval_time_content_layout);

    /********************/

    QWidget* memory_configuration_toggle_content = new QWidget(page_content);
    memory_configuration_toggle_content->setFixedHeight(pageContentUniformHeight);

    QHBoxLayout* memory_configuration_toggle_content_layout = new QHBoxLayout(memory_configuration_toggle_content);
    memory_configuration_toggle_content_layout->setSpacing(0);
    memory_configuration_toggle_content_layout->setContentsMargins(QMargins());

    _memory_configuration_toggle_desc = new QLabel(memory_configuration_toggle_content);
    _memory_configuration_toggle_desc->setObjectName(QStringLiteral("memory-configuration-toggle-desc"));
    _memory_configuration_toggle_desc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _memory_configuration_toggle_desc->setFocusPolicy(Qt::NoFocus);
    _memory_configuration_toggle_desc->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _memory_configuration_toggle_desc->setText(tr("Memory Configuration"));

    QRadioButton* memory_configuration_toggle_btn = new QRadioButton(memory_configuration_toggle_content);
    memory_configuration_toggle_btn->setObjectName(QStringLiteral("random-interval-toggle-btn"));
    memory_configuration_toggle_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    memory_configuration_toggle_btn->setChecked(app_settings.EnableMemoryConfiguration());

    memory_configuration_toggle_content_layout->addWidget(_memory_configuration_toggle_desc);
    memory_configuration_toggle_content_layout->addWidget(memory_configuration_toggle_btn);
    memory_configuration_toggle_content->setLayout(memory_configuration_toggle_content_layout);

    /********************/

    page_content_layout->addWidget(click_type_content);
    page_content_layout->addWidget(interval_time_content);
    page_content_layout->addWidget(random_interval_toggle_content);
    page_content_layout->addWidget(random_interval_time_content);
    page_content_layout->addWidget(memory_configuration_toggle_content);
    page_content_layout->addStretch();

    central_layout->addWidget(_page_title);
    central_layout->addWidget(page_content);

    setLayout(central_layout);

    /********************/

    connect(_click_type_list, &QComboBox::currentIndexChanged, &SettingsAgent::instance(), &SettingsAgent::setClickType);
    connect(interval_time, &QDoubleSpinBox::valueChanged, &SettingsAgent::instance(), &SettingsAgent::setIntervalTime);

    connect(random_interval_toggle_btn, &QRadioButton::toggled, random_interval_time,[random_interval_time, interval_time](bool checked) {
        random_interval_time->setEnabled(checked);
        interval_time->setEnabled(!checked);
        SettingsAgent::instance().setEnableRandomInterval(checked);
    });

    connect(random_interval_time, &QDoubleSpinBox::valueChanged, &SettingsAgent::instance(), &SettingsAgent::setRandomIntervalTime);
    connect(memory_configuration_toggle_btn, &QRadioButton::toggled, &SettingsAgent::instance(), &SettingsAgent::setEnableMemoryConfiguration);

    // hotkey event
    connect(settings_page._hotkey_reader, &HotkeyLineEdit::hotkeyActivated, this, [=, &settings_page]() {
        if (NavPage::_clicker_thread->isRunning()) {
            NavPage::_clicker->stop();
            NavPage::_clicker_thread->quit();
            NavPage::_clicker_thread->wait();

            _click_type_list->setEnabled(true);
            interval_time->setEnabled(!random_interval_toggle_btn->isChecked());
            random_interval_toggle_btn->setEnabled(true);
            random_interval_time->setEnabled(random_interval_toggle_btn->isChecked());
            memory_configuration_toggle_btn->setEnabled(true);
        } else {
            Qt::MouseButton btn_type;
            if (_click_type_list->currentIndex() == 0) {
                btn_type = Qt::LeftButton;
            } else if (_click_type_list->currentIndex() == 1) {
                btn_type = Qt::RightButton;
            } else if (_click_type_list->currentIndex() == 2) {
                btn_type = Qt::MiddleButton;
            } else {
                btn_type = Qt::LeftButton;
            }

            int interval = static_cast<int>(interval_time->value() * 1000);                     // 转为毫秒值
            bool random_interval_flag = random_interval_toggle_btn->isChecked();
            int max_random_interval = static_cast<int>(random_interval_time->value() * 1000);   // 转为毫秒值

            NavPage::_clicker->initParameters(btn_type, interval, random_interval_flag, max_random_interval);
            NavPage::_clicker_thread->start();   // Note: This should be initiated through a sub-thread.

            _click_type_list->setEnabled(false);
            interval_time->setEnabled(false);
            random_interval_toggle_btn->setEnabled(false);
            random_interval_time->setEnabled(false);
            memory_configuration_toggle_btn->setEnabled(false);
        }
    });
}

MouseClickPage::~MouseClickPage()
{
    // There is a possibility that the user hasn't stopped clicking when closing the program.
    if (NavPage::_clicker_thread->isRunning()) {
        NavPage::_clicker->stop();
        NavPage::_clicker_thread->quit();
        NavPage::_clicker_thread->wait();
    }
}

QString& MouseClickPage::getThemeFiles(Theme::ThemeMode theme)
{
    return MouseClickPage::_theme_files[theme];
}

void MouseClickPage::retranslateUi()
{
    _page_title->setText(tr("Mouse Click"));
    _click_type_desc->setText(tr("Click Type"));
    _click_type_list->setItemText(0, tr("Left Mouse Button"));
    _click_type_list->setItemText(1, tr("Right Mouse Button"));
    _click_type_list->setItemText(2, tr("Middle Mouse Button"));
    _interval_time_desc->setText(tr("Interval Time"));
    _random_interval_toggle_desc->setText(tr("Random Start Interval"));
    _random_interval_time_desc->setText(tr("Set Max Random Interval"));
    _memory_configuration_toggle_desc->setText(tr("Memory Configuration"));
}

void MouseClickPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    NavPage::changeEvent(event);
}
