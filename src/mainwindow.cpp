#include "mainwindow.h"

#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
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
#include <QThread>
#include <QTime>
#include <QTreeWidget>
#include <QUrl>

#include <QWKWidgets/widgetwindowagent.h>
#include "modules/nav_pages/beautifycursorpage.h"
#include "qwk_window_bar/windowbar.h"
#include "qwk_window_bar/windowbutton.h"

#include "modules/nav_pages/mouseclickpage.h"
#include "modules/nav_pages/settingspage.h"
#include "modules/settingsagent.h"
#include "modules/traymenu.h"

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

    // 连点运行时禁用页面内容区（导航栏保持可交互），最小化至系统托盘 / 恢复窗口
    connect(_settings_page, &SettingsPage::hotkeyActivated, this, [this]() {
        bool running = NavPage::clickerThread()->isRunning();
        _navigation_pages->setEnabled(!running);

        if (running) {
            // 连点已启动：最小化到系统托盘
            _was_maximized_before_tray = isMaximized();
            _was_hidden_before_clicker = !isVisible();
            hide();
            if (_tray_icon) {
                _tray_icon->showMessage(
                    tr("MouseClick"),
                    tr("Clicker started"),
                    QSystemTrayIcon::Information,
                    3000);
            }
        } else {
            // 连点已停止：仅在启动前为显示状态时才恢复窗口
            if (!_was_hidden_before_clicker) {
                if (_was_maximized_before_tray) {
                    showMaximized();
                } else {
                    showNormal();
                }
                raise();
                activateWindow();
            }
            if (_tray_icon) {
                _tray_icon->showMessage(
                    tr("MouseClick"),
                    tr("Clicker stopped"),
                    QSystemTrayIcon::Information,
                    3000);
            }
        }
    });

    /******************/

    connectInit();

    setupSystemTray();
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
    _settings_page = new SettingsPage(settings_page_title, navigation_pages);
    BeautifyCursorPage* beautify_cursor_page = new BeautifyCursorPage(beautify_cursor_page_title, navigation_pages);
    MouseClickPage* mouse_click_page = new MouseClickPage(mouse_click_page_title, *_settings_page, navigation_pages);

    navigation_pages->addWidget(mouse_click_page);
    navigation_pages->addWidget(beautify_cursor_page);
    navigation_pages->addWidget(_settings_page);

    // set Default page
    navigation_pages->setCurrentIndex(0);

    central_layout->addWidget(navigation);
    central_layout->addWidget(navigation_pages);

    _navigation_pages = navigation_pages;

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

    // 更新系统托盘菜单文本
    if (_tray_open_action) {
        _tray_open_action->setText(tr("Open Main Interface"));
    }
    if (_tray_website_action) {
        _tray_website_action->setText(tr("Official Website"));
    }
    if (_tray_exit_action) {
        _tray_exit_action->setText(tr("Exit"));
    }
    if (_tray_icon) {
        _tray_icon->setToolTip(tr("MouseClick"));
    }
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
    connect(&SettingsAgent::instance(), &SettingsAgent::currentThemeChanged, this, &MainWindow::applyTrayMenuStyle);
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (_force_quit || QCoreApplication::closingDown()) {
        event->accept();
        QMainWindow::closeEvent(event);
        return;
    }

    if (SettingsAgent::instance().CloseButtonBehavior() == "minimize") {
        // 最小化至系统托盘
        _was_maximized_before_tray = isMaximized();
        hide();
        event->ignore();
    } else {
        // 正常退出
        event->accept();
        // 停止连点（如果正在运行）
        if (NavPage::clickerThread()->isRunning()) {
            NavPage::clicker()->stop();
            NavPage::clickerThread()->quit();
            NavPage::clickerThread()->wait();
        }
        QApplication::quit();
    }
}

void MainWindow::setupSystemTray()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    _tray_icon = new QSystemTrayIcon(QIcon(":/svg/favicon.svg"), this);
    _tray_icon->setToolTip(tr("MouseClick"));

    _tray_menu = new TrayMenu(this);

    _tray_open_action = _tray_menu->addAction(tr("Open Main Interface"));
    _tray_menu->addSeparator();
    _tray_website_action = _tray_menu->addAction(tr("Official Website"));
    _tray_menu->addSeparator();
    _tray_exit_action = _tray_menu->addAction(tr("Exit"));

    // 不使用 setContextMenu()，改用 popup() 手动弹出 Qt 渲染菜单，
    // 以便 QSS 样式（深色/浅色主题）能正确应用到菜单背景与文字
    applyTrayMenuStyle();

    // 托盘图标交互：左键/双击恢复窗口，右键弹出菜单
    connect(_tray_icon, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger ||
            reason == QSystemTrayIcon::DoubleClick) {
            if (_was_maximized_before_tray) {
                showMaximized();
            } else {
                showNormal();
            }
            raise();
            activateWindow();
        } else if (reason == QSystemTrayIcon::Context) {
            _tray_menu->popup(QCursor::pos());
        }
    });

    // 打开主界面
    connect(_tray_open_action, &QAction::triggered, this, [this]() {
        if (_was_maximized_before_tray) {
            showMaximized();
        } else {
            showNormal();
        }
        raise();
        activateWindow();
    });

    // 官网
    connect(_tray_website_action, &QAction::triggered, this, []() {
        QDesktopServices::openUrl(
            QUrl("https://github.com/SeaEpoch/MouseClick"));
    });

    // 退出
    connect(_tray_exit_action, &QAction::triggered, this, [this]() {
        _force_quit = true;
        // 停止连点（如果正在运行）
        if (NavPage::clickerThread()->isRunning()) {
            NavPage::clicker()->stop();
            NavPage::clickerThread()->quit();
            NavPage::clickerThread()->wait();
        }
        qApp->quit();
    });

    _tray_icon->show();
}

void MainWindow::applyTrayMenuStyle()
{
    if (!_tray_menu) {
        return;
    }

    bool is_dark = SettingsAgent::instance().ThemeMode() == Theme::Dark;
    _tray_menu->setDarkMode(is_dark);
}
