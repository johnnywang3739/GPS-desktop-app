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
            int endIdx = buffer.indexOf("&\r\n", startIdx);

            if (startIdx == -1 || endIdx == -1) {
                break;
            }

            QString sentence = QString::fromUtf8(buffer.mid(startIdx, endIdx - startIdx + 3));
            buffer.remove(0, endIdx + 3);

            if (!verifyChecksum(sentence)) {
                continue;
            }

            if (parseGNSSSentence(sentence, gnssData)) {
                if (gnssData.utc_time.isEmpty() || gnssData.latitude_dd == 0.0 || gnssData.longitude_dd == 0.0) {
                    ui.infoTextBox->clear();
                    ui.infoTextBox->append("Invalid data received, check GPS antenna connection!");
                    ui.infoTextBox->append("Raw Data: " + sentence); 
                    if (showingRealTimeLocation) {
                        QString jsCommand = "hideMarker();";
                        webView->page()->runJavaScript(jsCommand);
                    }

                    ui.showRealTimeButton->setEnabled(false);
                    ui.toggleRecordingButton->setEnabled(false);
                    showingRealTimeLocation = false;
                    continue;
                }
                else {
                    ui.showRealTimeButton->setEnabled(true);
                    ui.toggleRecordingButton->setEnabled(true);
                }

                if (gnssData.num_satellites > 0) {
                    validDataCount++;

                    if (validDataCount >= updateFrequency) {
                        validDataCount = 0;

                        int totalSatellites = gnssData.satellites.size();
                        double totalSNR = 0.0;

                        for (const auto& satellite : gnssData.satellites) {
                            totalSNR += satellite.snr;
                        }

                        double averageSNR = (totalSatellites > 0) ? totalSNR / totalSatellites : 0.0;

                        QString formattedData;

                        QString dateStr = gnssData.date;
                        QString yearStr = dateStr.mid(4, 2);
                        int year = yearStr.toInt();
                        year += (year < 70) ? 2000 : 1900;

                        QDateTime utcDateTime = QDateTime::fromString(dateStr.left(4) + QString::number(year) + gnssData.utc_time, "ddMMyyyyhhmmss.zzz");
                        utcDateTime.setTimeSpec(Qt::UTC);
                        QDateTime localDateTime = utcDateTime.toLocalTime();

                        double speed_knots = gnssData.speed_over_ground.toDouble();
                        double speed_kmh = speed_knots * 1.852;
                        double speed_mph = speed_knots * 1.15078;

                        formattedData = QString("Local Date & Time: %1\nLatitude: %2 %3\nLongitude: %4 %5\nSpeed Over Ground: %6 knots, %7 km/h, %8 mph\nNumber of Satellites: %9\nAverage SNR: %10")
                            .arg(localDateTime.toString("yyyy-MM-dd hh:mm:ss"))
                            .arg(QString::number(gnssData.latitude_dd, 'g', 17))
                            .arg(gnssData.ns_indicator)
                            .arg(QString::number(gnssData.longitude_dd, 'g', 17)) 
                            .arg(gnssData.ew_indicator)
                            .arg(speed_knots, 0, 'f', 2)
                            .arg(speed_kmh, 0, 'f', 2)
                            .arg(speed_mph, 0, 'f', 2)
                            .arg(totalSatellites)
                            .arg(averageSNR, 0, 'f', 2);


                        ui.infoTextBox->clear();
                        ui.infoTextBox->append(formattedData);

                        CSVUtils::writeDataToCsv(formattedData, recordingData, csvFile);

                        if (showingRealTimeLocation) {
                            QString jsFormattedData = formattedData;
                            jsFormattedData.replace("\n", "\\n").replace("'", "\\'");
                            QString jsCommand = QString("updateMarkerPosition(%1, %2, `%3`, %4);")
                                .arg(gnssData.latitude_dd)
                                .arg(gnssData.longitude_dd)
                                .arg(jsFormattedData)
                                .arg(averageSNR, 0, 'f', 2); 
                            webView->page()->runJavaScript(jsCommand);
                        }

                        if (showingRealTimeLocation) {
                            ui.toggleRecordingButton->setEnabled(true);
                        }
                    }
                }
            }
            else {
                qDebug() << "Failed to parse GNSS sentence.";
            }
        }

        if (buffer.size() > 4096) {
            buffer.clear();
        }
    }

} // namespace SerialPortUtils
