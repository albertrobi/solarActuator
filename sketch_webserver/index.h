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
  Time zone GMT+: <span id="timeZone">0</span><br>
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
<h1>Motor Turning Direction : <span id="motorTurningDirection">Left</span>
	<button type="button" onclick="motorTurnLeft()">Turn left</button>
	<button type="button" onclick="motorTurnRight()">Turn right</button><BR>
<div>
<div>
	Motor :  <span id="motorState">Stoped</span>
	<button type="button" onclick="sendMotorStart()">Motor Start</button>
	<button type="button" onclick="sendMotorStop()">Motor Stop</button><BR>
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

function getStatusData() {
  var password =  document.getElementById("password").value; 
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var statusData = JSON.parse(this.responseText);
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
      //timeZone
       document.getElementById("timeZone").innerHTML = ""+statusData.timeZone;
    } else if (this.status == 400) {
      document.getElementById("motorTurningDirection").style.color = "red";
      document.getElementById("motorState").style.color = "red";
    }
  };
  xhttp.open("GET", "getStatusData?TOTPKEY="+password, true);
  xhttp.send();

  // get some relevant data
  getCurrentDateAndTime(); 
  getSensorFeedBackData();
  getSunriseAndSunset();
}


setInterval(function() {
  // Call a function repetatively with 2 Second interval
  getSensorFeedBackData();
  getCurrentDateAndTime();
}, 20000); //60000mSeconds update rate

function getSensorFeedBackData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("feedBackValue").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "readFeedBack", true);
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
