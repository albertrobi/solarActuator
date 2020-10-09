#include <Boards.h>
#include <Firmata.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
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
#include <ESP8266TOTP.h>
#include <DHT.h>
#include "index.h" //Our HTML webpage contents with javascripts

#define LED 2  //On board LED
#define godeanu_latitude    46.770080              //Aleea Godeanu Cluj cordinates
#define godeanu_longtitude  23.615710
#define DHTPIN D5    // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);

//variables for time
int timezone = 0; // 2*3600;
int dst = 0; //day light saving
int romaniaTimeZone = 3; // UTC +2 romania timezont
time_t ntp_time = 0;

// sunrize/sunset/daylightSeconds
time_t sunrizeTime = 0;
time_t sunsetTime = 0 ;
double dayLightSec = 0;

//TOTP DATA
totpData data;

//ALARMS
AlarmID_t panelMovingAlarm = -1;
AlarmID_t sunTrackerAlarm = -1;
AlarmID_t everyDaySunrizeAlarm = -1;
AlarmID_t panelToInitialPosAlarm = -1;
AlarmID_t windSpeedTrackerAlarm = -1;

MDNSResponder mdns;

// Network credentials
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

void handleNotFound()
{
  server.sendHeader("Location", "/", true); //Redirect to our html web page
  server.send(302, "text/plane", "");
}

