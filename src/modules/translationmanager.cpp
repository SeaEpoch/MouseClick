#include "translationmanager.h"

#include <QCoreApplication>

TranslationManager& TranslationManager::instance()
{
    static TranslationManager instance;
    return instance;
}

TranslationManager::TranslationManager(QObject* parent)
    : QObject{parent}
{}

TranslationManager::~TranslationManager()
{
    if (_loaded && qApp) {
        QCoreApplication::removeTranslator(_translator);
    }
}

void TranslationManager::init()
{
    _translator = new QTranslator(this);
}

void TranslationManager::switchLanguage(const QString& language)
{
    const QString qmPath = ":/i18n/MouseClick_" + language;

    if (!_loaded) {
        _loaded = _translator->load(qmPath);
        if (_loaded) {
            QCoreApplication::installTranslator(_translator);
        }
    } else {
        QCoreApplication::removeTranslator(_translator);
        QTranslator* newTranslator = new QTranslator(this);
        if (newTranslator->load(qmPath)) {
            delete _translator;
            _translator = newTranslator;
        } else {
            delete newTranslator;
        }
        QCoreApplication::installTranslator(_translator);
    }
}
