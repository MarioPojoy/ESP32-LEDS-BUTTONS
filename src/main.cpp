#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "credentials.h"

const char* PARAM_INPUT = "state";

const int output = GPIO_NUM_18;
const int output2 = GPIO_NUM_19;
const int output3 = GPIO_NUM_23;

const int buttonPin = GPIO_NUM_17;
const int buttonPin2 = GPIO_NUM_16;
const int buttonPin3 = GPIO_NUM_27;

const int readyLed = GPIO_NUM_2;

// Variables will change:
int ledState = HIGH;          // the current state of the output pin
int ledState2 = HIGH;
int ledState3 = HIGH;

int buttonState;             // the current reading from the input pin
int buttonState2;
int buttonState3;

int lastButtonState = HIGH;   // the previous reading from the input pin
int lastButtonState2 = HIGH;
int lastButtonState3 = HIGH;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 CONTROL TABLERO</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>TABLERO</h2>
  %BUTTONPLACEHOLDER%
<script>
function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?state=1", true); }
  else { xhr.open("GET", "/update?state=0", true); }
  xhr.send();
}
function toggleCheckbox2(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update2?state=1", true); }
  else { xhr.open("GET", "/update2?state=0", true); }
  xhr.send();
}
function toggleCheckbox3(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update3?state=1", true); }
  else { xhr.open("GET", "/update3?state=0", true); }
  xhr.send();
}

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      if( this.responseText == 1){ 
        inputChecked = false;
        outputStateM = "APAGADA";
      }
      else { 
        inputChecked = true;
        outputStateM = "ENCENDIDA";
      }
      document.getElementById("output").checked = inputChecked;
      document.getElementById("outputState").innerHTML = outputStateM;
    }
  };
  xhttp.open("GET", "/state", true);
  xhttp.send();
}, 500 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      if( this.responseText == 1){ 
        inputChecked = false;
        outputStateM = "APAGADA";
      }
      else { 
        inputChecked = true;
        outputStateM = "ENCENDIDA";
      }
      document.getElementById("output2").checked = inputChecked;
      document.getElementById("outputState2").innerHTML = outputStateM;
    }
  };
  xhttp.open("GET", "/state2", true);
  xhttp.send();
}, 500 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      if( this.responseText == 1){ 
        inputChecked = false;
        outputStateM = "APAGADA";
      }
      else { 
        inputChecked = true;
        outputStateM = "ENCENDIDA";
      }
      document.getElementById("output3").checked = inputChecked;
      document.getElementById("outputState3").innerHTML = outputStateM;
    }
  };
  xhttp.open("GET", "/state3", true);
  xhttp.send();
}, 500 ) ;
</script>
</body>
</html>
)rawliteral";

String outputState(){
  if(ledState){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}

String outputState2(){
  if(ledState2){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}

String outputState3(){
  if(ledState3){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    String outputStateValue = outputState();
    String outputStateValue2 = outputState2();
    String outputStateValue3 = outputState3();
    buttons+= "<h4> LUZ <span id=\"outputState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";
    buttons+= "<h4> TOMA 1 <span id=\"outputState2\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox2(this)\" id=\"output2\" " + outputStateValue2 + "><span class=\"slider\"></span></label>";
    buttons+= "<h4> TOMA 2 <span id=\"outputState3\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox3(this)\" id=\"output3\" " + outputStateValue3 + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(output, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);
  pinMode(readyLed, OUTPUT);
  digitalWrite(output, HIGH);
  digitalWrite(output2, HIGH);
  digitalWrite(output3, HIGH);
  digitalWrite(readyLed, LOW);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  digitalWrite(readyLed, HIGH);
  
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      inputParam = PARAM_INPUT;
      digitalWrite(output, inputMessage.toInt());
      ledState = !ledState;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  server.on("/update2", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      inputParam = PARAM_INPUT;
      digitalWrite(output2, inputMessage.toInt());
      ledState2 = !ledState2;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  server.on("/update3", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      inputParam = PARAM_INPUT;
      digitalWrite(output3, inputMessage.toInt());
      ledState3 = !ledState3;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  // Send a GET request to <ESP_IP>/state
  server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(digitalRead(output)).c_str());
  });

  server.on("/state2", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(digitalRead(output2)).c_str());
  });

  server.on("/state3", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(digitalRead(output3)).c_str());
  });
  
  // Start server
  server.begin();
}
  
void loop() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);
  int reading2 = digitalRead(buttonPin2);
  int reading3 = digitalRead(buttonPin3);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == LOW) {
        ledState = !ledState;
      }
    }
  }

  if (reading2 != lastButtonState2) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading2 != buttonState2) {
      buttonState2 = reading2;

      // only toggle the LED if the new button state is HIGH
      if (buttonState2 == LOW) {
        ledState2 = !ledState2;
      }
    }
  }

  if (reading3 != lastButtonState3) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading3 != buttonState3) {
      buttonState3 = reading3;

      // only toggle the LED if the new button state is HIGH
      if (buttonState3 == LOW) {
        ledState3 = !ledState3;
      }
    }
  }

  // set the LED:
  digitalWrite(output, ledState);
  digitalWrite(output2, ledState2);
  digitalWrite(output3, ledState3);

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
  lastButtonState2 = reading2;
  lastButtonState3 = reading3;
}