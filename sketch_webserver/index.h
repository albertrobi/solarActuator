const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<div id="demo">
<h1>The ESP8266 NodeMCU Solar Motor controller</h1>
  <button type="button" onclick="sendData(1)">LED ON</button>
  <button type="button" onclick="sendData(0)">LED OFF</button><BR>
</div>

<div>
  Motor Feedback Value is : <span id="FeedBackValue">0</span><br>
    LED State is : <span id="LEDState">NA</span>
</div>

<div>
  Feedback : <span id="resetFeedBackCounter"></span>
  <button type="button" onclick="resetFeedBackCounter()">Reset</button>
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
function sendData(led) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("LEDState").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "setLED?LEDstate="+led, true);
  xhttp.send();
}

function sendMotorStart() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("motorState").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "motorStart", true);
  xhttp.send();
}

function sendMotorStop() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("motorState").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "motorStop", true);
  xhttp.send();
}

function motorTurnLeft() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("motorTurningDirection").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "motorTurnLeft", true);
  xhttp.send();
}

function motorTurnRight() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("motorTurningDirection").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "motorTurnRight", true);
  xhttp.send();
}

function resetFeedBackCounter () {
   var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("resetFeedBackCounter").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "resetFeedBackCounter", true);
  xhttp.send();
}


setInterval(function() {
  // Call a function repetatively with 2 Second interval
  getData();
}, 2000); //2000mSeconds update rate

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("FeedBackValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readFeedBack", true);
  xhttp.send();
}
</script>
</body>
</html>
)=====";
