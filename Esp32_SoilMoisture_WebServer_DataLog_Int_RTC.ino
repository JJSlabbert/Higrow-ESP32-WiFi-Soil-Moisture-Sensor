
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

const char* ssid = "KOOSSEHUIS";
const char* password = "PlaasHuis";
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
float gwc=exp(-0.0015*asoilmoist + 0.7072); //global variablr to store the gravimetric soil water content

// NTP (Network Time Protocol  : See The SimpleTime Example
#include "time.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600; //GMT=0, GMT+1=3600, GMT+2=7200.........
const int   daylightOffset_sec = 3600; //GMT=0, GMT+1=3600, GMT+2=7200.........

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
    <p>Soil Moisture, Sensor Value: "+String(asoilmoist)+"</p>\
    <p>Soil Moisture, gravimetric soil water content (WaterMass/DrySoilMass): "+String(gwc)+"</p>\
    <p>Temperature: "   +String(temp)+" &#176;C</p>\
    <p>Humidity: "   +String(hum)+" %</p>\
    <p><a href='datalog.txt'>DataLog</a> (This download is not working yet. Check the root folder of the Micro SD Card)</p>\
  </body>\
</html>";
  server.send(200, "text/html", webtext);
  digitalWrite(led, 1);
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
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  
}

void loop(void) 
{
  int n=1000000; //n is a timing parimeter. large n (n=1000000) means data will be logged +/- every 50 seconds
  for (int i=0; i<=n;i++)
  {
    if (i==n)
      {        
        asoilmoist=0.95*asoilmoist+0.05*analogRead(32);//exponential smoothing of soil moisture
        gwc=exp(-0.0015*asoilmoist + 0.7072);        
        hum = dht.readHumidity();
        temp = dht.readTemperature();

        struct tm timeinfo;
        getLocalTime(&timeinfo);
        char timeStringBuff[100]; //100 chars should be enough
        strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
          
        File dataFile = SD.open("/datalog.txt", FILE_APPEND);
        String dataString=String(asoilmoist)+", "+String(gwc)+", "+String(temp)+", "+String(hum)+", "+timeStringBuff;
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
      else
      {
        asoilmoist=0.95*asoilmoist+0.05*analogRead(32);//exponential smoothing of soil moisture
        gwc=exp(-0.0015*asoilmoist + 0.7072);
        hum = dht.readHumidity();
        temp = dht.readTemperature();
        server.handleClient();
      }
  }        
}
