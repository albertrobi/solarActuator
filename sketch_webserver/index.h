const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body onload="getStatusData();">

<div id="demo">
<h1>The ESP8266 NodeMCU Solar Motor controller</h1>
</div>

<p>
<div> 
  <h4>
    Password: <input type="text" id="password">
  </h4>
<div> 
</p>

<div>
  Current time is : <span id="currentDateTime">0</span><br>
  Time zone GMT+: <span id="timeZone">0</span> 
  <button type="button" onclick="setCustomDateTimeAdd()">Increase Day Time +2 </button> 
  <button type="button" onclick="setCustomDateTimeSub()">Decrease Day Time -2 </button>
  <br> 
  Install new Software : 
  <button type="button" onclick="startArduinoOta()">Start OTA</button>
<div>
<div>
   Sunrize time is : <span id="sunrizeDateTime">0</span>
  <button type="button" onclick="getSunriseAndSunset()">Get sunrize</button>
<div>
<div>
  Sun Auto Track is : <span id="autoTrackState"> OFF </span>
  <button type="button" onclick="startAutoSunTrack()">Auto Track ON/OFF</button>
</div>
<div>
  Motor Feedback Value is : <span id="feedBackValue">0</span>
  <button type="button" onclick="resetFeedBackCounter()">Reset Feedback counter</button>
<div>
<div>
  Wind speed value is : <span id="windSpeedValue">0</span>  Wind guard : <span id="windGuardValue"></span>  <button type="button" onclick="handleWindGuard()">ON/OFF</button>
<div>
<div>
  Panel is at secure position : <span id="panelAtSecurePositionValue"></span>
<div>
<div>
  Photo sensor value UP: <span id="photoSensor1Value"></span> DOWN: <span id="photoSensor2Value"></span>
<div>
<div>
  Humidity: <span id="humidityValue"></span> Temperature: <span id="temperatureValue"></span>
<div>

<div>
<h1>Motor Turning Direction : <span id="motorTurningDirection">Left</span>
	<button type="button" onclick="motorTurnLeft()">Turn left</button>
	<button type="button" onclick="motorTurnRight()">Turn right</button><BR>
<div>
<div>
	Motor :  <span id="motorState">Stoped</span>
	<button type="button" onclick="sendMotorStart()">Motor Start</button>
	<button type="button" onclick="sendMotorStop()">Motor Stop</button><BR>
<div>
<div>
  Magnet State :  <span id="magnetState">Actived</span>
  <button type="button" onclick="sendMagnetActivate()">Magnet Activate</button>
  <button type="button" onclick="sendMagnetDeactivate()">Magnet Deactivate</button><BR>
<div>

<script>
function startAutoSunTrack() {
  var password =  document.getElementById("password").value; 
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("autoTrackState").innerHTML = this.responseText;
      document.getElementById("autoTrackState").style.color = "blue";
    } else if (this.status == 400) {
      document.getElementById("autoTrackState").style.color = "red";
    }
  };
  xhttp.open("GET", "autoSunTrack?TOTPKEY="+password, true);
  xhttp.send();
}

function sendMotorStart() {
  var password =  document.getElementById("password").value; 
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
     if (this.readyState == 4 && this.status == 200) {
      document.getElementById("motorState").innerHTML = this.responseText;
      document.getElementById("motorState").style.color = "blue";
    } else if (this.status == 400) {
      document.getElementById("motorState").style.color = "red";
    }
  };
  xhttp.open("GET", "motorStart?TOTPKEY="+password, true);
  xhttp.send();
}

function sendMotorStop() {
  var password =  document.getElementById("password").value; 
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
     if (this.readyState == 4 && this.status == 200) {
      document.getElementById("motorState").innerHTML = this.responseText;
      document.getElementById("motorState").style.color = "blue";
    } else if (this.status == 400) {
      document.getElementById("motorState").style.color = "red";
    }
  };
  xhttp.open("GET", "motorStop?TOTPKEY="+password, true);
  xhttp.send();
}

