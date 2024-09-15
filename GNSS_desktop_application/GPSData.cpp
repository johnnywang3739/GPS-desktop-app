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
    if (!sentence.startsWith('$') || !sentence.contains('*')) {
        return false;
    }

    int asteriskIndex = sentence.indexOf('*');
    if (asteriskIndex == -1 || asteriskIndex + 2 >= sentence.size()) {
        return false; // Invalid format or no checksum
    }

    // Extract the data part (everything between '$' and '*')
    QString dataPart = sentence.mid(1, asteriskIndex - 1); // Skip the '$'

    // Extract the received checksum (two characters after '*')
    QString receivedChecksum = sentence.mid(asteriskIndex + 1, 2);

    // Calculate the checksum
    int calculatedChecksum = 0;
    for (QChar c : dataPart) {
        calculatedChecksum ^= c.unicode();
    }

    // Convert the calculated checksum to a 2-character uppercase hexadecimal string
    QString calculatedChecksumStr = QString("%1").arg(calculatedChecksum, 2, 16, QLatin1Char('0')).toUpper();

    // Compare the calculated checksum with the received checksum
    return calculatedChecksumStr == receivedChecksum;
}
bool parseGNSSSentence(const QString& sentence, GNSSData& data) {
    if (!verifyChecksum(sentence)) {
        qDebug() << "Checksum verification failed.";
        return false;  // Checksum failed
    }

    // Clear previous data
    data.clear();

    QString content = sentence.mid(1, sentence.indexOf('*') - 1);
    QStringList fields = content.split(',');

    if (fields.size() < 8) {
        qDebug() << "Not enough fields to parse GNSS data.";
        return false;
    }

    // Parse basic GNSS data
    data.utc_time = fields[0];
    data.latitude = fields[1];
    data.ns_indicator = fields[2];
    data.longitude = fields[3];
    data.ew_indicator = fields[4];
    data.speed_over_ground = fields[5];
    data.date = fields[6];

    // Convert latitude and longitude to decimal degrees
    data.latitude_dd = convertToDecimalDegrees(data.latitude, data.ns_indicator);
    data.longitude_dd = convertToDecimalDegrees(data.longitude, data.ew_indicator);

    bool ok;
    data.num_satellites = fields[7].toInt(&ok);
    if (!ok) {
        qDebug() << "Invalid number of satellites.";
        return false;
    }

    int index = 8;
    // Parse satellite data (in view)
    for (int i = 0; i < data.num_satellites && (index + 6) < fields.size(); i++) {
        SatelliteInfo satInfo;
        satInfo.talker_id = fields[index++];
        satInfo.sat_id = fields[index++].toInt(&ok);
        if (!ok) continue;
        satInfo.elevation = fields[index++].toInt(&ok);
        if (!ok) continue;
        satInfo.azimuth = fields[index++].toInt(&ok);
        if (!ok) continue;
        satInfo.signal_1_id = fields[index++];    // First signal ID (e.g., L1, E5a)
        satInfo.snr_1 = fields[index++].toInt(&ok); // First signal SNR
        if (!ok) continue;

        satInfo.signal_2_id = fields[index++];    // Second signal ID (e.g., L5, E1)
        satInfo.snr_2 = fields[index++].toInt(&ok); // Second signal SNR (-1 if missing)
        if (!ok) satInfo.snr_2 = -1;

        data.satellites.push_back(satInfo);
    }

    // Parse total active satellite count (received at the beginning of the GSA section)
    if (index < fields.size()) {
        data.total_active_satellites = fields[index++].toInt(&ok);  // Parse total active satellites
        if (!ok) {
            qDebug() << "Invalid total active satellites.";
            return false;
        }
    }

    // Parse active satellite data (GP, GL, GA, GB) and DOP values
    while (index < fields.size()) {
        QString talker_id = fields[index++];
        QVector<int> active_sat_ids;

        while (index < fields.size() && fields[index].toInt(&ok)) {
            active_sat_ids.push_back(fields[index++].toInt(&ok));
            if (!ok) break;
        }

        QString active_sat_ids_str;
        for (int i = 0; i < active_sat_ids.size(); ++i) {
            active_sat_ids_str += QString::number(active_sat_ids[i]);
            if (i < active_sat_ids.size() - 1) {
                active_sat_ids_str += ", ";
            }
        }
        if (index + 2 < fields.size()) {
            QString pdop = fields[index++];
            QString hdop = fields[index++];
            QString vdop = fields[index++];
            bool anyMarkedActive = false; 
            for (auto& sat : data.satellites) {
                if (sat.talker_id == talker_id && active_sat_ids.contains(sat.sat_id)) {
                    sat.is_active = true;
                    sat.pdop = pdop;
                    sat.hdop = hdop;
                    sat.vdop = vdop;

                }
            }


        }
    }

    return true;
}