void navigateSolarControll() {
  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

// variables for wifi update
bool ota_flag = false;

// variables for sun auto tracking
bool sunAutoTrack = false;
bool initSunAutoTrack = false;
bool checkTotp = false;
bool isWindGuardOn = true;
bool isPanelAtSecurePostion = false;

// Motor variables
int turnRight = 0;
String motorTurningDirection = "Right"; //MAX 572 turns - error 14
volatile unsigned int maxRotation = 572;
const unsigned int max_wind_speed = 150; // max wind speed supported
int windSpeedHighCount = 0;

volatile unsigned int feedBackCount = 0;
volatile unsigned int desiredPosition = 0;
volatile unsigned int feedBackCountMovePosition = 0;
volatile unsigned int lastFeedBackCount = 0;
volatile unsigned int sameFeedBackNr = 0;
int windSpeed = 0;  // value read from the wind sensor
int photoSensor1 = 0;  // value read from the photo sensor
int photoSensor2 = 0;  // value read from the photo sensor
float temp = 0;
float humidity = 0; 

const int motorDirection = D1;
const int motor = D2;
const int keepOnHighD3 = D3;
const int keepOnHighD4 = D4;
const int readFeedBackD6 = D6;
const int magnet = D7;
const int analogInPin = A0;  // Analog Pin ADC0 = A0
const int analogSelD0 = D0; // selegt which analog source to listen S0
const int analogSelD8 = D8; // selegt which analog source to listen S1

// config static IP
IPAddress ip(192, 168, 50, 5); // where 155 is the desired IP Address
IPAddress gateway(192, 168, 0, 1); // set gateway
IPAddress subnet(255, 255, 255, 0); // set subnet mask


/*************************************************************************/
/*** Methods WEB AND MOTOR ***********************************************/
/*************************************************************************/
void getSensorData() {
  String totpKey = server.arg("TOTPKEY");
  if (isTokenValid(totpKey)) {
      // print the readings in the Serial Monitor
    StaticJsonBuffer<300> JSONbuffer;
    JsonObject& JSONencoder = JSONbuffer.createObject();
    JSONencoder["feedBackValue"] = feedBackCount;
    JSONencoder["windSpeed"] = windSpeed;
    JSONencoder["windGuardOn"] = isWindGuardOn;
    JSONencoder["panelAtSecurePosition"] = isPanelAtSecurePostion;
    JSONencoder["temperature"] = temp;
    JSONencoder["humidity"] = humidity;
    JSONencoder["photoSensor1"] = photoSensor1;
    JSONencoder["photoSensor2"] = photoSensor2;

    //add also status data
    JSONencoder["motorDirection"] = digitalRead(motorDirection);
    JSONencoder["motorStarted"] = digitalRead(motor);
    JSONencoder["timeZone"] = romaniaTimeZone;
    JSONencoder["magnetActive"] = digitalRead(magnet);
    JSONencoder["sunAutoTrack"] = sunAutoTrack;
    
    String json;
    JSONencoder.prettyPrintTo(json);
    server.send(200, "text/json", json);
    
  }
}

void resetFeedBackCounter() {
  String totpKey = server.arg("TOTPKEY");
  if (isTokenValid(totpKey)) {
    Serial.println("FeedBack Reset Count to 0. ");
    feedBackCount = 0;
    server.send(200, "text/plane", "0");
  } else {
    Serial.println("Invalid TOTP key provided");
    server.send(400, "text/html", "BAD Request");
  }
}

void startAutoSunTrack() {
  String totpKey = server.arg("TOTPKEY");
  if (isTokenValid(totpKey)) {
    if (!sunAutoTrack) {
      sunAutoTrack = true;
      Serial.println("Sun Auto Track ON");
      server.send(200, "text/html", "ON");
      initSolarTracking();
    } else {
      stopSunAutoTrack();
      Serial.println("Sun Auto Track OFF");
      server.send(200, "text/html", "OFF");
    }
  } else {
    Serial.println("Invalid TOTP key provided");
    server.send(400, "text/html", "BAD Request");
  }
}

void handleMotorStart() {
  String totpKey = server.arg("TOTPKEY");
  if (isTokenValid(totpKey)) {
    magnetDeactivate();
    digitalWrite ( motor, HIGH );
    Serial.println("Motor Start");
    server.send(200, "text/html", "Started");
  } else {
    Serial.println("Invalid TOTP key provided");
    server.send(400, "text/html", "BAD Request");
  }
}

void handleMotorStop() {
  String totpKey = server.arg("TOTPKEY");
  // if (isTokenValid(totpKey)) {
  digitalWrite ( motor, LOW );
  Serial.println("Motor Stop");
  server.send(200, "text/html", "Stoped");
  //     } else {
  //       Serial.println("Invalid TOTP key provided");
  //       server.send(400, "text/html", "BAD Request");
  //     }
}

void handleMotorTurnLeft() {
  String totpKey = server.arg("TOTPKEY");
  if (isTokenValid(totpKey)) {
    turnRight = 0;
    Serial.println("Motor Left");
    digitalWrite ( motorDirection, LOW );
    server.send(200, "text/html", "LEFT");
  } else {
    Serial.println("Invalid TOTP key provided");
    server.send(400, "text/html", "BAD Request");
  }
}

void handleMotorTurnRight() {
  String totpKey = server.arg("TOTPKEY");
  if (isTokenValid(totpKey)) {
    turnRight = 1;
    digitalWrite ( motorDirection, HIGH );
    Serial.println("Motor Right");
    server.send(200, "text/html", "Right");
  } else {
    Serial.println("Invalid TOTP key provided");
    server.send(400, "text/html", "BAD Request");
  }
}

void handleWindGuard() {
  isWindGuardOn = !isWindGuardOn;
  server.send(200, "text/html", String(isWindGuardOn));
}

void handleMagnetActivate() {
   magnetActivate();
   server.send(200, "text/html", "Actived");
}

void handleMagnetDeactivate() {
    magnetDeactivate();
    server.send(200, "text/html", "Deactivate");
}

void magnetActivate() {
    digitalWrite ( magnet, HIGH );
    Serial.println("Magnet Activated");
}

void magnetDeactivate() {
    digitalWrite ( magnet, LOW );
    Serial.println("Magnet Deactivated");
}

void handleGetStatusData() {
  String totpKey = server.arg("TOTPKEY");
  if (isTokenValid(totpKey)) {
    StaticJsonBuffer<300> JSONbuffer;
    JsonObject& JSONencoder = JSONbuffer.createObject();
    JSONencoder["motorDirection"] = digitalRead(motorDirection);
    JSONencoder["motorStarted"] = digitalRead(motor);
    JSONencoder["timeZone"] = romaniaTimeZone;
    JSONencoder["magnetActive"] = digitalRead(magnet);
    JSONencoder["sunAutoTrack"] = sunAutoTrack;

    String json;
    JSONencoder.prettyPrintTo(json);
  
    server.send(200, "text/json", json);
    
  }
}

/*************************************************************************/
/*** Methods *************************************************************/
/*************************************************************************/
//-----accumulate counts from flow sensor one, on pin2, interupt 0
void ICACHE_RAM_ATTR flowOneInterupt()
{
  if (digitalRead(motor) == HIGH) {
    if (turnRight == 1 ) {
      feedBackCount++;
    } else {
      feedBackCount--;
    }
     Serial.print("FeedBack counts:  "); Serial.println(feedBackCount);
  }
}



/***************************************************************************************/
/*** Date and Time Methods *************************************************************/

/* Get Date and Time*/
void getDateAndTime()
{
  // to get current time
  time_t now = time(nullptr);
  struct tm curentDayTime = *localtime(&now);
  curentDayTime.tm_hour = curentDayTime.tm_hour + romaniaTimeZone;

  String currentTime = String(curentDayTime.tm_mday) + "/" + String(curentDayTime.tm_mon + 1) + "/" + String(curentDayTime.tm_year + 1900) + " ";
  currentTime = currentTime + String(curentDayTime.tm_hour) + ":" + String(curentDayTime.tm_min) + ":" + String(curentDayTime.tm_sec);
  
  // Serial.println("Current Time:" + currentTime);
  server.send(200, "text/html", currentTime);
}

void setCustomDateTimeAdd()
{
  romaniaTimeZone = romaniaTimeZone + 2;
  getDateAndTime();
}
void setCustomDateTimeSub()
{
  romaniaTimeZone = romaniaTimeZone - 2;
  getDateAndTime();
}

/** Sunrise and sunset **/
void getSunriseAndSunset() {

  Serial.println("Start and get sunrize!" + String(WiFi.status()) + " -- " + String(WiFi.status() == WL_CONNECTED));
  time_t now = time(nullptr);
  struct tm* currentTime = localtime(&now);
  String currentTimeString = String(currentTime->tm_year + 1900) + "-" + String(currentTime->tm_mon + 1) + "-" + String(currentTime->tm_mday);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;  //Object of class HTTPClient

    http.begin("http://api.sunrise-sunset.org/json?lat=" + String(godeanu_latitude) + "&lng=" + String(godeanu_longtitude) + "&date=" + currentTimeString);
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
      const char* results_day_length = results["day_length"]; // "11:02:17"

      const char* status = root["status"]; // "OK"

      // Output to serial monitor
      Serial.println();
      Serial.println("Sunrise:");
      Serial.println(results_sunrise);
      Serial.print("Sunset:");
      Serial.println(results_sunset);
      Serial.print("Day length:");
      Serial.println(results_day_length);

      char* copy = strdup(results_sunrise);
      char* sunrizeHours = strtok(copy, " :");
      sunrizeTime = convertToTimeT(sunrizeHours, "Sunrize hour:");
      free(copy);
      copy = strdup(results_sunset);
      char* sunsetHours = strtok(copy, " :");
      sunsetTime = convertToTimeT(sunrizeHours, "Sunset hour:");
      free(copy);
      copy = strdup(results_day_length);
      char* dayLength = strtok(copy, " :");
      dayLightSec = convertDayLightToSeconds(sunrizeHours, "Day light seconds:");
      free(copy);

      struct tm sunrizeTimeStruct = *localtime(&sunrizeTime); //convert the time to struct tm*
      // show on serial the calculate time
      String sunrizeCalcTime = String(sunrizeTimeStruct.tm_mday) + "/" + String(sunrizeTimeStruct.tm_mon + 1) + "/" + String(sunrizeTimeStruct.tm_year + 1900) + " ";
      sunrizeCalcTime = sunrizeCalcTime + String(sunrizeTimeStruct.tm_hour) + ":" + String(sunrizeTimeStruct.tm_min) + ":" + String(sunrizeTimeStruct.tm_sec);
      server.send(200, "text/html", sunrizeCalcTime);
    }
    http.end();   //Close connection
  }
  Serial.println("New Sunrize data!");
}