function motorTurnLeft() {
  var password =  document.getElementById("password").value; 
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("motorTurningDirection").innerHTML = this.responseText;
      document.getElementById("motorTurningDirection").style.color = "blue";
    } else if (this.status == 400) {
      document.getElementById("motorTurningDirection").style.color = "red";
    }
  };
  xhttp.open("GET", "motorTurnLeft?TOTPKEY="+password, true);
  xhttp.send();
}

function motorTurnRight() {
  var password =  document.getElementById("password").value; 
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("motorTurningDirection").innerHTML = this.responseText;
      document.getElementById("motorTurningDirection").style.color = "blue";
    } else if (this.status == 400) {
      document.getElementById("motorTurningDirection").style.color = "red";
    }
  };
  xhttp.open("GET", "motorTurnRight?TOTPKEY="+password, true);
  xhttp.send();
}

function sendMagnetActivate() {
  var password =  document.getElementById("password").value; 
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
     if (this.readyState == 4 && this.status == 200) {
      document.getElementById("magnetState").innerHTML = this.responseText;
      document.getElementById("magnetState").style.color = "blue";
    } else if (this.status == 400) {
      document.getElementById("magnetState").style.color = "red";
    }
  };
  xhttp.open("GET", "magnetActivate?TOTPKEY="+password, true);
  xhttp.send();
}

function sendMagnetDeactivate() {
  var password =  document.getElementById("password").value; 
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
     if (this.readyState == 4 && this.status == 200) {
      document.getElementById("magnetState").innerHTML = this.responseText;
      document.getElementById("magnetState").style.color = "blue";
    } else if (this.status == 400) {
      document.getElementById("magnetState").style.color = "red";
    }
  };
  xhttp.open("GET", "magnetDeactivate?TOTPKEY="+password, true);
  xhttp.send();
}

function resetFeedBackCounter () {
  var password =  document.getElementById("password").value;
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("feedBackValue").innerHTML = this.responseText;
      document.getElementById("feedBackValue").style.color = "blue";
    } else if (this.status == 400) {
      document.getElementById("feedBackValue").style.color = "red";
    }
  };
  xhttp.open("GET", "resetFeedBackCounter?TOTPKEY="+password, true);
  xhttp.send();
}

function handleWindGuard () {
  var password =  document.getElementById("password").value;
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("windGuardValue").innerHTML = this.responseText;
      document.getElementById("windGuardValue").style.color = "blue";
    } else if (this.status == 400) {
       document.getElementById("windGuardValue").style.color = "red";
    }
  };
  xhttp.open("GET", "handleWindGuard?TOTPKEY="+password, true);
  xhttp.send();
}

function getStatusData() {
  var password =  document.getElementById("password").value; 
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      showStatusData(JSON.parse(this.responseText));
    } else if (this.status == 400) {
      document.getElementById("motorTurningDirection").style.color = "red";
      document.getElementById("motorState").style.color = "red";
    }
  };
  xhttp.open("GET", "getStatusData?TOTPKEY="+password, true);
  xhttp.send();

  // get some relevant data
  getCurrentDateAndTime(); 
  getSensorData();
  getSunriseAndSunset();
}

function showStatusData(statusData) {
    if (statusData.motorDirection == 1) {
          document.getElementById("motorTurningDirection").innerHTML = "Right"
          document.getElementById("motorTurningDirection").style.color = "blue";
      } else {
         document.getElementById("motorTurningDirection").innerHTML = "Left"
         document.getElementById("motorTurningDirection").style.color = "blue";
      }
      if (statusData.motorStarted == 1) {
          document.getElementById("motorState").innerHTML = "Started"
          document.getElementById("motorState").style.color = "blue";
      } else {
         document.getElementById("motorState").innerHTML = "Stoped"
         document.getElementById("motorState").style.color = "blue";
      }
      if (statusData.magnetActive == 1) {
         document.getElementById("magnetState").innerHTML = "Actived";
         document.getElementById("magnetState").style.color = "blue";
      } else {
         document.getElementById("magnetState").innerHTML = "Deactivated";
         document.getElementById("magnetState").style.color = "blue";
      }
      // sunAutoTrack
      if (statusData.sunAutoTrack) {
         document.getElementById("autoTrackState").innerHTML = "ON";
         document.getElementById("autoTrackState").style.color = "blue";
      } else {
         document.getElementById("autoTrackState").innerHTML = "OFF";
         document.getElementById("autoTrackState").style.color = "blue";
      }
      //timeZone
       document.getElementById("timeZone").innerHTML = ""+statusData.timeZone; 
}


