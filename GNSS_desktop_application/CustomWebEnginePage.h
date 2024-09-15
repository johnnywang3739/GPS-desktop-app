#pragma once
#include <QDebug>
#include <QtWebEngineCore/QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebEngineCore/QWebEnginePage>

class CustomWebEnginePage : public QWebEnginePage
{
    Q_OBJECT

public:
    explicit CustomWebEnginePage(QObject* parent = nullptr) : QWebEnginePage(parent) {}

protected:
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID) override
    {
        Q_UNUSED(level)
            qDebug() << "JavaScript console message:" << message << "at line" << lineNumber << "from" << sourceID;
    }
};
