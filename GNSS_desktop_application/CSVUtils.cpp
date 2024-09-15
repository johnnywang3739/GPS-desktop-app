#include "CSVUtils.h"
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
void CSVUtils::writeDataToCsv(const QString& data, bool recordingData, QFile& csvFile) {
    if (recordingData && csvFile.isOpen()) {
        QTextStream out(&csvFile);
        QStringList dataFields = data.split('\n');

        QStringList csvRow;
        QDateTime utcDateTime = QDateTime::fromString(dataFields.value(0).split(": ").value(1).trimmed(), "yyyy-MM-dd hh:mm:ss");
        QString dateStr = utcDateTime.toString("yyyy-MM-dd");
        QString timeStr = utcDateTime.toString("hh:mm:ss");

        if (!dateStr.isEmpty() && !timeStr.isEmpty()) {
            csvRow << dateStr << timeStr;
        }
        else {
            qDebug() << "Error: Invalid date or time format.";
            return;  // Stop writing if date or time is invalid
        }

        // Extract latitude and longitude with full precision (17 decimal places)
        QString latitudeField = dataFields.value(1).split(": ").value(1).trimmed();
        QString latitude = latitudeField.split(" ").value(0);
        QString nsIndicator = latitudeField.split(" ").value(1);
        csvRow << QString::number(latitude.toDouble(), 'f', 17) << nsIndicator;

        QString longitudeField = dataFields.value(2).split(": ").value(1).trimmed();
        QString longitude = longitudeField.split(" ").value(0);
        QString ewIndicator = longitudeField.split(" ").value(1);
        csvRow << QString::number(longitude.toDouble(), 'f', 17) << ewIndicator;
        QString speedKmh = dataFields.value(3).split(": ").value(1).trimmed().split(" ").value(0);
        csvRow << speedKmh;
        QString numSatellitesInView = dataFields.value(4).split(": ").value(1).trimmed().split(",").value(0);
        csvRow << numSatellitesInView;
        QString numActiveSatellites = dataFields.value(4).split(": ").value(2).trimmed();
        csvRow << numActiveSatellites;
        QString activeSatellites = dataFields.value(5).split(": ").value(1).trimmed();
        activeSatellites.replace(",", ";");  // Replace commas with semicolons
        csvRow << activeSatellites;

        QString SNR = dataFields.value(6).split(": ").value(1).trimmed();
        csvRow << SNR;

        out << csvRow.join(",") << "\n";
    }
}
QString CSVUtils::createMarkerTitle(const GNSSData& gnssData) {
    QString formattedData;

    formattedData += "Date: " + gnssData.date + "\n";
    formattedData += "Time: " + gnssData.utc_time + "\n";
    formattedData += "Latitude: " + QString::number(gnssData.latitude_dd, 'g', 17) + " " + gnssData.ns_indicator + "\n";
    formattedData += "Longitude: " + QString::number(gnssData.longitude_dd, 'g', 17) + " " + gnssData.ew_indicator + "\n";
    formattedData += "Speed: " + QString::number(gnssData.speed_over_ground.toDouble(), 'f', 2) + " km/h\n";
    formattedData += "Satellites in view: " + QString::number(gnssData.num_satellites) + ", Active: " + QString::number(gnssData.total_active_satellites) + "\n";
    QString activeSatellites;
    for (const auto& sat : gnssData.satellites) {
        if (sat.is_active) {
            activeSatellites += sat.talker_id + QString::number(sat.sat_id) + "; "; 
        }
    }

    if (!activeSatellites.isEmpty()) {
        formattedData += "Active Satellites: " + activeSatellites.trimmed().chopped(1) + "\n"; 
    }
    else {
        formattedData += "Active Satellites: None\n";
    }

    if (gnssData.satellites.size() > 0) {
        QString SNR = QString::number(gnssData.satellites[0].snr_1, 'f', 2); 
        formattedData += "Average SNR: " + SNR + " dB\n";
    }
    else {
        formattedData += "Average SNR: N/A\n";
    }

    return formattedData;
}
