# Longan Nano with u-blox NEO-6M

Connect a u-blox NEO-6M module with a Longan Nano and display and record UTC time and position data

<img src="img.jpg" alt="" width="500px"/> todo: update image

## Compilation

A ```Longan Nano``` is required, the ```Longan Nano Lite``` has not enough memory.
Use the PlatformIO environment option to select which USART the GPS module is connected to: ```-e usart0``` or ```-e usart1```.

```
pio run -e usart0 -t upload
```

## Usage

The module has three screens: *GPS Position*, *Satellite Info* and *Logging*. Use a short button press to toggle between *GPS Position* screen and *Satellite Info* screen. A long button press will select the *Logging* screen. Another long button press will start / stop logging to the SD card.

## Logging

The logging will start when the UTC time and GPS position are found. The file name will be the start UTC time with the extension csv.
Always stop logging before power off, else data will be lost and the file system might get corrupted.

The Logging Indicator, the 2nd pixel at the top right of the screen, shows the logging state:
- red: logging off
- green: logging data
- yellow: waiting for data (GPS position or UTC time)

The logfile format is compatible with [GPSBabel](http://www.gpsbabel.org/index.html)'s unicsv format. GPSBabel can convert the logfile to many different formats (try ```gpsbabel -h``` for a full list of formats). E.g. to create a kml file for import in Google Earth use the command:

```gpsbabel -i unicsv -f 'yyymmdd hhmmss.csv' -o kml -F 'yyymmdd hhmmss.kml'```

## 3D Print STL files

todo: In the hw/ folder ... (usually blue) board [size] ... square antenna [size] ...