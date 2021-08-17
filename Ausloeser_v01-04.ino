// WifiManager
//#include <WiFi.h>
//#include <DNSServer.h>
//#include <ESP8266WebServer.h>
//#include <WiFiManager.h>

// ESP32 + GSM MODULE
#include "Adafruit_FONA.h"
#define SIM800L_RX     27
#define SIM800L_TX     26
#define SIM800L_PWRKEY 4
#define SIM800L_RST    5
#define SIM800L_POWER  23
char replybuffer[255];
HardwareSerial *sim800lSerial = &Serial1;
Adafruit_FONA sim800l = Adafruit_FONA(SIM800L_PWRKEY);
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
#define LED_BLUE  13
#define RELAY 14
String smsString = "";

// Servo requirements
// Include the ESP32 Arduino Servo Library instead of the original Arduino Servo Library
#include <ESP32Servo.h> 
 
Servo myservo;  // create servo object to control a servo
// Possible PWM GPIO pins on the ESP32: 0(used by on-board button),2,4,5(used by on-board LED),12-19,21-23,25-27,32-33 
int servoPin = 18;      // GPIO pin used to connect the servo control (digital out)
// Possible ADC pins on the ESP32: 0,2,4,12-15,32-39; 34-39 are recommended for analog input
int ADC_Max = 4096;     // This is the default ADC max value on the ESP32 (12 bit ADC width);
                        // this width can be set (in low-level oode) from 9-12 bits, for a
                        // a range of max values of 512-4096
int pos = 0;      // position in degrees

void setup() {
  //Setup Receiver SMS
  pinMode(LED_BLUE, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(SIM800L_POWER, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(SIM800L_POWER, HIGH);
  Serial.begin(115200);
  Serial.println(F("ESP32 with GSM SIM800L"));
  Serial.println(F("Initializing....(May take more than 10 seconds)"));
  delay(10000);
  // Make it slow so its easy to read!
  sim800lSerial->begin(4800, SERIAL_8N1, SIM800L_TX, SIM800L_RX);
  if (!sim800l.begin(*sim800lSerial)) {
    Serial.println(F("Couldn't find GSM SIM800L"));
    while (1);
  }
  Serial.println(F("GSM SIM800L is OK"));
  char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = sim800l.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("SIM card IMEI: "); Serial.println(imei);
  }
  // Set up the FONA to send a +CMTI notification
  // when an SMS is received
  sim800lSerial->print("AT+CNMI=2,1\r\n");
  Serial.println("GSM SIM800L Ready");

  //remove all SMS from index
  removeAllSMS();
  
  //Setup SEND EMAIL
  //WiFiManager wifiManager;
  //cria uma rede de nome ESP_AP com senha 12345678
  //wifiManager.autoConnect("ESP_32", "00000000");

  //Setup Servo
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);// Standard 50hz servo
  myservo.attach(servoPin, 1000, 2000);
  myservo.write(2500); //ausgangsposition
  delay(1000);
}

// More info receiver SMS Between setup and loop
long prevMillis = 0;
int interval = 1000;
char sim800lNotificationBuffer[64];          //for notifications from the FONA
char smsBuffer[250];
boolean ledState = false;


void loop() {
  // JUST SHOW SMS
  if (millis() - prevMillis > interval) {
    ledState = !ledState;
    digitalWrite(LED_BLUE, ledState);
    prevMillis = millis();
  }
  String buffer2;
  int data = 0;
  while (sim800l.available()) {
          char c = sim800l.read();
          //Serial.print(c);
          buffer2.concat(c);
          delay(10);
          data = 1;
  }
  if (data == 1){
    Serial.print(buffer2);
    if (buffer2.indexOf("+CMTI:") > 0){
        Serial.println("identify SMS receive");
        //get the sms message index +CMTI: "SM",18 -> index 18
        String part01 = getValue(buffer2,',',1);
        String smscontent = getSMSContent(part01.toInt());
        smscontent.toLowerCase();
        Serial.print("Switch based on: "); Serial.println(smscontent);
        switchCase(part01.toInt(), smscontent);
        removeSMS(part01.toInt());
    }
    data = 0;
  }
}

void switchCase(int smsIndexSlot, String content){
  if(content == "on"){
        triggerServo();
        SendSMS(smsIndexSlot,"Es wurde ausgeloest.\n Danke fuer die nutzung des Ausloeser Service. \n Viele Gruesse \n Chris & Vik.");
  } else if(content == "off"){
        SendSMS(smsIndexSlot,"Es wurde nicht ausgeloest.\n Danke fuer die nutzung des Ausloeser Service. \n Viele Gruesse \n Chris & Vik.");
  } else if(content == "status") {
        SendSMS(smsIndexSlot,"Status von deinem Ausloeser ist...");
  } else if(content == "help"){
        SendSMS(smsIndexSlot,"Es existieren die folgenden Befehle: \n [1] on \n [2] off \n [3] status \n [4] help \nViel Dank.");
  } else {
        SendSMS(smsIndexSlot,"Hi,\num mit dem ausloer zu kommunizieren koennen folgenden Befehle ausgefuert werden: \n [1] on \n [2] off \n [3] status \n [4] help \n \nViele Gruesse \nChris & Vik.");
  }

}

void triggerServo(){
    //myservo.write(400); //ausloser position
    myservo.write(2500); //ausgangsposition
    delay(1000);
    myservo.write(600); //ausloser position
    delay(1000);
    myservo.write(2500); //ausgangsposition
    delay(5000);
}

void removeAllSMS(){
  for (int i = 0; i < 20; i++) {
    sim800l.deleteSMS(i);  
  }
  Serial.println(F("All SMS Indexe cleared!"));
}

void removeSMS(int smsIndexSlot){
   // mazání SMS na kartě (20 pozic) 
   sim800l.deleteSMS(smsIndexSlot);
   /*
  for (int i = 0; i < 20; i++) {
    sim800l.deleteSMS(i);  
  }
  */
  Serial.println(F("SMS Index cleared!"));
}

String getSMSContent(int smsIndexSlot){
      // Retrieve SMS value.
      uint16_t smslen;
      // Pass in buffer and max len!
      if (sim800l.readSMS(smsIndexSlot, smsBuffer, 250, &smslen)) {
        // Atribui a mensagem a baixo em smsString
        smsString = String(smsBuffer); 
        //Serial.println(smsString);
        //SendSMS();
      }  
      return smsString;
}

void SendSMS(int smsIndexSlot, String message) {
  char callerIDbuffer[32];  //we'll store the SMS sender number in here
      // Retrieve SMS sender address/phone number.
  if (!sim800l.getSMSSender(smsIndexSlot, callerIDbuffer, 31)) {
        Serial.println("Didn't find SMS message in slot!");
  }
  Serial.print(F("FROM: ")); Serial.println(callerIDbuffer);
  String dobbleQuots = "\"";
  String textMessages = "AT+CMGS=" + dobbleQuots + String(callerIDbuffer) + dobbleQuots + "\r";
  Serial.println(textMessages); 
  Serial.println("Sending SMS...");               //Show this message on serial monitor
  sim800l.print("AT+CMGF=1\r");                   //Set the module to SMS mode
  delay(100);
  sim800l.print(textMessages);  //Your phone number don't forget to include your country code, example +212123456789"
  delay(500);
  sim800l.print(message);       //This is the text to send to the phone number, don't make it too long or you have to modify the SoftwareSerial buffer
  delay(500);
  sim800l.print((char)26);// (required according to the datasheet)
  delay(500);
  sim800l.println();
  Serial.println("Text Sent.");
  delay(500);
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
