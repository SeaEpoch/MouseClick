#include "mainwindow.h"

#include <QApplication>
#include <QFont>
#include <QFontDatabase>

#include "modules/settingsagent.h"
#include "modules/translationmanager.h"

int main(int argc, char* argv[])
{
    QGuiApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    qputenv("QT_WIN_DEBUG_CONSOLE", "attach");
    qputenv("QSG_INFO", "1");

    QApplication app(argc, argv);

    TranslationManager::instance().init();
    SettingsAgent& app_settings = SettingsAgent::instance();
    TranslationManager::instance().switchLanguage(app_settings.Language());

    // 设置字体
    int font_id = QFontDatabase::addApplicationFont(":/fonts/HarmonyOS_Sans_SC/HarmonyOS_Sans_SC_Regular.ttf");
    if (font_id != -1) {
        QStringList font_families = QFontDatabase::applicationFontFamilies(font_id);
        if (!font_families.empty()) {
            QFont font(font_families.at(0), 12);
            app.setFont(font);
        }
    }

    MainWindow window;
    window.show();

    return app.exec();
}
