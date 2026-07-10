#include "mainwindow.h"

#include <QCoreApplication>
#include <QEvent>
#include <QFile>
#include <QGuiApplication>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QHBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QDesktopServices>
#include <QListWidget>
#include <QStackedWidget>
#include <QTime>
#include <QTreeWidget>

#include <QWKWidgets/widgetwindowagent.h>
#include "modules/nav_pages/beautifycursorpage.h"
#include "qwk_window_bar/windowbar.h"
#include "qwk_window_bar/windowbutton.h"

#include "modules/nav_pages/mouseclickpage.h"
#include "modules/nav_pages/settingspage.h"
#include "modules/settingsagent.h"

QMap<Theme::ThemeMode, QString> MainWindow::_theme_files {
    {Theme::Light, (":/qss/light-style.qss")},
    {Theme::Dark, (":/qss/dark-style.qss")}
};

static inline void emulateLeaveEvent(QWidget* widget)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    QTimer::singleShot(0, widget, [widget]() {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        const QScreen* screen = widget->screen();
#else
        const QScreen* screen = widget->windowHandle()->screen();
#endif
        const QPoint globalPos = QCursor::pos(screen);
        if (!QRect(widget->mapToGlobal(QPoint{0, 0}), widget->size()).contains(globalPos)) {
            QCoreApplication::postEvent(widget, new QEvent(QEvent::Leave));
            if (widget->testAttribute(Qt::WA_Hover)) {
                const QPoint localPos = widget->mapFromGlobal(globalPos);
                const QPoint scenePos = widget->window()->mapFromGlobal(globalPos);
                static constexpr const auto oldPos = QPoint{};
                const Qt::KeyboardModifiers modifiers = QGuiApplication::keyboardModifiers();
#if (QT_VERSION >= QT_VERSION_CHECK(6, 4, 0))
                const auto event =
                    new QHoverEvent(QEvent::HoverLeave, scenePos, globalPos, oldPos, modifiers);
                Q_UNUSED(localPos);
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 3, 0))
                const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, globalPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#else
                const auto event =  new QHoverEvent(QEvent::HoverLeave, localPos, oldPos, modifiers);
                Q_UNUSED(scenePos);
#endif
                QCoreApplication::postEvent(widget, event);
            }
        }
    });
}


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    SettingsAgent& app_settings = SettingsAgent::instance();

    setWindowState(app_settings.WindowState());
    loadThemeStyelSheet(app_settings.ThemeMode());

    /******************/

    setMinimumSize(QSize(800, 600));
    resize(QSize(800, 600));
    windowInit(tr("MouseClick"), QIcon(":/svg/favicon.svg"));

    UIWidgetInit();

    /******************/

    connectInit();
}

void MainWindow::windowInit(const QString& title, const QIcon& icon)
{
    setObjectName(QStringLiteral("mainwindow"));

    // set the window agent
    _window_agent = new QWK::WidgetWindowAgent(this);
    _window_agent->setup(this);

    QLabel* titlebar_label = new QLabel();
    titlebar_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    titlebar_label->setObjectName(QStringLiteral("titlebar-label"));
    titlebar_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QWK::WindowButton* icon_btn = new QWK::WindowButton();
    icon_btn->setObjectName(QStringLiteral("titlebar-icon-button"));
    icon_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    icon_btn->setIconNormal(icon);

    QWK::WindowButton* min_btn = new QWK::WindowButton();
    min_btn->setObjectName(QStringLiteral("titlebar-min-button"));
    min_btn->setProperty("system-button", true);
    min_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QWK::WindowButton* max_btn = new QWK::WindowButton();
    max_btn->setCheckable(true);
    max_btn->setObjectName(QStringLiteral("titlebar-max-button"));
    max_btn->setProperty("system-button", true);
    max_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QWK::WindowButton* close_btn = new QWK::WindowButton();
    close_btn->setObjectName(QStringLiteral("titlebar-close-button"));
    close_btn->setProperty("system-button", true);
    close_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QWK::WindowBar* titlebar = new QWK::WindowBar();
    titlebar->setIconButton(icon_btn);
    titlebar->setMinButton(min_btn);
    titlebar->setMaxButton(max_btn);
    titlebar->setCloseButton(close_btn);
    titlebar->setTitleLabel(titlebar_label);
    titlebar->setHostWidget(this);

    _window_agent->setTitleBar(titlebar);
    _window_agent->setSystemButton(QWK::WindowAgentBase::WindowIcon, icon_btn);
    _window_agent->setSystemButton(QWK::WindowAgentBase::Minimize, min_btn);
    _window_agent->setSystemButton(QWK::WindowAgentBase::Maximize, max_btn);
    _window_agent->setSystemButton(QWK::WindowAgentBase::Close, close_btn);

    setMenuWidget(titlebar);

    // Adds simulated mouse events to the title bar buttons
    connect(icon_btn, &QAbstractButton::clicked, _window_agent, [this, icon_btn]() {
        icon_btn->setProperty("double-click-close", false);

        QTimer::singleShot(80, _window_agent, [this, icon_btn]() {
            if (icon_btn->property("double-click-close").toBool())
                return;
            _window_agent->showSystemMenu(icon_btn->mapToGlobal(QPoint{0, icon_btn->height()}));
        });
    });
    connect(icon_btn, &QWK::WindowButton::doubleClicked, this, [this, icon_btn]() {
        icon_btn->setProperty("double-click-close", true);
        close();
    });
    connect(titlebar, &QWK::WindowBar::minimizeRequested, this, &QWidget::showMinimized);
    connect(titlebar, &QWK::WindowBar::maximizeRequested, this, [this, max_btn](bool max) {
        if (max) {
            showMaximized();
        } else {
            showNormal();
        }

        // It's a Qt issue that if a QAbstractButton::clicked triggers a window's maximization,
        // the button remains to be hovered until the mouse move. As a result, we need to
        // manually send leave events to the button.
        emulateLeaveEvent(max_btn);
    });
    connect(titlebar, &QWK::WindowBar::closeRequested, this, &QWidget::close);

    // set the window title
    setWindowTitle(title);

    // set the window icon
    setWindowIcon(icon);
}