setInterval(function() {
  // Call a function repetatively with 2 Second interval
  getSensorData();
  getCurrentDateAndTime();
}, 10000); //10 seconds

function getSensorData() {
  var password =  document.getElementById("password").value; 
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var sensorData = JSON.parse(this.responseText);
      showStatusData(sensorData);
      if (sensorData.feedBackValue != null) {
         document.getElementById("feedBackValue").innerHTML = sensorData.feedBackValue;
          document.getElementById("feedBackValue").style.color = "blue";
      }
      if (sensorData.windSpeed != null) {
         document.getElementById("windSpeedValue").innerHTML = sensorData.windSpeed;
         document.getElementById("windSpeedValue").style.color = "blue";
      }
      if (sensorData.windGuardOn != null) {
         document.getElementById("windGuardValue").innerHTML = sensorData.windGuardOn;
         document.getElementById("windGuardValue").style.color = "blue";
      }
       if (sensorData.panelAtSecurePosition != null) {
         document.getElementById("panelAtSecurePositionValue").innerHTML = sensorData.panelAtSecurePosition;
         document.getElementById("panelAtSecurePositionValue").style.color = "blue";
      }
       if (sensorData.photoSensor1 != null) {
         document.getElementById("photoSensor1Value").innerHTML = sensorData.photoSensor1;
         document.getElementById("photoSensor1Value").style.color = "blue";
      }
      if (sensorData.photoSensor2 != null) {
         document.getElementById("photoSensor2Value").innerHTML = sensorData.photoSensor2;
         document.getElementById("photoSensor2Value").style.color = "blue";
      }
       if (sensorData.temperature != null) {
         document.getElementById("temperatureValue").innerHTML = sensorData.temperature;
         document.getElementById("temperatureValue").style.color = "blue";
      }
      if (sensorData.humidity != null) {
         document.getElementById("humidityValue").innerHTML = sensorData.humidity;
         document.getElementById("humidityValue").style.color = "blue";
      }
    } else {
      document.getElementById("windSpeedValue").style.color = "red";
      document.getElementById("feedBackValue").style.color = "red";
      document.getElementById("photoSensor1Value").style.color = "red";
      document.getElementById("photoSensor2Value").style.color = "red";
      document.getElementById("temperatureValue").style.color = "red";
      document.getElementById("humidityValue").style.color = "red";
      document.getElementById("panelAtSecurePositionValue").style.color = "red";
      document.getElementById("windGuardValue").style.color = "red";
    }
    };
  xhttp.open("GET", "getSensorData?TOTPKEY="+password, true);
  xhttp.send();
}

function getCurrentDateAndTime() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("currentDateTime").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "getDateAndTime", true);
  xhttp.send();
}

function setCustomDateTimeAdd() {
   var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("currentDateTime").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "setCustomDateTimeAdd", true);
  xhttp.send();
}

function setCustomDateTimeSub() {
   var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("currentDateTime").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "setCustomDateTimeSub", true);
  xhttp.send();
}

function getSunriseAndSunset() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("sunrizeDateTime").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "getSunriseAndSunset", true);
  xhttp.send();
}

function startArduinoOta() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
     //do nothing
    }
  };
  xhttp.open("GET", "startArduinoOta", true);
  xhttp.send();
}
</script>
</body>
</html>
)=====";
