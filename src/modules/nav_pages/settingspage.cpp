#include "settingspage.h"

#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <QThread>
#include <QTranslator>

#include "../hotkeylineedit.h"
#include "../settingsagent.h"
#include "../shared.h"

QMap<Theme::ThemeMode, QString> SettingsPage::_theme_files {
    {Theme::Light, (":/qss/modules/light-settingspage.qss")},
    {Theme::Dark, (":/qss/modules/dark-settingspage.qss")}
};

SettingsPage::SettingsPage(const QString& title, QWidget* parent)
    : NavPage{parent},
      _hotkey_reader(nullptr),
      _hotkey_clean(nullptr),
      _page_title(nullptr),
      _hotkey_desc(nullptr),
      _theme_toggle_desc(nullptr),
      _language_switch_desc(nullptr),
      _language_list(nullptr),
      _close_button_behavior_desc(nullptr),
      _close_button_behavior_list(nullptr)
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

    QWidget* hotkey_content = new QWidget(page_content);
    hotkey_content->setFixedHeight(pageContentUniformHeight);

    QHBoxLayout* hotkey_content_layout = new QHBoxLayout(hotkey_content);
    hotkey_content_layout->setSpacing(0);
    hotkey_content_layout->setContentsMargins(QMargins());

    _hotkey_desc = new QLabel(hotkey_content);
    _hotkey_desc->setObjectName(QStringLiteral("hotkey-desc"));
    _hotkey_desc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _hotkey_desc->setFocusPolicy(Qt::NoFocus);
    _hotkey_desc->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _hotkey_desc->setText(tr("Start/End Hotkey"));

    _hotkey_reader = new HotkeyLineEdit(hotkey_content);
    _hotkey_reader->setObjectName(QStringLiteral("hotkey-reader"));
    _hotkey_reader->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    // 设置保存的快捷键（上一次使用）
    const QString pre_hotkey = app_settings.Hotkey();
    if (pre_hotkey.isEmpty()) {
        _hotkey_reader->setHotkey("Ctrl+F2");   // 默认快捷键
    }
    else {
        _hotkey_reader->setHotkey(pre_hotkey);  // 上一次使用的快捷键
    }

    hotkey_content_layout->addWidget(_hotkey_desc);
    hotkey_content_layout->addWidget(_hotkey_reader);
    hotkey_content->setLayout(hotkey_content_layout);

    /********************/

    _hotkey_clean = new QPushButton(page_content);
    _hotkey_clean->setObjectName(QStringLiteral("hotkey-clean-btn"));
    _hotkey_clean->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _hotkey_clean->setFixedHeight(pageContentUniformHeight);
    _hotkey_clean->setText(tr("Hotkey Clean"));

    /********************/

    QWidget* theme_toggle_content = new QWidget(page_content);
    theme_toggle_content->setFixedHeight(pageContentUniformHeight);

    QHBoxLayout* theme_toggle_content_layout = new QHBoxLayout(theme_toggle_content);
    theme_toggle_content_layout->setSpacing(0);
    theme_toggle_content_layout->setContentsMargins(QMargins());

    _theme_toggle_desc = new QLabel(theme_toggle_content);
    _theme_toggle_desc->setObjectName(QStringLiteral("theme-toggle-desc"));
    _theme_toggle_desc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _theme_toggle_desc->setFocusPolicy(Qt::NoFocus);
    _theme_toggle_desc->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _theme_toggle_desc->setText(tr("Dark Theme"));

    QRadioButton* theme_toggle_btn = new QRadioButton(theme_toggle_content);
    theme_toggle_btn->setObjectName(QStringLiteral("theme-toggle-btn"));
    theme_toggle_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    theme_toggle_btn->setChecked(static_cast<bool>(app_settings.ThemeMode()));

    theme_toggle_content_layout->addWidget(_theme_toggle_desc);
    theme_toggle_content_layout->addWidget(theme_toggle_btn);
    theme_toggle_content->setLayout(theme_toggle_content_layout);

    /********************/

    QWidget* language_switch_content = new QWidget(page_content);
    language_switch_content->setFixedHeight(pageContentUniformHeight);

    QHBoxLayout* language_switch_content_layout = new QHBoxLayout(language_switch_content);
    language_switch_content_layout->setSpacing(0);
    language_switch_content_layout->setContentsMargins(QMargins());

    _language_switch_desc = new QLabel(language_switch_content);
    _language_switch_desc->setObjectName(QStringLiteral("language-switch-desc"));
    _language_switch_desc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _language_switch_desc->setFocusPolicy(Qt::NoFocus);
    _language_switch_desc->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _language_switch_desc->setText(tr("Language"));

    _language_list = new QComboBox(language_switch_content);
    _language_list->setObjectName(QStringLiteral("language-list"));
    _language_list->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _language_list->addItem(tr("English(United States)"), QVariant("en_US"));
    _language_list->addItem(tr("Chinese(Simplified)"), QVariant("zh_CN"));
    _language_list->addItem(tr("Chinese(Traditional)"), QVariant("zh_TW"));

    // 确定当前的语言
    QString current_language = app_settings.Language();

    // 遍历 QComboBox 项目，找到对应的选项并设置为选中状态
    for (int i = 0; i < _language_list->count(); ++i) {
        QVariant item_data = _language_list->itemData(i);
        if (item_data.toString() == current_language) {
            _language_list->setCurrentIndex(i);
            break;
        }
    }

    language_switch_content_layout->addWidget(_language_switch_desc);
    language_switch_content_layout->addWidget(_language_list);
    language_switch_content->setLayout(language_switch_content_layout);

    /********************/

    QWidget* close_btn_behavior_content = new QWidget(page_content);
    close_btn_behavior_content->setFixedHeight(pageContentUniformHeight);

    QHBoxLayout* close_btn_behavior_layout = new QHBoxLayout(close_btn_behavior_content);
    close_btn_behavior_layout->setSpacing(0);
    close_btn_behavior_layout->setContentsMargins(QMargins());

    _close_button_behavior_desc = new QLabel(close_btn_behavior_content);
    _close_button_behavior_desc->setObjectName(QStringLiteral("close-btn-behavior-desc"));
    _close_button_behavior_desc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _close_button_behavior_desc->setFocusPolicy(Qt::NoFocus);
    _close_button_behavior_desc->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _close_button_behavior_desc->setText(tr("Close Button Behavior"));

    _close_button_behavior_list = new QComboBox(close_btn_behavior_content);
    _close_button_behavior_list->setObjectName(QStringLiteral("close-btn-behavior-list"));
    _close_button_behavior_list->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    _close_button_behavior_list->addItem(tr("Minimize to Tray"), QVariant("minimize"));
    _close_button_behavior_list->addItem(tr("Exit Program"), QVariant("exit"));

    // 确定当前的关闭按钮行为
    QString current_behavior = app_settings.CloseButtonBehavior();
    for (int i = 0; i < _close_button_behavior_list->count(); ++i) {
        QVariant item_data = _close_button_behavior_list->itemData(i);
        if (item_data.toString() == current_behavior) {
            _close_button_behavior_list->setCurrentIndex(i);
            break;
        }
    }

    close_btn_behavior_layout->addWidget(_close_button_behavior_desc);
    close_btn_behavior_layout->addWidget(_close_button_behavior_list);
    close_btn_behavior_content->setLayout(close_btn_behavior_layout);

    /********************/

    page_content_layout->addWidget(hotkey_content);
    page_content_layout->addWidget(_hotkey_clean);
    page_content_layout->addWidget(theme_toggle_content);
    page_content_layout->addWidget(language_switch_content);
    page_content_layout->addWidget(close_btn_behavior_content);
    page_content_layout->addStretch();

    central_layout->addWidget(_page_title);
    central_layout->addWidget(page_content);

    setLayout(central_layout);

    /********************/

    connect(_hotkey_clean, &QPushButton::clicked, this, [this]() {
        _hotkey_reader->cleanHotKey();
    });

    connect(theme_toggle_btn, &QRadioButton::toggled, this, [this](bool checked) {
        auto theme_mode = static_cast<Theme::ThemeMode>(checked);
        SettingsAgent::instance().setThemeMode(theme_mode);
    });

    // 将 HotkeyLineEdit 的信号代理为 SettingsPage 的 public 信号
    connect(_hotkey_reader, &HotkeyLineEdit::hotkeyActivated,
            this, &SettingsPage::hotkeyActivated);

    connect(_hotkey_reader, &HotkeyLineEdit::currentHotkeyChanged, this, [](const QString& hotkey) {
        SettingsAgent::instance().setHotkey(hotkey);
    });

    connect(_language_list, &QComboBox::currentIndexChanged, this, [=](int index) {
        SettingsAgent& app_settings = SettingsAgent::instance();
        QString selected_language = _language_list->itemData(index).toString();

        // 判断选择的语言是否和当前语言相同，不同则进行切换
        if (selected_language != app_settings.Language()) {
            app_settings.setLanguage(selected_language);
            // setLanguage() 内部已触发 translator 热切换 + currentLanguageChanged 信号
        }
    });

    connect(_close_button_behavior_list, &QComboBox::currentIndexChanged, this, [=](int index) {
        SettingsAgent::instance().setCloseButtonBehavior(
            _close_button_behavior_list->itemData(index).toString());
    });
}

