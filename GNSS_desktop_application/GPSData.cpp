#include "GPSData.h"
#include <QStringList>

#include <cstdlib> // For std::strtold

double convertToDecimalDegrees(const QString& coord, const QString& direction) {
    if (coord.isEmpty() || direction.isEmpty()) return 0.0;

    bool ok;
    double decimalDegrees = coord.mid(0, (direction == "N" || direction == "S") ? 2 : 3).toDouble(&ok);
    if (!ok) return 0.0;

    double minutes = coord.mid((direction == "N" || direction == "S") ? 2 : 3).toDouble(&ok);
    if (!ok) return 0.0;

    decimalDegrees += minutes / 60.0;  // Divide minutes by 60 to get the decimal part

    if (direction == "S" || direction == "W") {
        decimalDegrees = -decimalDegrees;  // Negate for south or west coordinates
    }

    return decimalDegrees;
}




bool verifyChecksum(const QString& sentence) {
    if (!sentence.startsWith('$') || !sentence.contains('*') || !sentence.endsWith("&\r\n")) {
        return false;
    }

    int asteriskIndex = sentence.indexOf('*');
    if (asteriskIndex == -1 || asteriskIndex + 2 >= sentence.size()) {
        return false; // Invalid format or no checksum
    }

    QString dataPart = sentence.mid(1, asteriskIndex - 1); // Skip the '$'
    QString receivedChecksum = sentence.mid(asteriskIndex + 1, 2); // Two characters after '*'

 
    int calculatedChecksum = 0;
    for (QChar c : dataPart) {
        calculatedChecksum ^= c.unicode();
    }

    QString calculatedChecksumStr = QString("%1").arg(calculatedChecksum, 2, 16, QLatin1Char('0')).toUpper();

    return calculatedChecksumStr == receivedChecksum;
}

bool parseGNSSSentence(const QString& sentence, GNSSData& data) {
    if (!verifyChecksum(sentence)) {
        qDebug() << "Checksum verification failed.";
        return false; // Checksum failed
    }

    data.clear();

    QString content = sentence.mid(1, sentence.indexOf('*') - 1); // Extract content between '$' and '*'
    QStringList fields = content.split(',');

    if (fields.size() < 7) {
        qDebug() << "Not enough fields to parse GNSS data.";
        return false; // Minimum fields to parse
    }


    data.utc_time = fields[0];
    data.latitude = fields[1];
    data.ns_indicator = fields[2];
    data.longitude = fields[3];
    data.ew_indicator = fields[4];
    data.speed_over_ground = fields[5];
    data.date = fields[6];

    data.latitude_dd = convertToDecimalDegrees(data.latitude, data.ns_indicator);
    data.longitude_dd = convertToDecimalDegrees(data.longitude, data.ew_indicator);

    // Check if there is satellite data
    if (fields.size() > 8) {
        bool ok;
        data.num_satellites = fields[7].toInt(&ok);
        if (!ok) {
            qDebug() << "Invalid number of satellites.";
            return false;
        }

        data.satellites.reserve(data.num_satellites);

        for (int i = 8; i + 4 < fields.size(); i += 5) {
            SatelliteInfo satInfo;

            satInfo.talker_id = fields[i];
            satInfo.sat_id = fields[i + 1].toInt(&ok);
            if (!ok) {
                qDebug() << "Invalid satellite ID:" << fields[i + 1];
                continue; // Skip this satellite but try to parse the next
            }

            satInfo.elevation = fields[i + 2].toInt(&ok);
            if (!ok) {
                qDebug() << "Invalid elevation:" << fields[i + 2];
                continue;
            }

            satInfo.azimuth = fields[i + 3].toInt(&ok);
            if (!ok) {
                qDebug() << "Invalid azimuth:" << fields[i + 3];
                continue;
            }

            satInfo.snr = fields[i + 4].toInt(&ok);
            if (!ok) {
                qDebug() << "Invalid SNR:" << fields[i + 4];
                continue;
            }

            data.satellites.push_back(satInfo);
            // qDebug() << "Appended satellite:" << satInfo.talker_id << satInfo.sat_id;
        }
    }

    // qDebug() << "Exiting parseGNSSSentence function.";
    // qDebug() << "GNSSData object state:";
    // qDebug() << "  Satellites count: " << data.satellites.size();
    // for (const auto& sat : data.satellites) {
    //     qDebug() << "  Satellite info:" << sat.talker_id << sat.sat_id << sat.elevation << sat.azimuth << sat.snr;
    // }

    return true;
}
