#include "GNSS_desktop_application.h"
#include <QtWidgets/QApplication>
// user include begin

#include <QtWebEngineCore/QWebEngineSettings>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebEngineCore/QWebEngineProfile>
#include <QMainWindow>
// user include end
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    GNSS_desktop_application w;
    w.show();
    return a.exec();
}
