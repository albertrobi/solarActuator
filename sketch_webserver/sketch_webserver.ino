#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <DS3232RTC.h> 
#include <Time.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <ArduinoJson.h>
#include "index.h" //Our HTML webpage contents with javascripts

#define LED 2  //On board LED
#define godeanu_latitude    46.770433              //Aleea Godeanu Cluj cordinates
#define godeanu_longtitude  23.614970 

//variables for time
int timezone = 2*3600;
int dst = 0; //day light saving
time_t ntp_time = 0;

MDNSResponder mdns;

// Replace with your network credentials
const char* ssid = "BalazsEsAlbert";
const char* password = "emeseesrobi87";

ESP8266WebServer server(80); //Server on port 80

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
   String s = MAIN_page; //Read HTML contents
   server.send(200, "text/html", s); //Send web page
}

// variables for wifi update
bool ota_flag = false;
 
// Motor variables
int turnRight = 0;
String motorTurningDirection = "Right"; //MAX 1149-1150 turns - error 14

volatile unsigned int feedBackCount = 0;
const int motorDirection = D1;
const int motor = D2;
const int keepOnHighD3 = D3;
const int keepOnHighD4 = D4;
const int readFeedBackD6 = D6;

// config static IP
IPAddress ip(192, 168, 0, 155); // where 155 is the desired IP Address
IPAddress gateway(192, 168, 0, 1); // set gateway
IPAddress subnet(255, 255, 255, 0); // set subnet mask


/*************************************************************************/
/*** Methods WEB AND MOTOR ***********************************************/
/*************************************************************************/
 void getFeedBack() {
     String feedBackValue = String(feedBackCount);
     Serial.println("FeedBack Response Count: " + feedBackValue);
     server.send(200, "text/plane", feedBackValue); //Send FeddABack value only to client ajax request
  }

  void resetFeedBackCounter() {
     Serial.println("FeedBack Reset Count to 0. ");
     feedBackCount = 0;
     server.send(200, "text/plane", "Success");
  }
  
  void handleLED() {
     String ledState = "OFF";
     String t_state = server.arg("LEDstate"); //Refer  xhttp.open("GET", "setLED?LEDstate="+led, true);
     Serial.println(t_state);
     if(t_state == "1")
     {
      digitalWrite(LED,LOW); //LED ON
      ledState = "ON"; //Feedback parameter
     }
     else
     {
      digitalWrite(LED,HIGH); //LED OFF
      ledState = "OFF"; //Feedback parameter  
     }
     
     server.send(200, "text/plane", ledState); //Send web page
  }
  
  void handleMotorStart() {
     digitalWrite ( motor, HIGH );
     Serial.print("Motor Start");
     server.send(200, "text/html", "Started");
  }
  
  void handleMotorStop() {
     digitalWrite ( motor, LOW );
      Serial.print("Motor Stop");
     server.send(200, "text/html", "Stoped");
  }
  
  void handleMotorTurnLeft() {
    turnRight = 0;
    Serial.print("Motor Left");
    digitalWrite ( motorDirection, LOW );
    server.send(200, "text/html", "LEFT");
  }
  
  void handleMotorTurnRight() {
    turnRight = 1;
    digitalWrite ( motorDirection, HIGH );
    Serial.print("Motor Right");
    server.send(200, "text/html", "Right");
  }

/*************************************************************************/
/*** Methods *************************************************************/
/*************************************************************************/
  //-----accumulate counts from flow sensor one, on pin2, interupt 0
  void ICACHE_RAM_ATTR flowOneInterupt()
    {
     if (turnRight == 1) {
      feedBackCount--;
     } else {
      feedBackCount++;
     }
     Serial.print("FeedBack counts:  "); Serial.println(feedBackCount); 
   }

 

   /* Get Date and Time*/
   void getDateAndTime()
  {
   time_t now = time(nullptr);
   struct tm* p_tm = localtime(&now);
   String currentTime = String(p_tm->tm_mday) + "/" + String(p_tm->tm_mon+1) + "/" + String(p_tm->tm_year+1900) + " ";
   currentTime = currentTime + String(p_tm->tm_hour) + ":" + String(p_tm->tm_min) + ":" + String(p_tm->tm_sec);
   
   Serial.println("Current Time:" + currentTime);
   server.send(200, "text/html", currentTime);
  }

  /** Sunrise and sunset **/
  void getSunriseAndSunset() {

     if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;  //Object of class HTTPClient
      
        http.begin("http://api.sunrise-sunset.org/json?lat="+String(godeanu_latitude)+"&lng="+String(godeanu_longtitude)+"&date=2018-10-22");
        // add curent date and UTC + 3
        
        int httpCode = http.GET();
        //Check the returning code  
         Serial.print("result: " + httpCode);                                                                
        if (httpCode > 0) {
          // TODO: Parsing
          const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(10) + 340;
          DynamicJsonBuffer jsonBuffer(bufferSize);
          
          JsonObject& root = jsonBuffer.parseObject( http.getString());
          
          JsonObject& results = root["results"];
          const char* results_sunrise = results["sunrise"]; // "6:31:05 AM"
          const char* results_sunset = results["sunset"]; // "5:33:22 PM"
          const char* results_solar_noon = results["solar_noon"]; // "12:02:14 PM"
          const char* results_day_length = results["day_length"]; // "11:02:17"
          const char* results_civil_twilight_begin = results["civil_twilight_begin"]; // "6:04:53 AM"
          const char* results_civil_twilight_end = results["civil_twilight_end"]; // "5:59:34 PM"
          const char* results_nautical_twilight_begin = results["nautical_twilight_begin"]; // "5:34:43 AM"
          const char* results_nautical_twilight_end = results["nautical_twilight_end"]; // "6:29:44 PM"
          const char* results_astronomical_twilight_begin = results["astronomical_twilight_begin"]; // "5:04:44 AM"
          const char* results_astronomical_twilight_end = results["astronomical_twilight_end"]; // "6:59:43 PM"
          
          const char* status = root["status"]; // "OK"

            // Output to serial monitor
            Serial.print("Sunrise:");
            Serial.println(results_sunrise);
            Serial.print("Sunset:");
            Serial.println(results_sunset);
            Serial.print("Day length:"); 
            Serial.println(results_day_length);

             server.send(200, "text/html", results_sunrise);
        }
        http.end();   //Close connection
      }
  }

