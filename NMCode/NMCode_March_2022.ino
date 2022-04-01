
/*
   NMCode by this.is.NOISE inc. 

   https://github.com/thisisnoiseinc/NMCode

   Built upon:
    
    "BLE_MIDI Example by neilbags 
    https://github.com/neilbags/arduino-esp32-BLE-MIDI
    
    Based on BLE_notify example by Evandro Copercini."
*/



#include <BLEDevice.h>

#include <BLEUtils.h>

#include <BLEServer.h>

#include <BLE2902.h>

#include "esp_bt_main.h"

#include "esp_bt_device.h"

#define SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"

#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"


int potPin = 36; // Slider
int rotPin = 39; // Rotary Knob
bool rotMoving = true;
int midiCState = 0; // General current state
int led_Blue = 14; // BLE LED
int led_Green = 4; // CHANNEL LED
const int button = 12;
int potCstate = 0; // Slider current state
int rotCState = 0; // Rotary Knob current state
int outputValue = 0;
int ButtonNote = 0;
int Channel_SelectON = 0;
int Channel_SelectOFF = 0;
int Channel_SelectCC = 0;
int Buttonselect[button] = {  // Buttons put in order of reference board.
 16,
 17,
 18,
 21,
 19,
 25,
 22,
 23,
 27,
 26,
 35,
 34
  };
int buttonCstate[button] = {0}; // Button current state
int buttonPState[button] = {0}; // Button previous state
int OffNote[button] = {0};
int debounceDelay = 5;
int lastDebounceTime[button] = {0};
int i = 0;
const int numReadings = 15;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average1 = 0; // average current state
int lastaverage1 = 0; // average previous state



BLECharacteristic *pCharacteristic;

bool deviceConnected = false;

uint8_t midiPacket[] = {

   0x80,  // header

   0x80,  // timestamp, not implemented 

   0x00,  // status

   0x3c,  // 0x3c == 60 == middle c

   0x00   // velocity

};

class MyServerCallbacks: public BLEServerCallbacks {

    void onConnect(BLEServer* pServer) {

      deviceConnected = true;

    };



    void onDisconnect(BLEServer* pServer) {

      deviceConnected = false;

    }

};

bool initBluetooth()
{
  if (!btStart()) {
    Serial.println("Failed to initialize controller");
    return false;
  }
 
  if (esp_bluedroid_init() != ESP_OK) {
    Serial.println("Failed to initialize bluedroid");
    return false;
  }
 
  if (esp_bluedroid_enable() != ESP_OK) {
    Serial.println("Failed to enable bluedroid");
    return false;
  }
 
}

void setup() {

  Serial.begin(115200);

  initBluetooth();
  const uint8_t* point = esp_bt_dev_get_address();
 
  char str[6];
 
  sprintf(str, "NMSVE %02X %02X %02X", (int)point[3], (int)point[4], (int)point[5]);
  Serial.print(str);

  BLEDevice::init(str);

    

  // Create the BLE Server

  BLEServer *pServer = BLEDevice::createServer();

  pServer->setCallbacks(new MyServerCallbacks());



  // Create the BLE Service

  BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));



  // Create a BLE Characteristic

  pCharacteristic = pService->createCharacteristic(

    BLEUUID(CHARACTERISTIC_UUID),

    BLECharacteristic::PROPERTY_READ   |

    BLECharacteristic::PROPERTY_WRITE  |

    BLECharacteristic::PROPERTY_NOTIFY |

    BLECharacteristic::PROPERTY_WRITE_NR

  );



  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml

  // Create a BLE Descriptor

  pCharacteristic->addDescriptor(new BLE2902());



  // Start the service

  pService->start();



  // Start advertising

  BLEAdvertising *pAdvertising = pServer->getAdvertising();

  pAdvertising->addServiceUUID(pService->getUUID());

  pAdvertising->start();

  // Initialize buttons + LED's

   for (int i = 0; i < button; i++){
  pinMode (Buttonselect[i], INPUT);
  pinMode (led_Blue, OUTPUT);
  pinMode (led_Green, OUTPUT);
  }

  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  while (Channel_SelectON == 0) {
  
  digitalWrite(led_Green, HIGH);
  
    for (int i = 0; i < button; i++) {
      buttonCstate[i] = digitalRead(Buttonselect[i]);
      if (buttonCstate[i] == HIGH) {
        Channel_SelectON = (i + 144);
        Channel_SelectOFF = (i + 128);
        Channel_SelectCC = (i + 176);
      }
    }
      }
      
        digitalWrite(led_Green, LOW);

}

void loop(){

 // Ensure device is connected to BLE

  if (deviceConnected == false) { 
  digitalWrite(led_Blue, HIGH);
  delay(1000);
  digitalWrite(led_Blue, LOW);
  delay(1000);
}

 // Enter Default Mode

else {

  digitalWrite(led_Blue, HIGH);
  BUTTONS();
  ROTARY();
}
}
 

// Control Button functions for Default Mode

void BUTTONS(){

 for (int i = 0; i < button; i++){
buttonCstate[i] = digitalRead(Buttonselect[i]);
potCstate = analogRead(potPin);
outputValue = map(potCstate, 0, 4095, 3, 9);
ButtonNote = (outputValue * 12 + i);

if (outputValue == 3 || outputValue == 5 || outputValue == 7 || outputValue == 9) {
  digitalWrite(led_Green, HIGH);
}

else {
  digitalWrite(led_Green, LOW);
}
 
 if ((millis() - lastDebounceTime[i]) > debounceDelay) {
  
  if (buttonPState[i] != buttonCstate[i]) {
        lastDebounceTime[i] = millis();

  if (buttonCstate[i] == HIGH) {  

   midiPacket[2] = Channel_SelectON;
   Serial.println(Channel_SelectON); 

   midiPacket[3] = ButtonNote;
   Serial.println(midiPacket[3]);

   midiPacket[4] = 100;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   OffNote[i] = ButtonNote;
  }

 else {
  midiPacket[2] = Channel_SelectOFF;
   Serial.println(Channel_SelectOFF);

   midiPacket[3] = OffNote[i];

   midiPacket[4] = 0;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();
 }
buttonPState[i] = buttonCstate[i];
}
}
  }
}

void potaverage1() {
  
  for (int p = 0; p < 15; p++) {
  rotCState = analogRead(rotPin);
  midiCState = map(rotCState, 0, 4095, 0, 127);
  
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = midiCState;
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average1 = total / numReadings;
  delay(1);        // delay in between reads for stability
}
}

// Control Rotary Knob functions for Default + Split Mode

void ROTARY(){

 potaverage1();
 
 if (average1 != lastaverage1) {
    rotMoving = true;
  }

  else {
    rotMoving = false;
  }

  if (rotMoving == true) {
    
   midiPacket[2] = Channel_SelectCC;
   Serial.println(Channel_SelectCC);

   midiPacket[3] = 0x01;
   Serial.println(0x01);

   midiPacket[4] = average1;
   Serial.println(average1);

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   lastaverage1 = average1;
 }
  }
