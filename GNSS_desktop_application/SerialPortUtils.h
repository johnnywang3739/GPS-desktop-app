#pragma once

#include <QSerialPort>
#include <QFile>
#include <QtWebEngineWidgets/QWebEngineView>
#include "GPSData.h"

// Forward declaration of the UI class
namespace Ui {
    class GNSS_desktop_applicationClass;
}

namespace SerialPortUtils {
    void readSerialData(QSerialPort* serial, GNSSData& gnssData, int& updateFrequency, bool& showingRealTimeLocation, bool& recordingData, QFile& csvFile, QWebEngineView* webView, Ui::GNSS_desktop_applicationClass& ui);
}
