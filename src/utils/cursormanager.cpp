#include "cursormanager.h"
#include "logger.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QSettings>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// Windows 光标方案字符串中的标准顺序
const QStringList CursorManager::cursorTypeOrder = {
    "Arrow",       "Help",        "AppStarting",
    "Wait",        "Crosshair",   "IBeam",
    "NWPen",       "No",          "SizeNS",
    "SizeWE",      "SizeNESW",    "SizeNWSE",
    "SizeAll",     "UpArrow",     "Hand",
    "Pin",         "Person"
};

CursorManager::CursorManager() {}

bool CursorManager::installCursor(const QString& cursorName, const QJsonObject& config)
{
    const QString themeName = config["name"].toString();
    const QString srcDir = QCoreApplication::applicationDirPath() + "/cursors/" + cursorName + "/";
    const QString destDir = "C:/Windows/Cursors/" + themeName + "/";

    QDir src(srcDir);
    if (!src.exists()) {
        LOG_WARNING(QString("Source cursor directory does not exist: %1").arg(srcDir));
        return false;
    }

    // 确保目标子目录存在
    QDir dest(destDir);
    if (!dest.exists()) {
        QDir parent("C:/Windows/Cursors/");
        if (!parent.mkdir(themeName)) {
            LOG_WARNING(QString("Failed to create cursor theme directory: %1").arg(destDir));
            return false;
        }
    }

    QStringList failedFiles;
    const QFileInfoList fileList = src.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    for (const QFileInfo& fileInfo : fileList) {
        const QString fileName = fileInfo.fileName();

        // 跳过 config.json 和 logo.png
        if (fileName == "config.json" || fileName == "logo.png") {
            continue;
        }

        const QString destPath = destDir + fileName;

        // 如果目标文件已存在，先删除再复制（QFile::copy 不会覆盖已有文件）
        if (QFile::exists(destPath)) {
            QFile::remove(destPath);
        }

        if (!QFile::copy(fileInfo.absoluteFilePath(), destPath)) {
            failedFiles.append(fileName);
            LOG_WARNING(QString("Failed to copy cursor file: %1 -> %2")
                            .arg(fileInfo.absoluteFilePath(), destPath));
        }
    }

    if (!failedFiles.isEmpty()) {
        LOG_WARNING(QString("Cursor install partially failed (%1/%2 files): %3")
                        .arg(failedFiles.size())
                        .arg(fileList.size() - 2)  // 排除 config.json 和 logo.png
                        .arg(failedFiles.join(", ")));
        return false;
    }

    return true;
}

