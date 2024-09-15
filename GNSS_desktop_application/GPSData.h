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
    QString signal_1_id;  // First Signal ID (e.g., L1, E5a)
    int snr_1;            // SNR for the first signal
    QString signal_2_id;  // Second Signal ID (e.g., L5, E1)
    int snr_2;            // SNR for the second signal (-1 if not present)
    bool is_active;
    QString pdop;
    QString hdop;
    QString vdop;

    SatelliteInfo()
        : sat_id(0), elevation(0), azimuth(0), snr_1(0), snr_2(-1), is_active(false), pdop(""), hdop(""), vdop("") {}
    ~SatelliteInfo() {}
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
    int num_satellites;  // Number of satellites in view
    QVector<SatelliteInfo> satellites;  // Stores both satellites in view and active ones
    int total_active_satellites;        // Total active satellites

    // Use double instead of long double
    double latitude_dd;  // Decimal degrees latitude
    double longitude_dd; // Decimal degrees longitude

    GNSSData()
        : utc_time(""), latitude(""), ns_indicator(""), longitude(""), ew_indicator(""),
        speed_over_ground(""), date(""), num_satellites(0), latitude_dd(0.0), longitude_dd(0.0),
        total_active_satellites(0) {}

    GNSSData(const GNSSData& other)
        : utc_time(other.utc_time), latitude(other.latitude), ns_indicator(other.ns_indicator),
        longitude(other.longitude), ew_indicator(other.ew_indicator),
        speed_over_ground(other.speed_over_ground), date(other.date),
        num_satellites(other.num_satellites), satellites(other.satellites),
        total_active_satellites(other.total_active_satellites),
        latitude_dd(other.latitude_dd), longitude_dd(other.longitude_dd) {}

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
            total_active_satellites = other.total_active_satellites;
            latitude_dd = other.latitude_dd;
            longitude_dd = other.longitude_dd;
        }
        return *this;
    }

    ~GNSSData() {}

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
        total_active_satellites = 0;
        latitude_dd = 0.0;
        longitude_dd = 0.0;
    }

    bool isValid() const {
        return !utc_time.isEmpty() && utc_time != "00:00:00" && latitude_dd != 0.0 && longitude_dd != 0.0;
    }
};

bool verifyChecksum(const QString& sentence);
bool parseGNSSSentence(const QString& sentence, GNSSData& data);
double convertToDecimalDegrees(const QString& coordinate, const QString& direction);

#endif // GPSDATA_H
