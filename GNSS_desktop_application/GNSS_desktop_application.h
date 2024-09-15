#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_GNSS_desktop_application.h"

// User includes begin
#include <QSerialPort>
#include <QWebEngineView>
#include <QProcess>
#include <QRegularExpression>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include "GPSData.h"
#include "CSVUtils.h"
#include "NetworkUtils.h"
#include "SerialPortUtils.h"  
// User includes end

class GNSS_desktop_application : public QMainWindow
{
    Q_OBJECT

public:
    explicit GNSS_desktop_application(QWidget* parent = nullptr);
    ~GNSS_desktop_application();

    void changeLocation(double lat, double lng, double SNR);
protected:
    void closeEvent(QCloseEvent* event) override; // Ensure this line is present and protected

private slots:
    void onConnectButtonClicked();
    void onShowRealTimeButtonClicked();
    void onToggleRecordingButtonClicked(); 
    void onLoadHistoricalDataButtonClicked();
    void onClearMapButtonClicked();
    //void onClearDataButtonClicked();
    void onBrowseButtonClicked();


signals:
    void locationChanged(double lat, double lng, double SNR);

private:
    Ui::GNSS_desktop_applicationClass ui;

    QSerialPort* serial;
    QWebEngineView* webView;
    int baudRate;
    QString comPort;
    QString fileLocation;
    int updateFrequency;
    bool showingRealTimeLocation;
    bool recordingData;  

    QFile csvFile; 
    GNSSData gnssData;  

};
