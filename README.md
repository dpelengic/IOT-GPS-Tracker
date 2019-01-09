# IOT-GPS-Tracker
GPS tracker with Rephone Geo Kit by seeedstudio.com

## About
  
This code runs on Rephone Geo Kit (seeedstudio.com) and an additional Xadow Basic Sensor chip (for accelerometer functionalities).
  
Rephone Geo Kit:  
![GeoKit](https://github.com/diarpi/IOT-GPS-Tracker/blob/master/images/geo_kit.jpg)
![GeoKit2](https://github.com/diarpi/IOT-GPS-Tracker/blob/master/images/geo_kit_2.jpg)
  
Xadow Basic Sensors:  
![XadowBasicSensors](https://github.com/diarpi/IOT-GPS-Tracker/blob/master/images/Xadow_Basic_Sensors.png)
  
This projects primary use case is a car GPS tracker, in case a theft occurs.
  
Device requires a working nano SIM card, without PIN lock and is controled via SMS commands, where only the phone number, matching the hardcoded value, can actually invoke commands successfully.
  
**The following commands are supported:**  
**Status** - returns battery %.  
**Location** - returns UTC DateTime and link to google maps, with longitute and latitude.  
**Fence** - toggles "Geo" Fence functionality (on/off). Since GPS fetching is expensive battery wise, accelerometer is used to detect movement instead.  
  
**TODO:**  
**Reset** - reboot device.  
**Master** - change "master" number.  
  
When "Geo" Fence is enabled, x,y,z accelerometer values are saved until Fence is disabled and compared to new ones every second.   Currently up to +/-200 difference is ignored (feel free to tweak that according to your needs), otherwise alarm is triggered and SMS is sent every 10 seconds to the hardcoded master number.
  
Please follow the device vendor guide on how to set-up the Development environment.
  
Tested on Arduino IDE version (RePhone specific configuration):
```
1.6.0
```
  
Any suggestions are welcome !

## Before starting

Change the "master" number to your own - include the international country code:   
```
...
char mnum[13] = "+11122333444"; // master, control number
...
```
Optionally, tweak the allowed difference (margin) for accelerometer values:   
```
...
int verify_fence(long x, long y, long z) {
  // x,y,z "mismatch" allowed
  long margin = 200;
  ...
```
   
## Example usage



Check battery %:
```
SND -- Status
REC -- Battery Level 100.
```

Request current location:
```
SND -- Location
REC -- 
UTC DateTime: 17-5-31 20:00:00
Location https://maps.google.com/maps?q=4X.XXXXXX,1X.XXXXXX
```

Toggle "Geo" Fence functionality:
```
SND -- Fence
REC -- Fence 1 / 0
```

"Geo" Fence detects movement/shaking:
```
REC -- ALARM! MOVEMENT DETECTED!
```
  

