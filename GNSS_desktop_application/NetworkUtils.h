#pragma once
#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <QtWebEngineWidgets/QWebEngineView>
#include "ui_GNSS_desktop_application.h"

namespace NetworkUtils {
    void checkInternetQuality(QWebEngineView* webView, Ui::GNSS_desktop_applicationClass& ui);
}

#endif // NETWORKUTILS_H
