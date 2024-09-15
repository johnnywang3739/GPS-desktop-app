# GPS Viewer Application

This GNSS Desktop Application is a tool for viewing real-time and historical GPS data on a Google Maps interface. It allows users to connect to a GNSS device via a serial port, visualize real-time location data, load historical GPS data from CSV files, and manage data display on the map.

## Features

- **Real-Time GPS Data Visualization**: Connect to a GNSS device using a serial port and visualize real-time location updates on a Google Map.
- **GPS Data Recording**: Record GPS data in real-time to a CSV file for future reference and analysis.
- **Historical Data Loading**: Load historical GPS data from CSV files and display them as permanent markers on the map.
- **Marker Customization**: Real-time and historical markers are displayed with color coding based on the signal-to-noise ratio (SNR) value.
- **Wi-Fi Signal Indicator**: Visual indicator of the Wi-Fi signal strength within the application.
- **Control Panel**: Includes options to start/stop recording, load historical data, toggle real-time location display, and refresh the map.
- **Input Validation and Safety Measures**: The application includes safeguards to prevent users from entering incorrect information that may cause the application to crash, enhancing stability and user experience.


## Code Structure

The application is composed of several key components, each defined in separate C++ files:

- **CSVUtils.cpp**: Provides utilities for handling CSV files, such as writing GNSS data to CSV and generating titles for map markers based on the data.
  
- **GNSS_desktop_application.cpp**: Contains the main application logic, including UI setup, serial port communication, real-time data processing, and interaction with the Google Maps interface.

- **GPSData.cpp**: Defines data structures and functions for parsing and handling GNSS sentences, converting coordinates, and verifying data integrity.

- **main.cpp**: The entry point of the application, which initializes and runs the Qt application.

- **NetworkUtils.cpp**: Handles network-related functionality, such as checking Wi-Fi signal strength and updating the Wi-Fi signal indicator on the map.

- **SerialPortUtils.cpp**: Manages serial port communication, reading data from the GNSS device, and processing it for display and recording.

## Requirements

- **Visual Studio**: 2022 Community Version
- **Qt**: Qt 6.7.2, including Qt Widgets, Qt WebEngine, and Qt Serial Port modules.
- **Google Maps API Key**: A valid Google Maps JavaScript API key is required to be added to the google_maps.html before running the code.
- **GNSS Device**: Quectel LC29H with STM32 as a transmitter that formats the each NMEA data pulse into one sentence.


### Set Up Development Environment 

#### C/C++ General

Add the following include directories in your project settings:

- `C:\Qt\6.7.2\msvc2019_64\include\QtWebEngineWidgets`
- `C:\Qt\6.7.2\msvc2019_64\include\QtSerialPort`

#### Linker Settings

Add the following directories to the **Linker** -> **Additional Library Directories**:

- `C:\Qt\6.7.2\msvc2019_64\lib`
- `%(AdditionalLibraryDirectories)`

Add the following libraries to the **Linker** -> **Input** -> **Additional Dependencies**:

- `Qt6WebEngineWidgets.lib`
- `Qt6WebEngineCore.lib`
- `Qt6WebChannel.lib`
- `Qt6Core.lib`
- `Qt6Gui.lib`
- `Qt6Widgets.lib`
- `Qt6SerialPort.lib`
- `Qt6Network.lib`

#### Post-Build Events

Set up the following post-build events to copy necessary HTML files to the output directory:

```shell
xcopy /Y "$(ProjectDir)google_maps.html" "$(OutDir)*"
xcopy /Y "$(ProjectDir)test.html" "$(OutDir)*"
```

####  Troubleshooting

Debug Mode Crash: If you experience crashes in debug mode but not in release mode, this may be related to the Qt version or specific settings. Refer to the discussion on the Qt Forum for potential solutions.

## Installer Creation

To create an installer for the GNSS Desktop Application, you can use InnoSetup Compiler 6.3.3. The InnoSetup script file (`.iss`) is provided in the installer folder of the project.

### Steps to Compile the Installer

1. **Download and Install InnoSetup Compiler**: 
   - You can download InnoSetup Compiler 6.3.3 from the [official website](https://jrsoftware.org/isinfo.php).
   
2. **Open the InnoSetup Script File**:
   - Launch the InnoSetup Compiler.
   - Open the `.iss` file located in the `installer` folder of the project.

3. **Compile the Script**:
   - Click on the "Compile" button in the InnoSetup Compiler to generate the installer.
   - The compiled installer executable will be created in the specified output directory.

4. **Distribute the Installer**:
   - You can now distribute the generated installer executable to install the GNSS Desktop Application on other machines.
