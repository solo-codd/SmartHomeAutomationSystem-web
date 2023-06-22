// Libraries
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <RtcDS1302.h>
#include <ThreeWire.h>
#include <DHT.h>
#include <Servo.h>

// Defining the pin numbers for the modules
const int pinEna = 13;
const int pinDat = 11;
const int pinClk = 12;
const int trigPin = 5;
const int echoPin = 6;
const int servoPin = 9;
const int pirPin = 8;
const int relayPin1 = 7;
const int relayPin2 = 4;
const int relayPin3 = 3;
const int buzzerPin = 2;
const int ldrPin = A0;

#define RX_PIN 0
#define TX_PIN 1
#define DHTpin 10
#define DHTTYPE DHT11

// Defining variables to store last read times for each sensor
unsigned long lastLDRRead = 0;
unsigned long lastTempRead = 0;
unsigned long lastPIRRead = 0;
unsigned long lastUSRead = 0;
unsigned long lastRTCRead = 0;

// Defining time intervals for each sensor (in milliseconds)
const unsigned long LDRInterval = 10000; // read every 10 seconds
const unsigned long TempInterval = 10000; // read every 10 seconds
const unsigned long PIRInterval = 3000; // read every 3 second
const unsigned long USInterval = 1000; // read every 1 seconds
const unsigned long RTCInterval = 60000;// read every 60 seconds

// Declarations
int sen1Value = 0;
int distance = 0;
int temperature = 0;
int ldrValue = 0;
int pos = 0;
int tempThreshold = 30;
boolean pirTriggered = false;
int lightThreshold = 500;
volatile int i = 5;

ThreeWire myWire(11, 12, 13); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
DHT dht(DHTpin, DHTTYPE);
SoftwareSerial ss(0, 1);
Servo myservo;

void setup() {
  myservo.attach(servoPin);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pirPin, INPUT);
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayPin3, OUTPUT);
  digitalWrite(relayPin1, LOW);
  digitalWrite(relayPin2, LOW);
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600);
  ss.begin(9600);
  dht.begin();
  Rtc.Begin();
  
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  if (!Rtc.IsDateTimeValid()) {
    Serial.print("   ||RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected()) {
    Serial.print("  ||RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning()) {
    Serial.print("  ||RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled) {
    Serial.print("  ||RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled) {
    Rtc.SetDateTime(compiled);
    Serial.print("  ||RTC is newer than compile time");
  }
  else if (now == compiled) {
    Serial.print("  ||RTC is the same as compile time");
  }
  
}

void loop() {
  bool isCommandReceived = checkCommandFromNodeMCU();
  if (isCommandReceived) {
    // Command received, control the relays based on the command
    // Process the received commands here
  }
  else {
    // No command received, control the relay based on sensor conditions
    //--------servo motor--------------
    if (millis() - lastUSRead >= USInterval) {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      long duration = pulseIn(echoPin, HIGH);
      distance = duration * 0.034 / 2;
      Serial.print("Distance: ");
      Serial.println(distance);
      lastUSRead = millis();
    }
      if (distance < 6 && distance >= 2) { 
    for(pos=0;pos<=90;pos++){
        myservo.write(pos);
       delay(30);
       Serial.print(" ||Door opened");
    }
    delay(2000);
    for(pos=90;pos<=180;pos++){
      myservo.write(pos);
      tone(buzzerPin,100,150);
      delay(30);
      Serial.print("  ||Door Closed");
    }  
    myservo.write(90);
  }


  //-----------fan---------------
  if (millis() - lastPIRRead >= PIRInterval) {
    sen1Value = digitalRead(pirPin);
    Serial.print("  PIR value");
    Serial.print(sen1Value);
    lastPIRRead = millis();
  }
  if (millis() - lastTempRead >= TempInterval) {
   temperature = dht.readTemperature(); 
    Serial.print("  ||Temperature: ");
    Serial.print(temperature);
    Serial.print(" C");
    lastTempRead = millis();
  }

  if (sen1Value == 1) {
      pirTriggered = true;
      Serial.print("  || Motion Detected ");
      if (temperature > tempThreshold) {
        digitalWrite(relayPin1, HIGH);
        Serial.print(":fan on");
        i=5;        
      }
      else {
        digitalWrite(relayPin1, LOW);
        Serial.print(":fan off"); 
        i=5;               
      }
    }
  else if(sen1Value== 0) {
    if (pirTriggered == true && i > 0) {
      Serial.print("  || Motion Detected delay ");
        if (temperature > tempThreshold) {
          digitalWrite(relayPin1, HIGH);       
          Serial.print(":fan on");
          i--;         
          delay (30);          
        }
        else {        
          digitalWrite(relayPin1, LOW);
          Serial.print(":fan off");
          i-- ;               
          delay(30);
        }
    }      
    else{
      pirTriggered = false;
      Serial.print("  || NO Motion Detected  ");
      digitalWrite(relayPin1, LOW);
      Serial.println(":fan off");
      i=0;     
      } 
  }
  
  //-------bulb---------------
  if (millis() - lastLDRRead >= LDRInterval) {
   ldrValue = analogRead(ldrPin);
    Serial.print("  LDR value:");
    Serial.print(ldrValue);
    lastLDRRead = millis();
  }

  if (ldrValue < lightThreshold && pirTriggered) {
     digitalWrite(relayPin2, HIGH);
     Serial.print(" Bulb on");
   } 
  else {
      digitalWrite(relayPin2, LOW);
      Serial.print("  Bulb off");
    }

  //------------rtc-------------
  if (millis() - lastRTCRead >= RTCInterval) {
    RtcDateTime now = Rtc.GetDateTime();    // get current time
    if (!now.IsValid()) {
        Serial.print(" ||RTC lost confidence in the DateTime!");
    }  
    if (now.Hour() >= 6 && now.Hour() < 24)  {
      digitalWrite(relayPin3, HIGH); // turn on relay pin
      Serial.print("  ||Socket ON");
    } 
    else {
      digitalWrite(relayPin3, LOW); // turn off relay pin
      Serial.print("  ||Socket OFF");
    }
    lastRTCRead = millis(); 
  }

    // Create a JSON document
    DynamicJsonDocument jsonDoc(2048);

    // Add sensor data to the JSON document
    jsonDoc["ldrValue"] = ldrValue;
    jsonDoc["temperature"] = temperature;
    jsonDoc["distance"] = distance;

    // Serialize the JSON document to a string
    String jsonStr;
    serializeJson(jsonDoc, jsonStr);

    // Send the JSON data to NodeMCU
    ss.println(jsonStr);

    delay(2000);
  }
}

bool checkCommandFromNodeMCU() {
  if (ss.available()) {
    String receivedData = ss.readStringUntil('\n');
    DynamicJsonDocument doc(2048);

    DeserializationError error = deserializeJson(doc, receivedData);
    if (error) {
      Serial.print("JSON deserialization failed: ");
      Serial.println(error.c_str());
      return false;
    }

    if (doc.containsKey("command")) {
      String command = doc["command"].as<String>();
      Serial.println("Received command from NodeMCU: " + command);

      // Process the received command here
      if (command == "LIGHT_OFF") {
        // Code to turn off the lights using the relay
        digitalWrite(relayPin2, HIGH); // Assuming relayPin2 controls the lights
        Serial.println("Lights turned off");
      }
      // Add more conditions for different commands as per your requirements

      return true;
    }
  }
  return false;
}
