#ifndef TRANSLATIONMANAGER_H
#define TRANSLATIONMANAGER_H

#include <QObject>
#include <QTranslator>

class TranslationManager : public QObject
{
    Q_OBJECT
public:
    static TranslationManager& instance();
    void init();
    void switchLanguage(const QString& language);

private:
    explicit TranslationManager(QObject* parent = nullptr);
    ~TranslationManager();
    Q_DISABLE_COPY_MOVE(TranslationManager)

    QTranslator* _translator = nullptr;
    bool _loaded = false;
};

#endif // TRANSLATIONMANAGER_H
