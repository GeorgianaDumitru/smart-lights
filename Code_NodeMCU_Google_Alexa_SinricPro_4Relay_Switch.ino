#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "SinricPro.h"
#include "SinricProSwitch.h"

#include <map>

#define WIFI_SSID         "GoZiLlA Home AP"    
#define WIFI_PASS         "masterpiece12"
#define APP_KEY           "1622b872-b217-4c80-a27b-58ffb2d013e8"    
#define APP_SECRET        "4c6ff950-4ef1-47f1-95bf-4e53043b5505-0a4fe6d2-42e4-4e65-9efb-d34f75a0cfe1"   


#define device_ID_1   "62b02759fce0b9e02e731882"
#define device_ID_2   "62a8db6efce0b9e02e6fa650"
#define device_ID_3   "62a8db7ffb740f77fc0ec5d5"
#define device_ID_4   "62a8db8afce0b9e02e6fa68a"
#define device_ID_5   "62b0276cfce0b9e02e7318a9"


#define RelayPin1 5  
#define RelayPin2 4  
#define RelayPin3 14 
#define RelayPin4 12 
#define RelayPin5 16 

#define SwitchPin1 10 
#define SwitchPin2 0   
#define SwitchPin3 13  
#define SwitchPin4 3   
#define SwitchPin5 2   

#define wifiLed   16   



#define DEBOUNCE_TIME 250

typedef struct {      
  int relayPIN;
  int flipSwitchPIN;
} deviceConfig_t;



std::map<String, deviceConfig_t> devices = {

    {device_ID_1, {  RelayPin1, SwitchPin1 }},
    {device_ID_2, {  RelayPin2, SwitchPin2 }},
    {device_ID_3, {  RelayPin3, SwitchPin3 }},
    {device_ID_4, {  RelayPin4, SwitchPin4 }},
    {device_ID_5, {  RelayPin5, SwitchPin5 }}     
};

typedef struct {      
  String deviceId;
  bool lastFlipSwitchState;
  unsigned long lastFlipSwitchChange;
} flipSwitchConfig_t;

std::map<int, flipSwitchConfig_t> flipSwitches;   

void setupRelays() { 
  for (auto &device : devices) {          
    int relayPIN = device.second.relayPIN; 
    pinMode(relayPIN, OUTPUT);         
    digitalWrite(relayPIN, HIGH);
  }
}

void setupFlipSwitches() {
  for (auto &device : devices)  {                   
    flipSwitchConfig_t flipSwitchConfig;           

    flipSwitchConfig.deviceId = device.first;     
    flipSwitchConfig.lastFlipSwitchChange = 0;      
    flipSwitchConfig.lastFlipSwitchState = true;    

    int flipSwitchPIN = device.second.flipSwitchPIN;  

    flipSwitches[flipSwitchPIN] = flipSwitchConfig;  
    pinMode(flipSwitchPIN, INPUT_PULLUP);                 
  }
}

bool onPowerState(String deviceId, bool &state)
{
  Serial.printf("%s: %s\r\n", deviceId.c_str(), state ? "on" : "off");
  int relayPIN = devices[deviceId].relayPIN; 
  digitalWrite(relayPIN, !state);            
  return true;
}

void handleFlipSwitches() {
  unsigned long actualMillis = millis();                                          
  for (auto &flipSwitch : flipSwitches) {                                        
    unsigned long lastFlipSwitchChange = flipSwitch.second.lastFlipSwitchChange; 

    if (actualMillis - lastFlipSwitchChange > DEBOUNCE_TIME) {                   

      int flipSwitchPIN = flipSwitch.first;                                      
      bool lastFlipSwitchState = flipSwitch.second.lastFlipSwitchState;           
      bool flipSwitchState = digitalRead(flipSwitchPIN);                          
      if (flipSwitchState != lastFlipSwitchState) {                               
   
          flipSwitch.second.lastFlipSwitchChange = actualMillis;                 
          String deviceId = flipSwitch.second.deviceId;                          
          int relayPIN = devices[deviceId].relayPIN;                            
          bool newRelayState = !digitalRead(relayPIN);                        
          digitalWrite(relayPIN, newRelayState);                                 

          SinricProSwitch &mySwitch = SinricPro[deviceId];                    
          mySwitch.sendPowerStateEvent(!newRelayState);                      
        flipSwitch.second.lastFlipSwitchState = flipSwitchState;              
      }
    }
  }
}

void setupWiFi()
{
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf(".");
    delay(250);
  }
  digitalWrite(wifiLed, LOW);
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

void setupSinricPro()
{
  for (auto &device : devices)
  {
    const char *deviceId = device.first.c_str();
    SinricProSwitch &mySwitch = SinricPro[deviceId];
    mySwitch.onPowerState(onPowerState);
  }

  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
}

void setup()
{
  Serial.begin(BAUD_RATE);

  pinMode(wifiLed, OUTPUT);
  digitalWrite(wifiLed, HIGH);

  setupRelays();
  setupFlipSwitches();
  setupWiFi();
  setupSinricPro();
}

void loop()
{
  SinricPro.handle();
  handleFlipSwitches();
}
