
//Micro SD Card Module Things
/*
 * Connect the SD card to the following pins:
 *
 * SD Card      | ESP3`2
 *    DAT2        -
 *    DAT3      SS //GPIO5
 *    CMD       MOSI  //GPIO23
 *    VSS       GND
 *    VDD       3.3V
 *    CLK       SCK //GPIO18
 *    VSS       GND
 *    DAT0      MISO  //GPIO19
 *    DAT1      -
 */
#include "FS.h"
#include "SD.h"
#include "SPI.h"

//ESP32 WiFi things
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = "xxxxxx";
const char* password = "xxxxxxx";
int port=80;

WebServer server(port);

const int led = LED_BUILTIN;

//DHT11 things
#include "DHT.h"
#define DHTPIN 22
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float hum = dht.readHumidity();
float temp = dht.readTemperature();

//Other things
float asoilmoist=analogRead(32);//global variable to store exponential smoothed soil moisture reading

// NTP (Network Time Protocol
#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate; // Variables to save date and time
String dayStamp; // Variables to save date and time
String timeStamp; // Variables to save date and time

void handleRoot() {
  digitalWrite(led, 0);
  String webtext ;
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  webtext="<html>\
  <head>\
    <meta http-equiv='refresh' content='10'/>\
    <title>WEMOS HIGROW ESP32 WIFI SOIL MOISTURE SENSOR</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>WEMOS HIGROW ESP32 WIFI SOIL MOISTURE SENSOR</h1>\   
    <br>\
    <p>Developed by JJ Slabbert. This code does not publish to any cloud service.</p>\
    <p>For soil moist, high values (range of +/-3344) means dry soil, lower values (+/- 1500) means wet soil. The Soil Moist Reading is influenced by the volumetric soil moisture content and electrical capacitive properties of the soil.</p>\
    <br>\
    <p>Date/Time: <span id='datetime'></span></p><script>var dt = new Date();document.getElementById('datetime').innerHTML = (('0'+dt.getDate()).slice(-2)) +'.'+ (('0'+(dt.getMonth()+1)).slice(-2)) +'.'+ (dt.getFullYear()) +' '+ (('0'+dt.getHours()).slice(-2)) +':'+ (('0'+dt.getMinutes()).slice(-2));</script>\
    <br>\
    <p>Soil Moisture: "+String(asoilmoist)+"</p>\
    <p>Temperature: "   +String(temp)+" &#176;C</p>\
    <p>Humidity: "   +String(hum)+" %</p>\
    <p><a href='datalog.txt'>DataLog</a> (This download is not working yet. Check the root folder of the Micro SD Card)</p>\
  </body>\
</html>";
  server.send(200, "text/html", webtext);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
  delay(1000);
  digitalWrite(led, 1);
}

void setup(void) {
  SD.begin(); //Start the SD card module
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  dht.begin();
  delay(2000);

// Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(7200); // Set offset time in seconds to adjust for your timezone, for example:
}

void loop(void) {
  for (int i=0;i<=5000000;i++){
    //The NTP requires WiFi WiFi use allot of current. Therefor datalogging is only done every 5:05 minutes
    if (i==5000000){
      asoilmoist=0.95*asoilmoist+0.05*analogRead(32);//exponential smoothing of soil moisture
      hum = dht.readHumidity();
      temp = dht.readTemperature();
      while(!timeClient.update()) {
      timeClient.forceUpdate();
      }
      // The formattedDate comes with the following format:
      // 2018-05-28T16:00:13Z
      formattedDate = timeClient.getFormattedDate();
        
      File dataFile = SD.open("/datalog.txt", FILE_APPEND);
      String dataString=String(asoilmoist)+", "+String(temp)+", "+String(hum)+", "+formattedDate;
      // if the file is available, write to it:
      if (dataFile) {
        dataFile.println(dataString);
        dataFile.close();
        // print to the serial port too:
        Serial.println(dataString);
      }
     
      // if the file isn't open, pop up an error:
      else {
        Serial.println("error opening datalog.txt");
        SD.begin();    
      }
      server.handleClient();
      
    }
    else{
      asoilmoist=0.99*asoilmoist+0.01*analogRead(32);//exponential smoothing of soil moisture
      hum = dht.readHumidity();
      temp = dht.readTemperature();
      server.handleClient();
    }  
  } 
} 