void MainWindow::loadThemeStyelSheet(Theme::ThemeMode theme)
{
    QFile style_file(_theme_files[theme]);
    if (style_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setStyleSheet(QString::fromUtf8(style_file.readAll()));
    }
}

void MainWindow::UIWidgetInit()
{
    QWidget* central_widget = new QWidget(this);
    central_widget->setObjectName(QStringLiteral("central-widget"));
    central_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout* central_layout = new QHBoxLayout(central_widget);
    central_layout->setSpacing(0);
    central_layout->setContentsMargins(QMargins(8, 8, 8, 8));

    QTreeWidget* navigation = new QTreeWidget(central_widget);
    navigation->setObjectName(QStringLiteral("side-nav"));
    navigation->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    navigation->setMaximumWidth(240);
    navigation->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navigation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navigation->setHeaderHidden(true);
    navigation->setIndentation(0);
    navigation->setColumnCount(1);

    QTreeWidgetItem* mouse_click_item = new QTreeWidgetItem(navigation);
    QTreeWidgetItem* beautify_cursor_item = new QTreeWidgetItem(navigation);
    QTreeWidgetItem* settings_item = new QTreeWidgetItem(navigation);

    QButtonGroup* navigation_item_btn_group = new QButtonGroup(navigation);

    const QString mouse_click_page_title = tr("Mouse Click");
    const QString beautify_cursor_page_title = tr("Beautify Cursor");
    const QString settings_page_title = tr("Settings");

    _nav_mouse_click = new QPushButton(mouse_click_page_title, navigation);
    _nav_beautify_cursor = new QPushButton(beautify_cursor_page_title, navigation);
    _nav_settings = new QPushButton(settings_page_title, navigation);

    _nav_mouse_click->setCheckable(true);
    _nav_beautify_cursor->setCheckable(true);
    _nav_settings->setCheckable(true);

    _nav_mouse_click->setObjectName(QStringLiteral("nav-item-mouse-click"));
    _nav_beautify_cursor->setObjectName(QStringLiteral("nav-item-beautify-cursor"));
    _nav_settings->setObjectName(QStringLiteral("nav-item-settings"));

    navigation_item_btn_group->addButton(_nav_mouse_click);
    navigation_item_btn_group->addButton(_nav_beautify_cursor);
    navigation_item_btn_group->addButton(_nav_settings);

    navigation->setItemWidget(mouse_click_item, 0, _nav_mouse_click);
    navigation->setItemWidget(beautify_cursor_item, 0, _nav_beautify_cursor);
    navigation->setItemWidget(settings_item, 0, _nav_settings);

    // set Default selected
    _nav_mouse_click->setChecked(true);
    navigation->setCurrentItem(mouse_click_item);

    QStackedWidget* navigation_pages = new QStackedWidget(central_widget);
    navigation_pages->setObjectName(QStringLiteral("nav-page"));
    navigation_pages->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    navigation_pages->setContentsMargins(QMargins());

    // SettingsPage 需要优先声明，这里的设计以后会改进
    SettingsPage* settings_page = new SettingsPage(settings_page_title, navigation_pages);
    BeautifyCursorPage* beautify_cursor_page = new BeautifyCursorPage(beautify_cursor_page_title, navigation_pages);
    MouseClickPage* mouse_click_page = new MouseClickPage(mouse_click_page_title, *settings_page, navigation_pages);

    navigation_pages->addWidget(mouse_click_page);
    navigation_pages->addWidget(beautify_cursor_page);
    navigation_pages->addWidget(settings_page);

    // set Default page
    navigation_pages->setCurrentIndex(0);

    central_layout->addWidget(navigation);
    central_layout->addWidget(navigation_pages);

    connect(navigation, &QTreeWidget::currentItemChanged, this, [=](QTreeWidgetItem *current, QTreeWidgetItem *previous) {
        if (current == mouse_click_item) {
            _nav_mouse_click->setChecked(true);
            navigation_pages->setCurrentIndex(0);
        } else if (current == beautify_cursor_item) {
            _nav_beautify_cursor->setChecked(true);
            navigation_pages->setCurrentIndex(1);
        } else if (current == settings_item) {
            _nav_settings->setChecked(true);
            navigation_pages->setCurrentIndex(2);
        }
    });

    setCentralWidget(central_widget);
}

void MainWindow::retranslateUi()
{
    setWindowTitle(tr("MouseClick"));
    _nav_mouse_click->setText(tr("Mouse Click"));
    _nav_beautify_cursor->setText(tr("Beautify Cursor"));
    _nav_settings->setText(tr("Settings"));
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::connectInit()
{
    connect(&SettingsAgent::instance(), &SettingsAgent::currentThemeChanged, this, &MainWindow::loadThemeStyelSheet);
    connect(this, &MainWindow::windowStateChanged, &SettingsAgent::instance(), &SettingsAgent::setWindowState);
}

MainWindow::~MainWindow()
{}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        Qt::WindowStates newState = windowState();
        emit windowStateChanged(newState);
    }
    return QWidget::event(event); // 保留其他事件处理
}
