Measure Soil Moisture with this capacitive, non-corroding sensor. Temperature and Humidity is available via the on board DHT11. Data is published to a web server hosted on the sensor. 

Esp32_SoilMoisture_WebServer.ino should be use if no micro sd card module is attached.
Esp32_SoilMoisture_WebServer_DataLog.ino requires an micro sd card and continues internet access to the NTP server. This option has very accurate time, but use large amounts of current and may deplete the battery.
Esp32_SoilMoisture_WebServer_DataLog_Int_RTC.ino requires an micro sd card and internet access to the NTP server after reset. It uses the internal RTC of the ESP32 to update the date/time received at reset from the NTP server. This is the most power efficient solution, but the time may not be as accurate.

Full build instructions on https://www.instructables.com/id/ESP32-WiFi-SOIL-MOISTURE-SENSOR/

I am a hobbyist, not a ingineer or programmer. The code is nothing but efficient. Feel free to improve.

I am repurposing this sensor as a siphoning rain gauge. The code and building instructions will soon follow.
https://youtu.be/N8aYW2bDOl4
