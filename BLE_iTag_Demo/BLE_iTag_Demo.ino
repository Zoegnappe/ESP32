

#include "BLEDevice.h"

/* Pin defenitions */
#define INP 0 //input pin Immediate Alert button
#define OUT 2 //Remote controlled output pin

/* Services and Characteristics defenitions */ 
#define GENERIC_ACCES   0x1800
#define DEVICE_NAME     0x2A00
#define BATTERY_SERVICE 0x180F
#define BATTERY_LEVEL   0x2A19
#define IMMEDIATE_ALERT 0x1802
#define ALERT_LEVEL     0x2A06
#define NOTIF_SERVICE   0xFFE0
#define NOTIF_CHARACT   0xFFE1

/* File globals */
//Address of my iTag (find with e.g. "nRF Connect" Android App)  
static uint8_t iTagAddress[] = {0xFF,0xFF,0xC0,0x0B,0xF8,0x09};
static BLEClient* pClient;
static bool outState = false;

/*************************************************************************************/
static void notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  outState^=1;    //toggle 
}

/************************************************************************************/
// Connect to the iTag(Server=Slave).
bool connectToServer() {
  pClient = BLEDevice::createClient();
  pClient->connect(*(new BLEAddress(iTagAddress)));
  if(pClient->isConnected()){ 
    getCharacteristic(NOTIF_SERVICE, NOTIF_CHARACT)->registerForNotify(notifyCallback);
    return(true);
  }  
  return(false);
}

/************************************************************************************/
BLERemoteCharacteristic* getCharacteristic(uint16_t serviceID, uint16_t charactID){
  BLEUUID serviceUUID(BLEUUID((uint16_t)serviceID));
  BLEUUID charactUUID(BLEUUID((uint16_t)charactID));
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  BLERemoteCharacteristic* pCharact = pRemoteService->getCharacteristic(charactUUID);
  return(pCharact);
}

/************************************************************************************/
char* deviceName(){
  return((char*)getCharacteristic(GENERIC_ACCES,DEVICE_NAME)->readValue().c_str());
}

/************************************************************************************/
uint8_t batteryLevel(){
  return(getCharacteristic(BATTERY_SERVICE,BATTERY_LEVEL)->readUInt8());
}

/************************************************************************************/
void alertLevel(uint8_t level){
  getCharacteristic(IMMEDIATE_ALERT,ALERT_LEVEL)->writeValue(level);
}

/************************************************************************************/
void handle_remote_control(){
  static bool outLastState = false;
  if(outState != outLastState){
    outLastState = outState;
    Serial.printf("Output State  : %s\n", outState?"ON":"OFF");
    digitalWrite(OUT, outState);
  }
}

/************************************************************************************/
void handle_immediate_alert(){
  static bool inpState = false,inpLastState = false;
  static bool alertState = false;
  inpState = digitalRead(INP);
  if(inpState != inpLastState){
    inpLastState = inpState;
    if(inpState == LOW){
      alertState ^= 1;
      Serial.printf("Alert State   : %s\n", alertState?"ON":"OFF");
      alertLevel(alertState?2:0);
    }  
  }  
}
/************************************************************************************/
/************************************************************************************/
void setup() {
  Serial.begin(115200);
  Serial.println("\n\iTag BLE Client");
  pinMode(OUT,OUTPUT);
  digitalWrite(OUT,LOW);
  
  BLEDevice::init("");
  do{  
    Serial.println("Connecting..."); 
    Serial.print(connectToServer()?"Connected":"Failed to connect"); 
    Serial.println(" to iTag.");
  } while(!pClient->isConnected());
  //Connected
  alertLevel(2); delay(500); alertLevel(0);
  Serial.printf("Device Name   : %s\n",deviceName()); 
  Serial.printf("Battery Level : %i %s\n\n",batteryLevel(),"%");
}

void loop() {
  if(pClient->isConnected()){
    handle_remote_control();
    handle_immediate_alert();
  }else{ 
    Serial.println("Reconnecting...");
    Serial.print(connectToServer()?"Connected":"Failed to connect"); 
    Serial.println(" to iTag.");
  }
  delay(50);
}
