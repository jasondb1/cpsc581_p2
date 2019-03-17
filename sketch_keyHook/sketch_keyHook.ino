#include "pitches.h"
//pin setup
//output pins
#define LED_R1 30
#define LED_G1 31
#define LED_R2 32
#define LED_G2 33
#define LED_R3 34
#define LED_G3 35
#define BUZZER 6
#define LED_BOARD = LED_BUILTIN //This could be used for error signalling

const int LED[] = {LED_R1, LED_G1, LED_R2, LED_G2, LED_R3, LED_G3};
const int LED_RED[] = {LED_R1, LED_R2, LED_R3};
const int LED_GRN[] = {LED_G1, LED_G2, LED_G3};

//input pins
#define KEY_1 51
#define KEY_2 52
#define KEY_3 53

#define PIR_1 21
#define BTN_1 18

const int IN[] = {KEY_1, KEY_2, KEY_3, PIR_1, BTN_1};
const int KEY[] = {KEY_1, KEY_2, KEY_3};

enum state{ALARM_OFF, ALARM_ON, CLOSED, OPEN};

volatile state systemState = ALARM_OFF;
volatile int preKeyNum = 0;
volatile int keyNum = 0;

int openTrigger = 4;

long lastKeyCheckedTime = 0;    // time when we check the number of keys on the hook
long KEY_CHECK_FREQ = 500;
int keyTaken = 0;



// MELODY and TIMING  =======================================
//  melody[] is an array of notes, accompanied by beats[], 
//  which sets each note's relative length (higher #, longer note) 
// Super Mario from: http://www.linuxcircle.com/2013/03/31/playing-mario-bros-tune-with-arduino-and-piezo-buzzer/
int melody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7, 
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0,
  NOTE_G6, 0, 0, 0, 

  NOTE_C7, 0, 0, NOTE_G6, 
  0, 0, NOTE_E6, 0, 
  0, NOTE_A6, 0, NOTE_B6, 
  0, NOTE_AS6, NOTE_A6, 0, 

  NOTE_G6, NOTE_E7, NOTE_G7, 
  NOTE_A7, 0, NOTE_F7, NOTE_G7, 
  0, NOTE_E7, 0,NOTE_C7, 
  NOTE_D7, NOTE_B6, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6, 
  0, 0, NOTE_E6, 0, 
  0, NOTE_A6, 0, NOTE_B6, 
  0, NOTE_AS6, NOTE_A6, 0, 

  NOTE_G6, NOTE_E7, NOTE_G7, 
  NOTE_A7, 0, NOTE_F7, NOTE_G7, 
  0, NOTE_E7, 0,NOTE_C7, 
  NOTE_D7, NOTE_B6, 0, 0
};
int beats[]  = {
  12, 12, 12, 12, 
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12, 

  12, 12, 12, 12,
  12, 12, 12, 12, 
  12, 12, 12, 12, 
  12, 12, 12, 12, 

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
};

int MAX_COUNT = sizeof(melody) / sizeof(melody[0]); // Melody length, for looping.
int thisNote = 0;

void playTone(int targetPin, long frequency, long length) {
  long delayValue = 1000000/frequency/2; // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * length/ 1000; // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to 
  //// get the total number of cycles to produce
  for (long i=0; i < numCycles; i++){ // for the calculated length of time...
    digitalWrite(targetPin,HIGH); // write the buzzer pin high to push out the diaphram
    delayMicroseconds(delayValue); // wait for the calculated delay value
    digitalWrite(targetPin,LOW); // write the buzzer pin low to pull back the diaphram
    delayMicroseconds(delayValue); // wait again or the calculated delay value
  }
}

////////////////////////////////////////////
// setup
//
//
void setup() {

  Serial.begin(9600);

  //Setup outputs
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_R1, OUTPUT);
  pinMode(LED_G1, OUTPUT);
  pinMode(LED_R2, OUTPUT);
  pinMode(LED_G2, OUTPUT);
  pinMode(LED_R3, OUTPUT);
  pinMode(LED_G3, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  //setup inputs
  pinMode(KEY_1, INPUT);
  pinMode(KEY_2, INPUT);
  pinMode(KEY_3, INPUT);
  pinMode(PIR_1, INPUT);
  pinMode(BTN_1, INPUT);

  digitalWrite(KEY_1, HIGH);
  digitalWrite(KEY_2, HIGH);
  digitalWrite(KEY_3, HIGH);

  digitalWrite(BTN_1, HIGH);
  digitalWrite(PIR_1, HIGH);

  int keyCheck = 0;
  preKeyNum = currentKeyNum();
  while(keyCheck < 3) {  // Eliminate noise
    keyNum = currentKeyNum();
    if(keyNum == preKeyNum) {
      keyCheck ++;
    } else {
      preKeyNum = keyNum;
      keyCheck = 0;
    }
    delay(50);
  }

  flashLED_reverse();
  openTrigger = 0;
  keyTaken = 0;

  attachInterrupt(digitalPinToInterrupt(18), turnOffAlarm, FALLING);
  attachInterrupt(digitalPinToInterrupt(21), motionFound, FALLING);
}


long triggerTime = 0;
long CHECK_KEY_TIME = 3000;
long alarmOnTime = 0;
long ALARM_EXPIRE_TIME = 12000;
boolean resetFlg = false;