/**
    convert the given token char to time_t
    get current time and update the hour, minutes and seconds
*/
time_t convertToTimeT(char* token, char* type) {

  // to get current time
  time_t now = time(nullptr);
  struct tm token_time = *localtime(&now);
  int i = 0;

  Serial.println();
  while ( token != NULL ) {
    if (i == 0) {
      token_time.tm_hour = atoi(token) + romaniaTimeZone;
      i++;
    } else if (i == 1) {
      token_time.tm_min = atoi(token);
      i++;
    } else if (i == 2) {
      token_time.tm_sec = atoi(token);
      i++;
    } else if (i == 3) {
      if (strcmp(token, "PM") == 0) {
        token_time.tm_hour = token_time.tm_hour + 12;
      }
      i++;
    }
    Serial.print(token );
    token = strtok(NULL, " :");
  }
  Serial.println();
  time_t calc_Time = mktime(&token_time); // create the new time
  token_time = *localtime(&calc_Time); //convert the new time to struct tm*

  // show on serial the calculate time
  String currentTime = String(token_time.tm_mday) + "/" + String(token_time.tm_mon + 1) + "/" + String(token_time.tm_year + 1900) + " ";
  currentTime = currentTime + String(token_time.tm_hour) + ":" + String(token_time.tm_min) + ":" + String(token_time.tm_sec);
  Serial.println(type + currentTime);

  return calc_Time;
}