SettingsPage::~SettingsPage()
{
    delete _hotkey_reader;
    delete _hotkey_clean;
}

QString& SettingsPage::getThemeFiles(Theme::ThemeMode theme)
{
    return SettingsPage::_theme_files[theme];
}

void SettingsPage::retranslateUi()
{
    _page_title->setText(tr("Settings"));
    _hotkey_desc->setText(tr("Start/End Hotkey"));
    _hotkey_clean->setText(tr("Hotkey Clean"));
    _hotkey_reader->setPlaceholderText(tr("Please set a shortcut hotkey"));
    _theme_toggle_desc->setText(tr("Dark Theme"));
    _language_switch_desc->setText(tr("Language"));
    _close_button_behavior_desc->setText(tr("Close Button Behavior"));

    // 语言选择下拉框需要重建项目（保留当前选中值）
    // blockSignals 防止 clear/addItem/setCurrentIndex 触发 currentIndexChanged，
    // 否则会再次调用 setLanguage() 导致递归
    const QVariant currentLang = _language_list->currentData();
    _language_list->blockSignals(true);
    _language_list->clear();
    _language_list->addItem(tr("English(United States)"), QVariant("en_US"));
    _language_list->addItem(tr("Chinese(Simplified)"), QVariant("zh_CN"));
    _language_list->addItem(tr("Chinese(Traditional)"), QVariant("zh_TW"));

    for (int i = 0; i < _language_list->count(); ++i) {
        if (_language_list->itemData(i) == currentLang) {
            _language_list->setCurrentIndex(i);
            break;
        }
    }
    _language_list->blockSignals(false);

    // 关闭按钮行为下拉框需要重建项目（保留当前选中值）
    const QVariant currentBehavior = _close_button_behavior_list->currentData();
    _close_button_behavior_list->blockSignals(true);
    _close_button_behavior_list->clear();
    _close_button_behavior_list->addItem(tr("Minimize to Tray"), QVariant("minimize"));
    _close_button_behavior_list->addItem(tr("Exit Program"), QVariant("exit"));
    for (int i = 0; i < _close_button_behavior_list->count(); ++i) {
        if (_close_button_behavior_list->itemData(i) == currentBehavior) {
            _close_button_behavior_list->setCurrentIndex(i);
            break;
        }
    }
    _close_button_behavior_list->blockSignals(false);
}

void SettingsPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    NavPage::changeEvent(event);
}
