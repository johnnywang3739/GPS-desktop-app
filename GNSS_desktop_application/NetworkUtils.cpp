#include "NetworkUtils.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QDebug>
#include "ui_GNSS_desktop_application.h" 

void NetworkUtils::checkInternetQuality(QWebEngineView* webView, Ui::GNSS_desktop_applicationClass& ui) {
    QNetworkAccessManager* networkManager = new QNetworkAccessManager(webView);

    static bool hadInternet = true;  
    QNetworkRequest pingRequest(QUrl("http://1.1.1.1")); 
    QElapsedTimer timer;
    timer.start();

    QNetworkReply* pingReply = networkManager->get(pingRequest);

    QObject::connect(pingReply, &QNetworkReply::finished, [pingReply, webView, networkManager, timer, &ui]() {
        if (pingReply->error() == QNetworkReply::NoError) {
            int pingTime = timer.elapsed();
            qDebug() << "Ping Time:" << pingTime << "ms";
            int signalStrength = 0;
            if (pingTime < 180) {
                signalStrength = 100; // Excellent
            }
            else if (pingTime < 280) {
                signalStrength = 70; // Good
            }
            else if (pingTime < 400) {
                signalStrength = 45; // Fair
            }
            else if (pingTime < 500) {
                signalStrength = 20; // Poor
            }
            else {
                signalStrength = 1; // Very Poor
            }
            QString jsCommand = QString("updateWifiSignalIcon(%1);").arg(signalStrength);
            webView->page()->runJavaScript(jsCommand);

            if (!hadInternet) {  
                ui.infoTextBox->append("Internet connection restored.");
                ui.infoTextBox->clear();
                hadInternet = true; 
            }
        }
        else {
            qDebug() << "Ping failed:" << pingReply->errorString();
            QString jsCommand = "updateWifiSignalIcon(0);";
            webView->page()->runJavaScript(jsCommand);
            ui.infoTextBox->append("No internet! Check network connection!");

            hadInternet = false;  
        }

        pingReply->deleteLater();
        });
}