void systemReset() {
  int keyCheck = 0;
  preKeyNum = currentKeyNum();
  while(keyCheck < 3) {  // Eliminate noise
    keyNum = currentKeyNum();
    if(keyNum == preKeyNum) {
      keyCheck ++;
    } else {
      preKeyNum = keyNum;
      keyCheck = 0;
    }
    delay(50);
  }

  buzzerOff();
  thisNote = 0;
  systemState = ALARM_OFF;
  keyTaken = 0;
  openTrigger = 4;
  //resetFlg = true;
}

void flashLED_reverse(){
  for(int i=0; i<6; i++){
    ledOff(LED[i]);
  } 
  for(int i=5; i>=0; i--){
    ledOn(LED[i]);
    delay(100);
    ledOff(LED[i]);
  } 
}

void flashLED(){
  for(int i=0; i<6; i++){
    ledOff(LED[i]);
  } 
  for(int i=0; i<6; i++){
    ledOn(LED[i]);
    delay(100);
    ledOff(LED[i]);
  } 
}

void loop(){
  setLEDStatus();
  keyNum = currentKeyNum();
  
  if(ALARM_OFF == systemState){
    if(1 == openTrigger){  // Door open
      triggerTime = millis();
      openTrigger = 2;
    }else if(2 == openTrigger){
      if(millis() - triggerTime < CHECK_KEY_TIME){
        if(keyNum>preKeyNum || keyTaken == 1){
          keyTaken = 1;
          openTrigger = 4;
        }
      }else {
          openTrigger = 3;  // trigger the alarm
      }
    }else if(3 == openTrigger){ 
      systemState = ALARM_ON;
      alarmOnTime = millis();
    }else if(4 == openTrigger){
       // Flash the LED to notify
       flashLED();
       setLEDStatus();
       delay(5000);   // duration until next motion detection
       flashLED_reverse();
       setLEDStatus();
       thisNote = 0;
       openTrigger = 0;
       keyTaken = 0;
    }
  }else{
    if(millis() - alarmOnTime > ALARM_EXPIRE_TIME || keyNum>preKeyNum || keyTaken == 1){
      keyTaken = 0;
      systemReset();
    }else {
        if(thisNote >= MAX_COUNT) thisNote = 0;
      // ---------------- play tone ---------------- //
         // to calculate the note duration, take one second
         // divided by the note type.
         //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
         int noteDuration = 1000/beats[thisNote];
  
         playTone(BUZZER, melody[thisNote],noteDuration);
  
         // to distinguish the notes, set a minimum time between them.
         // the note's duration + 30% seems to work well:
         int pauseBetweenNotes = noteDuration * 1.30;
         delay(pauseBetweenNotes);
  
         // stop the tone playing:
         playTone(BUZZER, 0,noteDuration);
      // ---------------- play tone ---------------- //
      thisNote++;
    }
  }

  if(millis() - lastKeyCheckedTime > KEY_CHECK_FREQ){ // update every KEY_CHECK_FREQ ms
    lastKeyCheckedTime = millis();
    if(preKeyNum > keyNum && keyTaken==0 && openTrigger!=4){
      keyTaken = 1;
    }

    preKeyNum = keyNum;
  }
  
}


///////////////////////////////////
// Detect door activity
//

void motionFound() {
    if(0 == openTrigger && ALARM_OFF == systemState)openTrigger = 1;
}


///////////////////////////////////
// Check current number of keys
//
int currentKeyNum() {
  int cnt=0; 
  for(int i=0; i<(sizeof(KEY) / sizeof(KEY[0])); i++) {
    if(!digitalRead(KEY[i])) {
      cnt ++;
    }
  }
  //Serial.println("Current Key Count = " + String(cnt));
  return cnt;
}

void turnOffAlarm(){
  digitalWrite(BUZZER, LOW);
  systemState = ALARM_OFF;
  preKeyNum = keyNum;
  buzzerOff();
  thisNote = 0;
  openTrigger = 4;
}



///////////////////////////////////
//Turn an LED on
//
boolean keyStatus(int key) {
//  Serial.println("[KEY "+ String(key)  +" STATUS]:" + digitalRead(KEY[key])) ;
  return digitalRead(digitalRead(KEY[key]));
}

///////////////////////////////////
//Turn an LED on based on key status
//
void setLEDStatus() {
  for (int i = 0; i < (sizeof(KEY) / sizeof(LED[0])); i++) {
    if(digitalRead(KEY[i])) {
      ledOn(LED_GRN[i]);
      ledOff(LED_RED[i]);
    } else {
      ledOn(LED_RED[i]);
      ledOff(LED_GRN[i]);
    }
  }
}

///////////////////////////////////
//Turn an LED on
//
void ledOn(int pin) {
  Serial.println("[LED ON]:" + pin);
  digitalWrite(pin, HIGH);
}


///////////////////////////////////
//Turn an LED off
//
void ledOff(int pin) {
  Serial.println("[LED OFF]:" + pin);
  digitalWrite(pin, LOW);
}

///////////////////////////////////
//Turn Buzzer On
//
void buzzerOn() {
  Serial.println("[BUZZER ON]");
  digitalWrite(BUZZER, HIGH);
}

///////////////////////////////////
//Turn Buzzer On
//
void buzzerOff() {
  Serial.println("[BUZZER OFF]");
  digitalWrite(BUZZER, LOW);
}