/**
    convert the given token char to nr of seconds of daylight
    get current daylight in nr seconds
*/
double convertDayLightToSeconds(char* token, char* type) {

  // to get current time
  double dayLightSec = 0;
  int i = 0;
  Serial.println();
  while ( token != NULL ) {
    if (i == 0) {
      dayLightSec = 3600 * atoi(token); // days
      i++;
    } else if (i == 1) {
      dayLightSec = dayLightSec + (60 * atoi(token)); // minutes
      i++;
    } else if (i == 2) {
      dayLightSec = dayLightSec + atoi(token); // seconds
      i++;
    }
    Serial.print(token );
    token = strtok(NULL, " :");
  }
  Serial.println();
  Serial.println(type + String(dayLightSec));

  return dayLightSec;
}

/**********************************************************************************************/
/*** Wind Speed Mesuring Methods  *************************************************************/
/** Wind speed Module **/ 
void measureWindSpeed() {
  digitalWrite ( analogSelD0, HIGH ); //S0
  digitalWrite ( analogSelD8, HIGH );  //S1
  windSpeed = analogRead(analogInPin);
  Serial.println("* Wind Speed = " + String(windSpeed));
  measurePhotoSensor1();
  measurePhotoSensor2();
  measureTemperature();
  if (windSpeed > max_wind_speed) {
    windSpeedHighCount++;
    // move panel to initial position only if wind continues to blow high at least 10 seconds each count is 2 seconds
    if (windSpeedHighCount == 5 && !Alarm.isAllocated(panelToInitialPosAlarm) && isWindGuardOn && !isPanelAtSecurePostion) {
       Serial.println("******* Wind Speed too high moving panel to secure position " + String(windSpeed));
      stopSunAutoTrack();
      movePanelToSecurePosition();
    }
  } else {
    windSpeedHighCount = 0;
  }
}

void measurePhotoSensor1() {
  digitalWrite ( analogSelD0, LOW ); //S0
  digitalWrite ( analogSelD8, HIGH );  //S1
  photoSensor1 = analogRead(analogInPin);
  Serial.println("* Photo sensor1 = " + String(photoSensor1));
}

void measurePhotoSensor2() {
  digitalWrite ( analogSelD0, HIGH ); //S0
  digitalWrite ( analogSelD8, LOW );  //S1
  photoSensor2 = analogRead(analogInPin);
  Serial.println("* Photo sensor2 = " + String(photoSensor2));
}
void measureTemperature() {
// Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  humidity = dht.readHumidity();
  // Read temperature as Celsius
  temp = dht.readTemperature();
   Serial.println("* Temperature = " + String(temp));
   Serial.println("* Humidity = " + String(humidity));
}

/********************************************************************************************/
/*** Arduino OTA Update Methods *************************************************************/

/** Arduino OTA Update **/
void startArduinoOta () {
  // start wifi update
  //  Serial.println("OTA START");
  //  ota_flag = true;
  server.send(200, "text/html", "ok");
}

void allowDeviceUpdate()
{
  server.send(200, "text/plain", "Setting flag...");
  ota_flag = true;
}

