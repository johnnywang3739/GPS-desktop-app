# NMEA Message Parser for Quectel LC29H

This software module is specifically designed to parse NMEA sentences from the Quectel LC29H GNSS module. It supports a wide range of NMEA sentence formats to effectively extract and manage GPS and satellite data.

## Features

- **Comprehensive Sentence Support:** Capable of parsing GSV, GGA, RMC, GLL, VTG, and GSA sentences.
- **Advanced Memory Management:** Dynamically handles data for multiple satellites, ensuring efficient performance.
- **Enhanced Parsing Capability:** Offers robust extraction of information from each NMEA sentence, outperforming alternatives like TinyGPS by providing detailed satellite information. [See TinyGPS](https://github.com/mikalhart/TinyGPS).

## Supported NMEA Sentences

- **GSV (Satellites in View):** Details on satellites currently visible to the receiver.
- **GGA (GPS Fix Data):** Information about the current fix including location, altitude, and time.
- **RMC (Recommended Minimum):** Essential GPS pvt (position, velocity, time) data.
- **GLL (Geographic Position):** Latitude and longitude information.
- **VTG (Track Made Good):** Speed and course over the ground.
- **GSA (Satellite Status):** Overall satellite reception quality and DOP values.

## Usage

To integrate this parser into your project, include `GPSUtils.h`, and use the provided functions to decode NMEA sentences:

```c
#include "GPSUtils.h"

GSV_Data gsvData;
parseGSV("GSV sentence", &gsvData);