/** Arduino OTA Update **/
  void startArduinoOta () {
    // start wifi update
  //  Serial.println("OTA START");
  //  ota_flag = true;
    server.send(200, "text/html", "ok");
  }

 void allowDeviceUpdate()
  {
    server.send(200,"text/plain", "Setting flag...");
    ota_flag = true;
  }

/** Sheduler methods **/
void Repeats(){
  Serial.println("15 second timer");
    // digital clock display of the time
   Serial.print(hour());
   printDigits(minute());
   printDigits(second());
   printDigits(day());
   printDigits(month());
   printDigits(year());
   Serial.println();          
}

/** Helper methods ****/
void printDigits(int digits)
{
 Serial.print(":");
 if(digits < 10)
   Serial.print('0');
 Serial.print(digits);
}

String webPage = "";

void setup(void){

  pinMode(readFeedBackD6, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(readFeedBackD6),flowOneInterupt,RISING);

  pinMode ( motorDirection, OUTPUT );
  pinMode ( motor, OUTPUT );
  pinMode ( keepOnHighD3, OUTPUT );
  pinMode ( keepOnHighD4, OUTPUT );
  pinMode ( keepOnHighD4, OUTPUT );

  digitalWrite ( motorDirection, LOW );
  digitalWrite ( motor, LOW );
  digitalWrite ( keepOnHighD3, HIGH );
  digitalWrite ( keepOnHighD4, HIGH );

  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    Alarm.delay(5000);
    ESP.restart();
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  WiFi.config(ip, gateway, subnet);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

/*************************************************************************/
/*** OTA Setup ***********************************************************/
/*************************************************************************/  

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  //ArduinoOTA.setPassword("robi");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

//  ArduinoOTA.onStart([]() {
//    String type;
//    if (ArduinoOTA.getCommand() == U_FLASH) {
//      type = "sketch";
//    } else { // U_SPIFFS
//      type = "filesystem";
//    }
//
//    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
//    Serial.println("Start updating " + type);
//  });
//  
//  ArduinoOTA.onEnd([]() {
//    Serial.println("\nEnd");
//  });
//  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
//    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
//  });
//  ArduinoOTA.onError([](ota_error_t error) {
//    Serial.printf("Error[%u]: ", error);
//    if (error == OTA_AUTH_ERROR) {
//      Serial.println("Auth Failed");
//    } else if (error == OTA_BEGIN_ERROR) {
//      Serial.println("Begin Failed");
//    } else if (error == OTA_CONNECT_ERROR) {
//      Serial.println("Connect Failed");
//    } else if (error == OTA_RECEIVE_ERROR) {
//      Serial.println("Receive Failed");
//    } else if (error == OTA_END_ERROR) {
//      Serial.println("End Failed");
//    }
//  });
//  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/setLED", handleLED);
  server.on("/readFeedBack", getFeedBack);
  server.on("/motorStart", handleMotorStart);
  server.on("/motorStop", handleMotorStop);
  server.on("/motorTurnRight", handleMotorTurnRight);
  server.on("/motorTurnLeft", handleMotorTurnLeft);
  server.on("/resetFeedBackCounter", resetFeedBackCounter);
  server.on("/startUpdate", allowDeviceUpdate);
  server.on("/getDateAndTime", getDateAndTime);
  server.on("/getSunriseAndSunset", getSunriseAndSunset);
  server.on("/startArduinoOta", startArduinoOta);
  

  server.begin();
  Serial.println("HTTP server started");

/*************************************************************************/
/*** Setup Time **********************************************************/
/*************************************************************************/ 
  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for internet time");
  while (!ntp_time){
     time(&ntp_time);
     Serial.println("*");
     Alarm.delay(1000); 
  }
  Serial.println("Time response....OK");
  time_t now = time(nullptr);
  setTime(now); 
  Alarm.timerRepeat(15, Repeats); // timer for every 15 seconds  
}
 
void loop(void){

/*************************************************************************/
/*** OTA Setup ***********************************************************/
/*************************************************************************/  
//  if(ota_flag)
//  {
//    Serial.println("IN OTA CYCLE");
//    //start OTA LED ON
//    digitalWrite(LED,LOW); //LED ON
//    
//    uint16_t time_elapsed = 0;
//    uint16_t time_start = millis();
//    while(time_elapsed < 25000)
//    {
//      ArduinoOTA.handle();
//      time_elapsed = millis()-time_start;
//      delay(10);
//    }
//    ota_flag = false;
//    //END OTA LED OFF 
//    digitalWrite(LED,HIGH); //LED OFF
//    
//     Serial.println("Exit OTA CYCLE");
//  }
  Alarm.delay(0);
  server.handleClient();
} 