bool CursorManager::applyCursor(const QString& cursorName, const QJsonObject& config)
{
    Q_UNUSED(cursorName)

    // 防御式校验：确保光标文件已安装到系统目录
    const QJsonObject cursorObj = config["cursor"].toObject();
    const QString themeName = config["name"].toString();
    const QString themeDir = "C:/Windows/Cursors/" + themeName + "/";

    for (const QString& cursorType : cursorTypeOrder) {
        const QString cursorFile = cursorObj.value(cursorType).toString();
        if (cursorFile.isEmpty()) {
            continue;
        }
        if (!QFile::exists(themeDir + cursorFile)) {
            LOG_WARNING(QString("Cursor file not installed, cannot apply: %1").arg(themeDir + cursorFile));
            return false;
        }
    }

    // 写入注册表
    writeCursorRegistry(cursorName, config);

#ifdef Q_OS_WIN
    // 立即刷新系统光标
    SystemParametersInfoW(SPI_SETCURSORS, 0, 0, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
#endif

    return true;
}

bool CursorManager::isInstalled(const QJsonObject& config) const
{
    const QJsonObject cursorObj = config["cursor"].toObject();
    const QString themeName = config["name"].toString();
    const QString themeDir = "C:/Windows/Cursors/" + themeName + "/";

    for (const QString& cursorType : cursorTypeOrder) {
        const QString cursorFile = cursorObj.value(cursorType).toString();
        if (cursorFile.isEmpty()) {
            continue;
        }
        if (QFile::exists(themeDir + cursorFile)) {
            return true;
        }
    }
    return false;
}

bool CursorManager::isSchemeRegistered(const QJsonObject& config) const
{
    const QString schemeName = config["name"].toString();
    QSettings schemeSettings("HKEY_CURRENT_USER\\Control Panel\\Cursors\\Schemes", QSettings::NativeFormat);
    return schemeSettings.contains(schemeName);
}

bool CursorManager::isCurrentlyApplied(const QJsonObject& config) const
{
    const QJsonObject cursorObj = config["cursor"].toObject();
    const QString themeName = config["name"].toString();
    const QString themePath = "C:\\Windows\\Cursors\\" + themeName + "\\";

    QSettings cursorSettings("HKEY_CURRENT_USER\\Control Panel\\Cursors", QSettings::NativeFormat);

    for (const QString& cursorType : cursorTypeOrder) {
        const QString cursorFile = cursorObj.value(cursorType).toString();
        if (cursorFile.isEmpty()) {
            continue;
        }
        const QString regValue = cursorSettings.value(cursorType).toString();
        if (regValue.contains(themePath)) {
            return true;
        }
    }
    return false;
}

bool CursorManager::uninstallCursor(const QString& cursorName, const QJsonObject& config)
{
    // 仅在卸载当前正在使用的光标主题时才重置光标
    if (isCurrentlyApplied(config)) {
        resetCursorRegistryToDefault();
#ifdef Q_OS_WIN
        SystemParametersInfoW(SPI_SETCURSORS, 0, 0, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
#endif
    }

    // 从 Schemes 中移除该光标主题
    const QString schemeName = config["name"].toString();
    QSettings schemeSettings("HKEY_CURRENT_USER\\Control Panel\\Cursors\\Schemes", QSettings::NativeFormat);
    schemeSettings.remove(schemeName);

    // 删除 C:\Windows\Cursors\<themeName>\ 下的文件及目录
    deleteCursorFilesFromSystem(cursorName, config);

    return true;
}

QString CursorManager::buildSchemeString(const QString& cursorName, const QJsonObject& config) const
{
    Q_UNUSED(cursorName)

    const QJsonObject cursorObj = config["cursor"].toObject();
    const QString themeName = config["name"].toString();
    const QString basePath = "C:\\Windows\\Cursors\\" + themeName + "\\";
    QStringList paths;

    for (const QString& cursorType : cursorTypeOrder) {
        const QString cursorFile = cursorObj.value(cursorType).toString();
        if (cursorFile.isEmpty()) {
            paths.append(QString());  // 空字符串表示使用系统默认光标
        } else {
            paths.append(basePath + cursorFile);
        }
    }

    return paths.join(',');
}

void CursorManager::writeCursorRegistry(const QString& cursorName, const QJsonObject& config)
{
    Q_UNUSED(cursorName)

    const QJsonObject cursorObj = config["cursor"].toObject();
    const QString schemeName = config["name"].toString();
    const QString themeName = config["name"].toString();
    const QString basePath = "C:\\Windows\\Cursors\\" + themeName + "\\";

    // 1. 写入方案到 HKCU\Control Panel\Cursors\Schemes
    const QString schemeString = buildSchemeString(cursorName, config);
    QSettings schemeSettings("HKEY_CURRENT_USER\\Control Panel\\Cursors\\Schemes", QSettings::NativeFormat);
    schemeSettings.setValue(schemeName, schemeString);

#ifdef QT_DEBUG
    LOG_DEBUG(QString("Registered cursor scheme: %1").arg(schemeName));
#endif

    // 2. 逐个写入光标路径到 HKCU\Control Panel\Cursors\<CursorType>
    QSettings cursorSettings("HKEY_CURRENT_USER\\Control Panel\\Cursors", QSettings::NativeFormat);

    for (const QString& cursorType : cursorTypeOrder) {
        const QString cursorFile = cursorObj.value(cursorType).toString();
        if (!cursorFile.isEmpty()) {
            cursorSettings.setValue(cursorType, basePath + cursorFile);
        } else {
            cursorSettings.setValue(cursorType, QString());  // 空表示使用默认光标
        }
    }

    // 处理 precisionhair（非标准光标类型，单独处理）
    if (cursorObj.contains("precisionhair") && !cursorObj.value("precisionhair").toString().isEmpty()) {
        const QString precisionFile = cursorObj.value("precisionhair").toString();
        cursorSettings.setValue("precisionhair", basePath + precisionFile);
    }
}

void CursorManager::resetCursorRegistryToDefault()
{
    QSettings cursorSettings("HKEY_CURRENT_USER\\Control Panel\\Cursors", QSettings::NativeFormat);

    // 将所有光标类型重置为空（系统将使用内置默认光标）
    for (const QString& cursorType : cursorTypeOrder) {
        cursorSettings.setValue(cursorType, QString());
    }
    cursorSettings.setValue("precisionhair", QString());

#ifdef QT_DEBUG
    LOG_DEBUG("Cursor registry reset to default");
#endif
}

void CursorManager::deleteCursorFilesFromSystem(const QString& cursorName, const QJsonObject& config)
{
    Q_UNUSED(cursorName)

    const QJsonObject cursorObj = config["cursor"].toObject();
    const QString themeName = config["name"].toString();
    const QString themeDir = "C:/Windows/Cursors/" + themeName + "/";

    for (const QString& cursorType : cursorTypeOrder) {
        const QString cursorFile = cursorObj.value(cursorType).toString();
        if (cursorFile.isEmpty()) {
            continue;
        }

        const QString fullPath = themeDir + cursorFile;
        QFile file(fullPath);
        if (file.exists()) {
            if (!file.remove()) {
                LOG_WARNING(QString("Failed to delete cursor file: %1").arg(fullPath));
            }
        }
    }

    // 删除 precisionhair 对应文件
    if (cursorObj.contains("precisionhair")) {
        const QString precisionFile = cursorObj.value("precisionhair").toString();
        if (!precisionFile.isEmpty()) {
            const QString fullPath = themeDir + precisionFile;
            QFile file(fullPath);
            if (file.exists()) {
                file.remove();
            }
        }
    }

    // 删除空目录（如果目录内还有其他文件则保留）
    QDir dir(themeDir);
    if (dir.exists()) {
        dir.rmdir(themeDir);  // rmdir 只会删除空目录
    }
}
