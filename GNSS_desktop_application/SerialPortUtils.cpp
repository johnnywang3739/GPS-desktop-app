#include "SerialPortUtils.h"
#include "CSVUtils.h"  // Include CSVUtils for writing data to CSV
#include "ui_GNSS_desktop_application.h"  // Include the UI header

#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

namespace SerialPortUtils {
    void readSerialData(QSerialPort* serial, GNSSData& gnssData, int& updateFrequency, bool& showingRealTimeLocation, bool& recordingData, QFile& csvFile, QWebEngineView* webView, Ui::GNSS_desktop_applicationClass& ui) {
        static QByteArray buffer;
        static int validDataCount = 0;

        buffer += serial->readAll();

        while (true) {
            int startIdx = buffer.indexOf('$');
            if (startIdx == -1) {
                break;
            }
            int endIdx = buffer.indexOf("\r\n", startIdx);
            if (endIdx == -1) {
                break;
            }

            QString sentence = QString::fromUtf8(buffer.mid(startIdx, endIdx - startIdx + 2));
            buffer.remove(0, endIdx + 2);

            if (!verifyChecksum(sentence)) {
                qDebug() << "Checksum verification failed for sentence: " << sentence;
                ui.infoTextBox->clear();  // Clear info box before displaying invalid data
                continue;
            }

            if (parseGNSSSentence(sentence, gnssData)) {
                if (gnssData.isValid()) {
                    validDataCount++;

                    if (validDataCount >= updateFrequency) {
                        validDataCount = 0;

                        QString activeSatellites;
                        int activeSatCount = 0;
                        double totalSNR = 0.0;
                        for (const auto& sat : gnssData.satellites) {
                            if (sat.is_active) {
                                activeSatellites += QString("%1%2, ").arg(sat.talker_id).arg(sat.sat_id);

                                // Use the highest SNR
                                int snr = std::max(sat.snr_1, sat.snr_2);
                                if (snr != -1) {
                                    totalSNR += snr;
                                    activeSatCount++;
                                }
                            }
                        }

                        // Calculate average SNR
                        double avgSNR = (activeSatCount > 0) ? totalSNR / activeSatCount : 0;
                        QString formattedData = QString("Local Time: %1\nLatitude: %2 %3\nLongitude: %4 %5\nSpeed: %6 km/h\nSatellites in view: %7, Active: %8")
                            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                            .arg(QString::number(gnssData.latitude_dd, 'g', 17)).arg(gnssData.ns_indicator)  
                            .arg(QString::number(gnssData.longitude_dd, 'g', 17)).arg(gnssData.ew_indicator)  
                            .arg(gnssData.speed_over_ground.toDouble() * 1.852, 0, 'f', 2)  // Convert knots to km/h
                            .arg(gnssData.num_satellites)
                            .arg(activeSatCount);  // Use the actual counted number

                        formattedData += QString("\nActive Satellites: %1\nAverage SNR: %2 dB")
                            .arg(activeSatellites.trimmed().chopped(1))  // Remove trailing comma
                            .arg(avgSNR, 0, 'f', 2);  // Display the average SNR

                        ui.infoTextBox->clear();  // Clear info box before displaying valid data
                        ui.infoTextBox->setText(formattedData);
                        CSVUtils::writeDataToCsv(formattedData, recordingData, csvFile);
                        if (showingRealTimeLocation) {
                            QString jsCommand = QString("updateMarkerPosition(%1, %2, `%3`, %4);")
                                .arg(QString::number(gnssData.latitude_dd, 'g', 17))  // Full precision for latitude
                                .arg(QString::number(gnssData.longitude_dd, 'g', 17))  // Full precision for longitude
                                .arg(formattedData)  
                                .arg(avgSNR, 0, 'f', 2);  // Pass average SNR for color update
                            webView->page()->runJavaScript(jsCommand);
                        }
                    }
                }
                else {
                    ui.infoTextBox->clear();  
                    QString rawDataMessage = QString("Invalid GNSS data received. Raw data: %1").arg(sentence);
                    ui.infoTextBox->append(rawDataMessage);  // Display the invalid raw data
                }
            }
            else {
                ui.infoTextBox->clear();  
                QString rawDataMessage = QString("Failed to parse GNSS data. Raw data: %1").arg(sentence);
                ui.infoTextBox->append(rawDataMessage);  // Display the invalid raw data if parsing fails
            }
        }

        // Clear buffer if it gets too large
        if (buffer.size() > 4096) {
            buffer.clear();
        }
    }
} // namespace SerialPortUtils
