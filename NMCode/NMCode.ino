/*
   NMCode by THIS.IS.NOISE Inc. 

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

#define SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"

#define CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"


int potPin = 36; // Slider
int rotPin = 39; // Rotary Knob
int rotPState = 0; // Rotary Knob previous state
bool rotMoving = true;
bool potMoving = true;
int midiCState = 0; // General current state
int midiPState = 0; // General previous state
int led_Blue = 14; // BLE LED
int led_Green = 4; // CHANNEL LED
const int button = 12;
int potCstate = 0; // Slider current state
int rotCState = 0; // Rotary Knob current state
int outputValue = 0;
int outputValuePState = 0;
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
int samplemode = 0;
int buttonCstate[button] = {0}; // Button current state
int buttonPState[button] = {0}; // Button previous state
int OffNote[button] = {0};
int debounceDelay = 5;
int lastDebounceTime[button] = {0};
int i = 0;
int R_Note[17];
int R_Velocity = 100;
int buttonPushCounter = 0;
int selectpause = 0;



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



void setup() {

  Serial.begin(115200);

  BLEDevice::init("NOISE MACHINE");

    

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

}

// Set Main Menu. Choose from 3 MODES.

  void MAINMENU() {

digitalWrite(led_Blue, LOW);
 
  while (samplemode == 0) {
    buttonCstate[0] = digitalRead(Buttonselect[0]);
    buttonCstate[1] = digitalRead(Buttonselect[1]);
    buttonCstate[2] = digitalRead(Buttonselect[2]);
    
    if (buttonCstate[0] == HIGH) {
      selectpause = 1;
    }

    else {

if (selectpause == 1) {
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
  samplemode = 1;
  break;
}
    }
    
  if (buttonCstate[1] == HIGH) {
    samplemode = 2;
    break;
  }
  if (buttonCstate[2] == HIGH) {
    samplemode = 3;
    break;
  }
  }
}

// After Initial Main Menu go to Loop and run through 1 of 3 MODES.

void loop(){

MAINMENU();

 // Ensure device is connected to BLE

 while (samplemode == 1) {
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
MENURETURN();
}
 }

 // Ensure device is connected to BLE

 while (samplemode == 2) {
   if (deviceConnected == false) { 
  digitalWrite(led_Blue, HIGH);
  delay(1000);
  digitalWrite(led_Blue, LOW);
  delay(1000);
}

 // Enter Split Mode

else {
  digitalWrite(led_Blue, HIGH);
  Channel_SelectCC = 176;
  SMODE();
  ROTARY();
  MENURETURN();
}
 }

 // Ensure device is connected to BLE

while (samplemode == 3) {
  if (deviceConnected == false) { 
  digitalWrite(led_Blue, HIGH);
  delay(1000);
  digitalWrite(led_Blue, LOW);
  delay(1000);
}

 // Enter Loop Mode

else {
  digitalWrite(led_Blue, HIGH);
  Channel_SelectON = 144;
  Channel_SelectOFF = 128;
  Channel_SelectCC = 176;
  LMODE();
  MENURETURN();
}
}
}

// Control Button functions for Default Mode

void BUTTONS(){
 for (int i = 0; i < button; i++){
buttonCstate[i] = digitalRead(Buttonselect[i]);
potCstate = analogRead(potPin);
outputValue = map(potCstate, 0, 4095, 2, 8);
ButtonNote = (outputValue * 12 + i);
 
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

// Control Rotary Knob functions for Default + Split Mode

void ROTARY(){
rotCState = analogRead(rotPin);
midiCState = map(rotCState, 0, 4095, 127, 0);

 if (midiPState > midiCState + 6 || 
      midiPState < midiCState - 6) {
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

   midiPacket[4] = midiCState;
   Serial.println(midiCState);

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   midiPState = midiCState;
 }
  }

  // Controls Split Mode functions

  void SMODE() {
 for (int i = 0; i < button; i++){
buttonCstate[i] = digitalRead(Buttonselect[i]);
potCstate = analogRead(potPin);
outputValue = map(potCstate, 0, 4095, 2, 8);
ButtonNote = (outputValue * 12 + i);

if (i <= 5) {
   if ((millis() - lastDebounceTime[i]) > debounceDelay) {
  
  if (buttonPState[i] != buttonCstate[i]) {
        lastDebounceTime[i] = millis();

  if (buttonCstate[i] == HIGH) {  
   Channel_SelectON = 144;
   midiPacket[2] = Channel_SelectON;
   Serial.println(Channel_SelectON); 

   midiPacket[3] = ButtonNote;
   Serial.println(ButtonNote);

   midiPacket[4] = 100;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   OffNote[i] = ButtonNote;
  }
  
  else{
    midiPacket[2] = 128;

   midiPacket[3] = OffNote[i];

   midiPacket[4] = 0;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();
  }
buttonPState[i] = buttonCstate[i];
}
}
}

if (i >= 6) {
 if ((millis() - lastDebounceTime[i]) > debounceDelay) {
  
  if (buttonPState[i] != buttonCstate[i]) {
        lastDebounceTime[i] = millis();

  if (buttonCstate[i] == HIGH) {  
   Channel_SelectON = 145;
   midiPacket[2] = Channel_SelectON;
   Serial.println(Channel_SelectON); 

   midiPacket[3] = ButtonNote;
   Serial.println(ButtonNote);

   midiPacket[4] = 100;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   OffNote[i] = ButtonNote;
  }
  
  else{
    midiPacket[2] = 129;

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
  }

  // Controls Loop Mode functions

  void LMODE() {
   if (midiCState == 600) {
    for (int i = 0; i < button; i++){
buttonCstate[i] = digitalRead(Buttonselect[i]);
potCstate = analogRead(potPin);
outputValue = map(potCstate, 0, 4095, 2, 8);
ButtonNote = (outputValue * 12 + i);
rotCState = analogRead(rotPin);
midiCState = map(rotCState, 0, 4095, 20, 600);


  if ((millis() - lastDebounceTime[i]) > debounceDelay) {
  
  if (buttonPState[i] != buttonCstate[i]) {
        lastDebounceTime[i] = millis();

  if (buttonCstate[i] == HIGH) {  

   if (ButtonNote != 24) {
   midiPacket[2] = Channel_SelectON;
   Serial.println(Channel_SelectON); 

   R_Note[buttonPushCounter] = ButtonNote;
   midiPacket[3] = ButtonNote;
   Serial.println(ButtonNote);

    midiPacket[4] = 100;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   OffNote[i] = ButtonNote;
  }

  else {
    R_Note[buttonPushCounter] = 24;
    if (buttonPushCounter < 16) {
      buttonPushCounter++;
    }
  }
  }
  else {
    if (ButtonNote != 24) {
    midiPacket[2] = Channel_SelectOFF;
   Serial.println(Channel_SelectOFF);

   midiPacket[3] = OffNote[i];

   midiPacket[4] = 0;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();
    if (buttonPushCounter < 16) {
      buttonPushCounter++;
    }
  }
  }
  buttonPState[i] = buttonCstate[i];
}
  }
  }
    }
    
  else {
rotCState = analogRead(rotPin);
midiCState = map(rotCState, 0, 4095, 20, 600);
potCstate = analogRead(potPin);
outputValue = map(potCstate, 0, 4095, 127, 0);

 if (outputValuePState > outputValue + 6 || 
      outputValuePState < outputValue - 6) {
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

   midiPacket[4] = outputValue;
   Serial.println(outputValue);

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   outputValuePState = outputValue;
 }

    for (int n = 0; n < buttonPushCounter; n++) {
      
buttonCstate[i] = digitalRead(Buttonselect[i]);
rotCState = analogRead(rotPin);
midiCState = map(rotCState, 0, 4095, 20, 600);
potCstate = analogRead(potPin);
outputValue = map(potCstate, 0, 4095, 127, 0);

 if (outputValuePState > outputValue + 6 || 
      outputValuePState < outputValue - 6) {
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

   midiPacket[4] = outputValue;
   Serial.println(outputValue);

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   outputValuePState = outputValue;
 }

if (R_Note[n] != 24) {
      midiPacket[2] = Channel_SelectON;
      Serial.println(Channel_SelectON);

      midiPacket[3] = R_Note[n];
      Serial.println(R_Note[n]);

      midiPacket[4] = 100;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   delay(60000 / midiCState);

   midiPacket[2] = Channel_SelectOFF;
   Serial.println(Channel_SelectOFF);

   midiPacket[3] = R_Note[n];
   Serial.println(R_Note[n]);

   midiPacket[4] = 0;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();
}

else {
  delay(60000 / midiCState);
}
   if (buttonCstate[0] == HIGH) {
    buttonPushCounter = 0;
   }
    }
    }
  }

  //Controls Returning to Main Menu (ran at the end of each mode)

  void MENURETURN() {
    if (digitalRead(Buttonselect[3]) && digitalRead(Buttonselect[8]) && digitalRead(Buttonselect[11]) && potCstate == 0) {
samplemode = 0;
Channel_SelectON = 0;
selectpause = 0;

midiPacket[2] = Channel_SelectOFF;
   Serial.println(Channel_SelectOFF);

   midiPacket[3] = (27);
   Serial.println(27);

   midiPacket[4] = 0;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   midiPacket[2] = Channel_SelectOFF;
   Serial.println(Channel_SelectOFF);

   midiPacket[3] = (32);
   Serial.println(32);

   midiPacket[4] = 0;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   midiPacket[2] = Channel_SelectOFF;
   Serial.println(Channel_SelectOFF);

   midiPacket[3] = (35);
   Serial.println(35);

   midiPacket[4] = 0;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();
}
  }


  