/** Sheduler methods **/
void Repeats() {
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
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*
   returns an indicator whether the totp value in the local suppliedTotp string variable
   matches the current otp calculated from the current epoch offset and secret key bytes
*/
bool isTokenValid( String suppliedTotp) {
  if (checkTotp) {
    char *endptr;
    int suppliedOtp = strtoul(suppliedTotp.c_str(), &endptr, 10);
    time_t now = time(nullptr);
    //check the supplied OTP against the current secret key
    return ((*endptr == '\0') &&
            (ESP8266TOTP::IsTokenValid(now, data.keyBytes, suppliedOtp)));
  } else return true;
}

/*************************************************************************/
/*** Methods Handling Auto Solar Tracking ********************************/
/*************************************************************************/
void initSolarTracking () {
  if (sunAutoTrack) { //if sun tracking enabled
    Serial.println("---- Sun auto track started ---- ");
    sameFeedBackNr = 0;
    lastFeedBackCount = feedBackCount;
    //move panel to starting position, rotate max left (city)
    turnRight = 0;
    Serial.println("Motor Left");
    digitalWrite ( motorDirection, LOW );
    digitalWrite ( motor, HIGH );
    Serial.println("Motor Start");
    time_t now = time(nullptr);
    setTime(now);
    Serial.println("--- Disable Sun Tracker Alarm");
    Alarm.free (sunTrackerAlarm);
    Serial.println("--- Disable Panel Moving Alarm");
    Alarm.free (panelMovingAlarm);
    Serial.println("--- Disable Panel Init Alarm");
    Alarm.free (panelToInitialPosAlarm);
    if (!Alarm.isAllocated(panelToInitialPosAlarm)) {
        Serial.println("--- Start Panel Init Alarm (1)");
        panelToInitialPosAlarm = Alarm.timerRepeat(2, setPanelToInitialPosition); // timer for every 2 seconds
        startSunTracking (); // here starts the real sun pos calculation and tracking
    }
  }
}

// Wait till panel moves to initial position
void setPanelToInitialPosition () {
    if (feedBackCount != desiredPosition) { // move panel till desired postion does not change means that panel is max to left (city)
      Serial.println("--- Panel is moving ... ---");
      desiredPosition = feedBackCount;
      isPanelAtSecurePostion = false;
    } else {
      Serial.println("-----> Panel is at initial position");
      isPanelAtSecurePostion = true;
      Serial.println("--- Disable Panel Moving Alarm");
      Alarm.free (panelMovingAlarm);
      Serial.println("--- Disable Panel Init Alarm");
      Alarm.free (panelToInitialPosAlarm);
      // stop motor reset counters
      Serial.println("Motor Stop");
      digitalWrite ( motor, LOW );
      feedBackCount = 0;
      desiredPosition = 0;
     } 
  }

// calculate sun position and init the sun tracker
  void startSunTracking () {
    if (sunAutoTrack) { //if sun tracking enabled
      Serial.println("--- Disable Sun Tracker Alarm (1)");
      Alarm.free (sunTrackerAlarm);
      if (!Alarm.isAllocated(sunTrackerAlarm)) {
        Serial.println("--- Start Sun Tracker Alarm");
        sunTrackerAlarm = Alarm.timerRepeat(480, calcSunPositionAndRotate); // timer for every 8 minutes
      }
    } else {
      stopSunAutoTrack();
    }
  }

/*
   calculate sun position and rotate panel to the desired position
*/
void calcSunPositionAndRotate() {
  if (!Alarm.isAllocated(panelToInitialPosAlarm) && !Alarm.isAllocated(panelMovingAlarm)) { // check that panel is not moving
    if (sunAutoTrack && (digitalRead(motor) != HIGH)) { //if sun tracking enabled
      //caluclate position based on sunrize and sunset
      double currentDaySec = getSecondsOfDayToRefTime(sunrizeTime);
      Serial.println("Current day seconds since sunrize time: " + String(currentDaySec));
      Serial.println("Max daylight sec: " + String(dayLightSec));
      Serial.println("Current Day Sec: " + String(currentDaySec));
      if (currentDaySec > 0 && dayLightSec > 0 && currentDaySec < dayLightSec) {
        desiredPosition = maxRotation - ((maxRotation * currentDaySec) / dayLightSec);
        Serial.println("Panel desired position: " + String(desiredPosition));
        if (desiredPosition > 0 && desiredPosition < maxRotation) {
          if (desiredPosition > feedBackCount) {
             Serial.println("--- Disable Panel Moving Alarm");
             Alarm.free (panelMovingAlarm);
            // start rotation right
            magnetDeactivate();
            turnRight = 1;
            digitalWrite ( motorDirection, HIGH );
            Serial.println("Motor Right");
            digitalWrite ( motor, HIGH );
            Serial.println("Motor Start");
            isPanelAtSecurePostion = false; 
            Serial.println("--- Start Panel Moving Alarm - rotate to position (1)");
            panelMovingAlarm = Alarm.timerRepeat(1, roateToPosition);
          } else if (desiredPosition < feedBackCount && feedBackCount < maxRotation) {
            Serial.println("--- Disable Panel Moving Alarm");
            Alarm.free (panelMovingAlarm);
            // start rotation left (city)
            magnetDeactivate();
            turnRight = 0;
            Serial.println("Motor Left");
            digitalWrite ( motorDirection, LOW );
            digitalWrite ( motor, HIGH );
            Serial.println("Motor Start");
            isPanelAtSecurePostion = false;
            Serial.println("--- Start Panel Moving Alarm - rotate to position (2)");
            panelMovingAlarm = Alarm.timerRepeat(1, roateToPosition);
          } else if (feedBackCount > maxRotation) {
            Serial.println("Reset feedback count to 0 from : " + String(feedBackCount));
            feedBackCount = 0;
          }
        }
      } else if (currentDaySec > (dayLightSec + 500) && !isPanelAtSecurePostion) { // if sunny day is over move panel back to main position / city position
        Serial.println("Day end move panel to starting position, feedBackCount: " + String(feedBackCount));
        movePanelToSecurePosition();
      } // it's here
    } else if (digitalRead(motor) != HIGH) {
      stopSunAutoTrack();
    } else {
      magnetDeactivate();
      Serial.println("Motor HIGH in calcSunPositionAndRotate ");
    }
  } else {
    Serial.println("Panel is moving right now, -> Sun Auto Track delayed ");
  }
}
/**
 * movePanelToSecurePosition - move panel to initial position activate magnets
 */
void movePanelToSecurePosition() {
  if (!Alarm.isAllocated(panelToInitialPosAlarm)) {
    // start rotation left
    //move panel to starting position, rotate max left (city)
    Serial.println("--- Disable Panel Moving Alarm");
    Alarm.free (panelMovingAlarm);
    magnetActivate();
    turnRight = 0;
    Serial.println("Motor Left");
    digitalWrite ( motorDirection, LOW );
    digitalWrite ( motor, HIGH );
    Serial.println("Motor Start");
    Serial.println("--- Start Panel Init Alarm (2)");
    panelToInitialPosAlarm = Alarm.timerRepeat(2, setPanelToInitialPosition); // timer for every 2 seconds
  } else {
    Serial.println("---movePanelToSecurePosition: panelToInitialPosAlarm is allocated");   
  }
}

/**
   rotate panel to given position
*/
void roateToPosition () {
  if (sunAutoTrack) { //if sun tracking enabled
    if ( (((feedBackCount <= desiredPosition) && turnRight == 1) || ((feedBackCount >= desiredPosition) && turnRight == 0)) && sameFeedBackNr < 11) {
      // we check that the panel is moving otherwise after 11 trials stop the auto tracking
      if (feedBackCount == lastFeedBackCount) { // if last known position is the same
        sameFeedBackNr++;
      } else {
        sameFeedBackNr = 0;
      }
      lastFeedBackCount = feedBackCount;  // last known position
      Serial.println("--- Panel is moving ... Position: " + String(feedBackCount) + " --- Desired position: "+String(desiredPosition));
    } else if (sameFeedBackNr >= 11) {
//      if (turnRight == 1) {
//        Serial.println("--- Panel at morning position: "+ String(feedBackCount) + " --- Desired position: "+String(desiredPosition));
//        Serial.println("--- Disable Panel Moving Alarm");
//        Alarm.free (panelMovingAlarm);
//        Serial.println("Motor Stop");
//        digitalWrite ( motor, LOW );
//        sameFeedBackNr = 0;
//      } else {
        Serial.println("--- Panel Error not moving!!! Position: " + String(feedBackCount) + " --- Desired position: "+String(desiredPosition) + " sameFeedBackNr:"+String(sameFeedBackNr));
        stopSunAutoTrack();
      //}
    } else {
      Serial.println("--- Panel in sun position ... Position: " + String(feedBackCount) + " --- Desired position: "+String(desiredPosition));
      Serial.println("--- Disable Panel Moving Alarm");
      Alarm.free (panelMovingAlarm);
      Serial.println("Motor Stop");
      digitalWrite ( motor, LOW );
    }
  } else {
    stopSunAutoTrack();
  }
}

/**
    stop sun auto tracking in case of an error
*/
void stopSunAutoTrack() {
  sunAutoTrack = false;
  Serial.println("--- Stop sun auto track ... Position: " + String(feedBackCount) + " ---");
  Serial.println("--- Disable Panel Moving Alarm");
  Alarm.free (panelMovingAlarm);
  Serial.println("--- Disable Sun Tracker Alarm");
  Alarm.free (sunTrackerAlarm);
  Serial.println("--- Disable Panel Init Alarm");
  Alarm.free (panelToInitialPosAlarm);
  Serial.println("Motor Stop");
  digitalWrite ( motor, LOW );
}

double getSecondsOfDayToRefTime(time_t ref_time) {
  // to get current time
  time_t now = time(nullptr);
  struct tm curentDayTime = *localtime(&now);
  curentDayTime.tm_hour = curentDayTime.tm_hour + romaniaTimeZone;

  struct tm refTime = *localtime(&ref_time); //convert the time to struct tm*

  String currentTime = String(curentDayTime.tm_mday) + "/" + String(curentDayTime.tm_mon + 1) + "/" + String(curentDayTime.tm_year + 1900) + " ";
  currentTime = currentTime + String(curentDayTime.tm_hour) + ":" + String(curentDayTime.tm_min) + ":" + String(curentDayTime.tm_sec);
  Serial.println("Current Day Time: " + currentTime);

  // show on serial the calculate time
  String currentTime2 = String(refTime.tm_mday) + "/" + String(refTime.tm_mon + 1) + "/" + String(refTime.tm_year + 1900) + " ";
  currentTime2 = currentTime2 + String(refTime.tm_hour) + ":" + String(refTime.tm_min) + ":" + String(refTime.tm_sec);
  Serial.println("Sunrize Time: " + currentTime2);

  double daySec = 0;
  int minusMin = 0;
  int minusHour = 0;

  if (curentDayTime.tm_sec < refTime.tm_sec) {
    daySec = curentDayTime.tm_sec - refTime.tm_sec + 60;
    minusMin = -1;
  } else {
    daySec = curentDayTime.tm_sec - refTime.tm_sec;
  }

  if ((curentDayTime.tm_min - minusMin ) < refTime.tm_min) {
    daySec = daySec + ( 60 * (curentDayTime.tm_min - minusMin - refTime.tm_min + 60));
    minusHour = -1;
  } else {
    daySec = daySec + ( 60 * (curentDayTime.tm_min + minusMin - refTime.tm_min));
  }

  daySec = daySec + ( 3600 * (curentDayTime.tm_hour + minusHour - refTime.tm_hour));

  return daySec;

}

String webPage = "";

void setup(void) {

  pinMode(readFeedBackD6, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(readFeedBackD6), flowOneInterupt, RISING);

  pinMode ( motorDirection, OUTPUT );
  pinMode ( motor, OUTPUT );
  pinMode ( keepOnHighD3, OUTPUT );
  pinMode ( keepOnHighD4, OUTPUT );
  pinMode ( magnet, OUTPUT );
  pinMode ( analogSelD0, OUTPUT );
  pinMode ( analogSelD8, OUTPUT );
  pinMode(DHTPIN, INPUT);

  digitalWrite ( motorDirection, LOW );
  digitalWrite ( motor, LOW );
  digitalWrite ( keepOnHighD3, HIGH );
  digitalWrite ( keepOnHighD4, HIGH );
  digitalWrite ( magnet, HIGH );
  digitalWrite ( analogSelD0, LOW );
  digitalWrite ( analogSelD8, LOW );

  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  dht.begin();

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

  /*************************************************************************/
  /*** TOTP Example ********************************************************/
  /*************************************************************************/
  // if (ESP8266TOTP::GetNewKey(data.keyBytes)) {
  //    Serial.println((char*)data.keyBytes);
  //
  //    unsigned char data32[BASE_32_ENCODE_LENGTH];
  //    if (ESP8266TOTP::GetBase32Key(data.keyBytes, data32)) {
  //      Serial.println("Base 32:");
  //      Serial.println(reinterpret_cast<char*>(&data32));
  //      Serial.println(now);
  //      int otp = ESP8266TOTP::GetTOTPToken(now, data.keyBytes);
  //      Serial.println("OTP Token: ");
  //      Serial.println(otp);
  //
  //      Serial.println(reinterpret_cast<char*>(&data32));
  //      Serial.println(ESP8266TOTP::GetQrCodeImageUri(data.keyBytes, "albertrobi", "albertrobi"));
  //
  //      if (ESP8266TOTP::IsTokenValid(now, data.keyBytes, otp)) {
  //
  //        //this code path will always be taken in this test application
  //        //since we're basically comparing a firmware calculated OTP with the same
  //        //firmware calculated OTP passed as the ESP8266TOTP::IsTokenValid candidateOtp parameter.
  //        //In a real application, STATIC_EPOCH would be an NTP client determined epoch time value
  //        //and the ESP8266TOTP::IsTokenValid candidateOtp parameter would be supplied to the
  //        //firmware in some way i.e. as a HTTP form post value
  //
  //        Serial.println("ESP8266TOTP::IsTokenValid..Yes it is!!");
  //      }
  //
  //    } else {
  //      Serial.println("ESP8266TOTP::GetBase32Key failed");
  //    }
  //
  //  } else {
  //    Serial.println("ESP8266TOTP::GetNewKey failed");
  //  }


  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.onNotFound(handleNotFound);
  server.on("/autoSunTrack", startAutoSunTrack);
  server.on("/getSensorData", getSensorData);
  server.on("/motorStart", handleMotorStart);
  server.on("/motorStop", handleMotorStop);
  server.on("/motorTurnRight", handleMotorTurnRight);
  server.on("/motorTurnLeft", handleMotorTurnLeft);
  server.on("/resetFeedBackCounter", resetFeedBackCounter);
  server.on("/startUpdate", allowDeviceUpdate);
  server.on("/getDateAndTime", getDateAndTime);
  server.on("/getSunriseAndSunset", getSunriseAndSunset);
  server.on("/startArduinoOta", startArduinoOta);
  server.on("/getStatusData", handleGetStatusData);
  server.on("/magnetActivate", handleMagnetActivate);
  server.on("/handleWindGuard", handleWindGuard);
  server.on("/magnetDeactivate", handleMagnetDeactivate);
  server.on("/setCustomDateTimeAdd", setCustomDateTimeAdd);
  server.on("/setCustomDateTimeSub", setCustomDateTimeSub);

  server.begin();
  Serial.println("HTTP server started");

  /*************************************************************************/
  /*** Setup Time **********************************************************/
  /*************************************************************************/
  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for internet time");
  while (!ntp_time) {
    time(&ntp_time);
    Serial.println("*");
    Alarm.delay(1000);
  }
  Serial.println("Time response....OK");


  /*************************************************************************/
  /*** Setup Scheduler ALARMS **********************************************************/
  /*************************************************************************/
  time_t now = time(nullptr);
  setTime(now);
  getSunriseAndSunset();

  Serial.println("--- Init Every Day Sunrize Alarm");
  everyDaySunrizeAlarm = Alarm.alarmRepeat(7 - romaniaTimeZone, 45, 0, getSunriseAndSunset); // every morning get sunrize and sunset, hour -3 since we are UTC
  Serial.println("--- Init Wind Speed Tracker ");
  windSpeedTrackerAlarm = Alarm.timerRepeat(2, measureWindSpeed); // timer for every 2 seconds; 

  /*************************************************************************/
  /*** Setup TOTP **********************************************************/
  /*************************************************************************/
  ESP8266TOTP::GetNewKey(data.keyBytes);
}

void loop(void) {
  
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
