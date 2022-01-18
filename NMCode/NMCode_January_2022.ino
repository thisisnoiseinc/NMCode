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
int R_Note[23];
int R_Velocity = 100;
int buttonPushCounter = 0;
int selectpause = 0;
int checkPot = 0;
const int numReadings = 15;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average1 = 0; // average current state
int lastaverage1 = 0; // average previous state
int average2 = 0; // average current state
int lastaverage2 = 0; // average previous state
int Menu_enter = 0; // Clears Loop Mode array & Causes 1 second delay when switching modes.
int Delay_DefaultModeSelection = 1; // delays/debounces selection of channel for default mode 
int block_mode_switch = 0;
int loop_enter_menu = 0;



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

  BLEDevice::init("NMSVE");

    

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

   if (Delay_DefaultModeSelection == 1) {
    delay(125);
    Delay_DefaultModeSelection = 0;
   }
  
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
  delay(125);
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

if (Menu_enter == 1) {
  digitalWrite(led_Green, HIGH);
  delay(500);
  Menu_enter = 0;
}

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

if (Menu_enter == 1) {
  digitalWrite(led_Green, HIGH);
  delay(500);
  Menu_enter = 0;
}

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
outputValue = map(potCstate, 0, 4095, 3, 9);
ButtonNote = (outputValue * 12 + i);

if (outputValue == 3 || outputValue == 5 || outputValue == 7 || outputValue == 9) {
  digitalWrite(led_Green, HIGH);
}

else {
  digitalWrite(led_Green, LOW);
}

if (Menu_enter == 1) {
  
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
  
  for (int p =0; p < 15; p++) {
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

void potaverage2() {

  for (int p =0; p < 15; p++) {
  potCstate = analogRead(potPin);
  outputValue = map(potCstate, 0, 4095, 0, 127);
  
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = outputValue;
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
  average2 = total / numReadings;
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

  // Controls Split Mode functions

  void SMODE() {
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

if (Menu_enter == 1) {
  
  digitalWrite(led_Green, LOW);

}

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

    block_mode_switch = 0;
    loop_enter_menu = 1;

    if (checkPot == 1) {
      
   midiPacket[2] = Channel_SelectCC;
   Serial.println(Channel_SelectCC);

   midiPacket[3] = 0x01;
   Serial.println(0x01);

   midiPacket[4] = 0;
   Serial.println(0);

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   checkPot = 0;
    }
   
    for (int i = 0; i < button; i++){
buttonCstate[i] = digitalRead(Buttonselect[i]);
potCstate = analogRead(potPin);
outputValue = map(potCstate, 0, 4095, 3, 9);
ButtonNote = (outputValue * 12 + i);
rotCState = analogRead(rotPin);
midiCState = map(rotCState, 0, 4095, 20, 600);

if (outputValue == 3 || outputValue == 5 || outputValue == 7 || outputValue == 9) {

  digitalWrite(led_Green, HIGH);
  
}

else {

  digitalWrite(led_Green, LOW);
  
}

while (Menu_enter == 1) {

  for (int n = 0; n < 22; n++){
    R_Note[n] = 0;
  }
  delay(500);
  buttonPushCounter = 0;
  Menu_enter = 0;
}

  if ((millis() - lastDebounceTime[i]) > debounceDelay) {
  
  if (buttonPState[i] != buttonCstate[i]) {
        lastDebounceTime[i] = millis();

  if (buttonCstate[i] == HIGH) {  

    if (ButtonNote == 119) {
   midiPacket[2] = Channel_SelectOFF;
   Serial.println(Channel_SelectOFF);

   midiPacket[3] = 119;

   midiPacket[4] = 0;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   break;
  }

   if (ButtonNote != 36) {
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
    R_Note[buttonPushCounter] = 36;
    if (buttonPushCounter < 22) {
      buttonPushCounter++;
    }
  }
  }
  else {
    if (ButtonNote != 36) {
   midiPacket[2] = Channel_SelectOFF;
   Serial.println(Channel_SelectOFF);

   midiPacket[3] = OffNote[i];

   midiPacket[4] = 0;

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();
    if (buttonPushCounter < 22) {
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
digitalWrite(led_Green, LOW);
rotCState = analogRead(rotPin);
midiCState = map(rotCState, 0, 4095, 20, 600);
block_mode_switch = 1;

 potaverage2();
 
 if (average2 != lastaverage2) {
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

   midiPacket[4] = average2;
   Serial.println(average2);

   pCharacteristic->setValue(midiPacket, 5);

   pCharacteristic->notify();

   lastaverage2 = average2;

   checkPot = 1;
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

if (R_Note[n] != 36) {

   if (R_Note[n] != 0) {
   
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
}

else {
  delay(60000 / midiCState);
}

if (buttonCstate[0] == HIGH) {
    buttonPushCounter = 0;
   }
   if (midiCState == 600) {
    break;
   }
    }
    }
  }

  //Controls Returning to Main Menu (ran at the end of each mode)

  void MENURETURN() {
    if (digitalRead(Buttonselect[11]) && potCstate == 4095 && block_mode_switch == 0) {
samplemode = 0;
Channel_SelectON = 0;
selectpause = 0;
Menu_enter = 1;
midiCState = 600;
Delay_DefaultModeSelection = 1;
digitalWrite(led_Green, LOW);
digitalWrite(led_Blue, LOW);

if (loop_enter_menu == 0) {
  
 for (int i = 0; i < button; i++){

while (buttonCstate[i] == HIGH) {

BUTTONS();
SMODE();

}
}
}
loop_enter_menu = 0;
}
}


  
