#ifndef CURSORMANAGER_H
#define CURSORMANAGER_H

#include <QJsonObject>
#include <QString>
#include <QStringList>

class CursorManager
{
public:
    explicit CursorManager();

    bool installCursor(const QString& cursorName, const QJsonObject& config);
    bool applyCursor(const QString& cursorName, const QJsonObject& config);
    bool uninstallCursor(const QString& cursorName, const QJsonObject& config);

    bool isInstalled(const QJsonObject& config) const;
    bool isSchemeRegistered(const QJsonObject& config) const;
    bool isCurrentlyApplied(const QJsonObject& config) const;

private:
    // Windows 光标注册表项在方案字符串中的顺序
    static const QStringList cursorTypeOrder;

    QString buildSchemeString(const QString& cursorName, const QJsonObject& config) const;
    void writeCursorRegistry(const QString& cursorName, const QJsonObject& config);
    void resetCursorRegistryToDefault();
    void deleteCursorFilesFromSystem(const QString& cursorName, const QJsonObject& config);
};

#endif // CURSORMANAGER_H
