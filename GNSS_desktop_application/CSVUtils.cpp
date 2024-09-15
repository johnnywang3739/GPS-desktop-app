#include "CSVUtils.h"
#include <QTextStream>
#include <QDateTime>

void CSVUtils::writeDataToCsv(const QString& data, bool recordingData, QFile& csvFile) {
    if (recordingData && csvFile.isOpen()) {
        QTextStream out(&csvFile);
        QString dataCopy = data; 
        QStringList dataFields = dataCopy.split('\n');

        QStringList csvRow;
        QDateTime utcDateTime = QDateTime::fromString(dataFields[0].split(": ")[1].trimmed(), "yyyy-MM-dd hh:mm:ss");
        QString dateStr = utcDateTime.toString("yyyy-MM-dd");
        QString timeStr = utcDateTime.toString("hh:mm:ss");
        csvRow << dateStr << timeStr;
        QString latitudeField = dataFields[1].split(": ")[1].trimmed();
        QString latitude = latitudeField.split(" ")[0];
        QString nsIndicator = latitudeField.split(" ")[1];
        csvRow << latitude << nsIndicator;
        QString longitudeField = dataFields[2].split(": ")[1].trimmed();
        QString longitude = longitudeField.split(" ")[0];
        QString ewIndicator = longitudeField.split(" ")[1];
        csvRow << longitude << ewIndicator;
        QString speedKnots = dataFields[3].split(": ")[1].trimmed().split(" ")[0];
        QString speedKmh = dataFields[3].split(", ")[1].trimmed().split(" ")[0];
        QString speedMph = dataFields[3].split(", ")[2].trimmed().split(" ")[0];
        csvRow << speedKnots << speedKmh << speedMph;
        QString numSatellites = dataFields[4].split(": ")[1].trimmed();
        csvRow << numSatellites;
        QString avgSNR = dataFields[5].split(": ")[1].trimmed();
        csvRow << avgSNR;
        out << csvRow.join(",") << "\n";
    }
}

QString CSVUtils::createMarkerTitle(const GNSSData& gnssData) {
    QString formattedData;
    QString dateStr = gnssData.date;
    QString yearStr = dateStr.mid(4, 2);
    int year = yearStr.toInt();
    year += (year < 70) ? 2000 : 1900; // Adjust the century
    QDateTime utcDateTime = QDateTime::fromString(dateStr.left(4) + QString::number(year) + gnssData.utc_time, "ddMMyyyyhhmmss.zzz");
    utcDateTime.setTimeSpec(Qt::UTC);
    QDateTime localDateTime = utcDateTime.toLocalTime();
    double speed_knots = gnssData.speed_over_ground.toDouble();
    double speed_kmh = speed_knots * 1.852;
    double speed_mph = speed_knots * 1.15078;

    formattedData += "Local Date & Time: " + localDateTime.toString("yyyy-MM-dd hh:mm:ss") + "\n";
    formattedData += "Latitude: " + QString::number(gnssData.latitude_dd, 'f', 6) + " " + gnssData.ns_indicator + "\n";
    formattedData += "Longitude: " + QString::number(gnssData.longitude_dd, 'f', 6) + " " + gnssData.ew_indicator + "\n";
    formattedData += "Speed Over Ground: " + QString::number(speed_knots, 'f', 2) + " knots, " +
        QString::number(speed_kmh, 'f', 2) + " km/h, " +
        QString::number(speed_mph, 'f', 2) + " mph\n";

    formattedData += "Number of Satellites: " + QString::number(gnssData.num_satellites) + "\n";
    formattedData += "Average SNR: " + QString::number(gnssData.satellites.size() > 0 ? gnssData.satellites[0].snr : 0, 'f', 2) + "\n";

    return formattedData;
}
