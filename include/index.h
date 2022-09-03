#include "Arduino.h"

const char HTTP_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, user-scalable=no, minimum-scale=1.0, maximum-scale=1.0">
    
    <style>
    body {
      overflow	: hidden;
      padding		: 0;
      margin		: 0;
      background-color: rgb(56, 44, 44);
    }
    .btn {
      box-shadow:inset 0px 2px 0px 0px #cf866c;
      background-color:#6e8cb3;
      border-radius:10px;
      border:1px solid #942911;
      display:inline-block;
      cursor:pointer;
      color:#e3e3e3;
      font-size:13px;
      padding:6px 24px;
      text-decoration:none;
      text-shadow:0px 1px 0px #854629;
      font-family: "Arial Black", Gadget, sans-serif
    }
    .btn:hover {
      background-color:#a65849;
    }
    .btn:active {
      position:relative;
      top:1px;
    }
    #resultLeft, #resultRight {
      font-family: "Arial Black", Gadget, sans-serif
    }
    #info {
      position	: absolute;
      top		: 0px;
      width		: 100%%;
      padding		: 5px;
      text-align	: center;
      color: rgb(214, 185, 185);;
    }
    #container {
      width		: 100%%;
      height		: 100%%;
      overflow	: hidden;
      padding		: 0;
      margin		: 0;
      -webkit-user-select	: none;
      -moz-user-select	: none;
    }
    #stream {
      display: block;
      margin-left: auto;
      margin-right: auto;
      width: 100%%;
      height: 100%%;
    }
    </style>
  </head>
  <body>
    <div id="container">
      <div>
        <img id="stream">
      </div>
    </div>
    <div id="info">
      <br/>
      <button class="btn" value="Normal" onclick="onModeChange(this.value)">Normal</button>
      <button class="btn" value="Spin" onclick="onModeChange(this.value)">Spin</button>
      <button class="btn" value="Arm" onclick="onModeChange(this.value)">Arm</button>
      <button class="btn" value="Gripper" onclick="onModeChange(this.value)">Gripper</button>
      <button class="btn" value="Head" onclick="onModeChange(this.value)">Head</button>
      <button class="btn" value="Camera" onclick="onModeChange(this.value)">Toggle Camera</button>
      <button class="btn" value="Led" onclick="onModeChange(this.value)">Toggle LED</button>
      <p></p>
      <span id="resultLeft"></span> <span id="resultRight"></span>
    </div> 
    <script>
      %VIRTUALJOYSTICK%
    </script>
    <script>
      const map = (value, x1, y1, x2, y2) => (value - x1) * (y2 - x2) / (y1 - x1) + x2;
      var roverConnected = false;
      var timer = null;
      var switchRoverMode = 1000;
      var switchArmMode = 1500;
      var moveHead = false;
      var isLedOn = false;
      var cameraOn = false;
      var connection;
      var reconnectTimerId = 0;

      console.log("touchscreen is", VirtualJoystick.touchScreenAvailable() ? "available" : "not available");

      function connect() {
        connection = new WebSocket('ws://192.168.4.1:80/ws');

        connection.onopen = function () {
          console.log("Websocket Open");
          roverConnected = true;
          reconnectTimerId = 0;
          timer = setInterval(sendData, 50);
        };
  
        connection.onerror = function (error) {
          console.log('WebSocket Error ' + error);
          if (roverConnected) {
            clearInterval(timer);
          }
          roverConnected = false;
        };

        connection.onmessage = function (e) {
          console.log('WS Bin length', e.data.size);
          if (e.data.size !== (13 * 4)) return; // We expect 13 float values.
           var buffer = e.data.arrayBuffer().then((buffer) => {
             var values = new Float32Array(buffer);
             var data = {
              temp: values[0],
              accX: values[1],
              accY: values[2],
              accZ: values[3],
              gyroX: values[4],
              gyroY: values[5],
              gyroZ: values[6],
              gyroAngleX: values[7],
              gyroAngleY: values[8],
              gyroAngleZ: values[9],
              angleX: values[10],
              angleY: values[11],
              angleZ: values[12],
             };
             // TODO Do something with the data
           });
        };

        connection.onclose = function (e) {
            console.log("Websocket close");
            if (roverConnected) {
              clearInterval(timer);
            }
            roverConnected = false;
            if (!reconnectTimerId) {
              reconnectTimerId = setTimeout(function() {
                connect();
              }, 100);
            }
        };
      }
      connect();

      var joystickRight	= new VirtualJoystick({
        container	: document.getElementById('container'),
        strokeStyle	: 'cyan',
        mouseSupport	: true,
        limitStickTravel: true,
      });

      joystickRight.addEventListener('touchStartValidation', function(event) {
        var touch	= event.changedTouches[0];
        if (touch.pageX < window.innerWidth / 2)	return false;
        return true
      });

      joystickRight.addEventListener('touchEnd', function() {

      });

      var joystickLeft	= new VirtualJoystick({
        container	: document.getElementById('container'),
        strokeStyle	: 'orange',
        mouseSupport	: true,
        limitStickTravel: true,
      });

      joystickLeft.addEventListener('touchStartValidation', function(event) {
        var touch	= event.changedTouches[0];
        if (touch.pageX >= window.innerWidth / 2)	return false;
        return true
      });

      joystickLeft.addEventListener('touchEnd', function() {

      });

      sendData = function (e) {
        var channelValues = new Uint16Array(6);
        var xLeft = Math.round(joystickLeft.deltaX());
        var yLeft = Math.round(joystickLeft.deltaY() * -1);
        var xRight = Math.round(joystickRight.deltaX());
        var yRight = Math.round(joystickRight.deltaY() * - 1);

         // If Normal Rover Drive mode use right joystick for steering
         // and left for forward/backward and ignore other value.
         // Makes it a bit easier to drive and to avoid accidentally
         // going forward when you want to just steer.
        if (switchRoverMode == 1000) {
          if (!moveHead) {
            channelValues[0] = map(xRight, -100, 100, 1000, 2000);;
            channelValues[1] = map(yLeft, -100, 100, 1000, 2000);
            channelValues[2] = 1500;
            channelValues[3] = 1500;
          } else if (moveHead === true) {
            channelValues[0] = 1500;
            channelValues[1] = 1500;
            channelValues[2] = map(-xRight, -100, 100, 1000, 2000);
            channelValues[3] = map(yRight, -100, 100, 1000, 2000);
          }
        } else {
          channelValues[0] = map(xRight, -100, 100, 1000, 2000);;
          channelValues[1] = map(yRight, -100, 100, 1000, 2000);;
          channelValues[2] = map(xLeft, -100, 100, 1000, 2000);
          channelValues[3] = map(yLeft, -100, 100, 1000, 2000);;
        }


        channelValues[4] = switchRoverMode;
        channelValues[5] = switchArmMode;

        //console.log(channelValues);

        if (roverConnected) {
          connection.send(channelValues);
        }

        var outputEl	= document.getElementById('resultLeft');
        var outputE2	= document.getElementById('resultRight');
        outputEl.innerHTML	= '<b>Left:</b> '
          + ' dx:' + xLeft
          + ' dy:' + yLeft
        outputE2.innerHTML	= '<b>Right:</b> '
          + ' dx:' + xRight
          + ' dy:' + yRight
      };

      function onModeChange(mode) {
        switch (mode) {
          case 'Normal':
            switchRoverMode = 1000;
            switchArmMode = 1500;
            moveHead = false;
            break;
          case 'Head':
            moveHead = true;
            switchRoverMode = 1000;
            switchArmMode = 1500;
            break;
          case 'Spin':
            switchRoverMode = 1500;
            switchArmMode = 1500;
            moveHead = false;
            break;
          case 'Arm':
            switchRoverMode = 2000;
            switchArmMode = 1000;
            moveHead = false;
            break;
          case 'Gripper':
            switchRoverMode = 2000;
            switchArmMode = 2000;
            moveHead = false;
            break;
          case 'Led':
            var url = 'http://192.168.4.10/control?var=led&val=';
            isLedOn = !isLedOn;
            if (isLedOn === true) {
              url += '1';
            } else {
              url += '0';
            }
            var httpRequest = new XMLHttpRequest();
            httpRequest.open('GET', url);
            httpRequest.send();
            break;
          case 'Camera':
            const blankSrc = 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNkYAAAAAYAAjCB0C8AAAAASUVORK5CYII=';
            cameraOn = !cameraOn;
            if (cameraOn == true) {
              document.getElementById("stream").src = 'http://192.168.4.10:81/stream';
            } else {
              document.getElementById("stream").src = blankSrc;
            }
            break;
          default:
            break;
        }
        sendData();
      }

    </script>
  </body>
</html>
)rawliteral";