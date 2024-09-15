#pragma once
#ifndef CSVUTILS_H
#define CSVUTILS_H

#include "GPSData.h"
#include <QString>
#include <QFile>

namespace CSVUtils {
    void writeDataToCsv(const QString& data, bool recordingData, QFile& csvFile);
    QString createMarkerTitle(const GNSSData& gnssData);
}

#endif // CSVUTILS_H
