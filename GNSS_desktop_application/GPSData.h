#ifndef GPSDATA_H
#define GPSDATA_H

#include <QString>
#include <QVector>
#include <QDebug> // For debugging output

class SatelliteInfo {
public:
    QString talker_id;
    int sat_id;
    int elevation;
    int azimuth;
    int snr;

    SatelliteInfo() : sat_id(0), elevation(0), azimuth(0), snr(0) {}
    ~SatelliteInfo() {
        // qDebug() << "SatelliteInfo object destroyed.";
    }
};

class GNSSData {
public:
    QString utc_time;
    QString latitude;
    QString ns_indicator;
    QString longitude;
    QString ew_indicator;
    QString speed_over_ground;
    QString date;
    int num_satellites;
    QVector<SatelliteInfo> satellites;  // Changed to QVector

    // Use double instead of long double
    double latitude_dd;  // Decimal degrees latitude
    double longitude_dd; // Decimal degrees longitude

    GNSSData()
        : utc_time(""), latitude(""), ns_indicator(""), longitude(""), ew_indicator(""),
        speed_over_ground(""), date(""), num_satellites(0), latitude_dd(0.0), longitude_dd(0.0) {
        // qDebug() << "GNSSData object created.";
    }

    GNSSData(const GNSSData& other)
        : utc_time(other.utc_time), latitude(other.latitude), ns_indicator(other.ns_indicator),
        longitude(other.longitude), ew_indicator(other.ew_indicator),
        speed_over_ground(other.speed_over_ground), date(other.date),
        num_satellites(other.num_satellites), satellites(other.satellites),
        latitude_dd(other.latitude_dd), longitude_dd(other.longitude_dd) {
        qDebug() << "GNSSData copied.";
    }

    GNSSData& operator=(const GNSSData& other) {
        if (this != &other) {
            utc_time = other.utc_time;
            latitude = other.latitude;
            ns_indicator = other.ns_indicator;
            longitude = other.longitude;
            ew_indicator = other.ew_indicator;
            speed_over_ground = other.speed_over_ground;
            date = other.date;
            num_satellites = other.num_satellites;
            satellites = other.satellites;
            latitude_dd = other.latitude_dd;
            longitude_dd = other.longitude_dd;
            qDebug() << "GNSSData assigned.";
        }
        return *this;
    }

    ~GNSSData() {
        // qDebug() << "GNSSData object destroyed.";
    }

    void clear() {
        utc_time.clear();
        latitude.clear();
        ns_indicator.clear();
        longitude.clear();
        ew_indicator.clear();
        speed_over_ground.clear();
        date.clear();
        num_satellites = 0;
        satellites.clear();
        latitude_dd = 0.0;
        longitude_dd = 0.0;
        // qDebug() << "GNSSData cleared.";
    }

    bool isValid() const {
        return !utc_time.isEmpty() && utc_time != "00:00:00" && latitude_dd != 0.0 && longitude_dd != 0.0;
    }
};

bool verifyChecksum(const QString& sentence);
bool parseGNSSSentence(const QString& sentence, GNSSData& data);
double convertToDecimalDegrees(const QString& coordinate, const QString& direction);

#endif // GPSDATA_H
