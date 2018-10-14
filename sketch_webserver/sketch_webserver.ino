#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "index.h" //Our HTML webpage contents with javascripts

#define LED 2  //On board LED

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
bool ota_flag = true;
uint16_t time_elapsed = 0;
 
// Motor variables
int turnRight = 0;
String motorTurningDirection = "Right";

volatile unsigned int feedBackCount = 0;
const int motorDirection = D1;
const int motor = D2;
const int keepOnHighD3 = D3;
const int keepOnHighD4 = D4;
const int sendFeedBackD5 = D5;
const int readFeedBackD6 = D6;

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

  void allowDeviceUpdate()
  {
    server.send(200,"text/plain", "Setting flag...");
    ota_flag = true;
    time_elapsed = 0;
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
  pinMode (sendFeedBackD5, OUTPUT );

  digitalWrite ( motorDirection, LOW );
  digitalWrite ( motor, LOW );
  digitalWrite ( keepOnHighD3, HIGH );
  digitalWrite ( keepOnHighD4, HIGH );
  digitalWrite ( sendFeedBackD5, HIGH );

//  pinMode(2, OUTPUT);
  delay(1000);
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
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
  
  server.begin();
  Serial.println("HTTP server started");

 
}
 
void loop(void){
  if(ota_flag)
  {
    uint16_t time_start = millis();
    while(time_elapsed < 15000)
    {
      ArduinoOTA.handle();
      time_elapsed = millis()-time_start;
      delay(10);
    }
    ota_flag = false;
  }
  server.handleClient();
} 
