
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

int sensorPIR = 0;
volatile state doorState = CLOSED;
volatile state systemState = ALARM_OFF;
volatile int preKeyNum = 0;
volatile int keyNum = 0;
volatile boolean keyChangeFlg = false;
volatile boolean keyTakeFlg = false;
volatile boolean openTrigger = false;
int keyTakeCounter = 0;
int openCounter = 0;
int alarmCounter = 0;
boolean alarmOn = false;

unsigned long preAlarmWait = 0;
unsigned long preAlarmDuration = 0;

unsigned long setWait = 2000;
unsigned long setDuration = 20000;

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

  attachInterrupt(digitalPinToInterrupt(18), turnOffAlarm, FALLING);
  attachInterrupt(digitalPinToInterrupt(21), motionFound, FALLING);

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
  
}

void loop() {
  setLEDStatus();
  Serial.println(String(keyAdded()));
  if (keyAdded()) {
    Serial.println("Key Added");
    openCounter = 0;
    alarmCounter = 20;
    keyTakeCounter = 0;
    openTrigger = false;
    keyTakeFlg = false;
  }
  if (!keyTakeFlg) {
    if (keyTaken()) {
      Serial.println("Key Taken");
      keyTakeFlg = true;
    }
  }
  if (keyTakeCounter>20) {
    Serial.println("Taken expired.");
    keyTakeFlg = false;
    keyTakeCounter = 0;
  } else {
    keyTakeCounter++;
  }
  if (openTrigger) {
    if (openCounter < 20) {
      openCounter++;
      Serial.println(String(openCounter));
    } else {
      activateAlarm();
      alarmOn = true;
      openTrigger = false;
      openCounter = 0;
    }
  }
  if (alarmOn == true) {
    if (alarmCounter < 20) {
      alarmCounter++;
    } else {
      turnOffAlarm();
      alarmCounter = 0;
      alarmOn = false;
    }
  }
  delay(500);
}
  
  // put your main code here, to run repeatedly:

//  unsigned long currAlarmDuration;
//  unsigned long currAlarmWait;
//  
//  if(ALARM_OFF == systemState) {
//    if(doorOpen()) {
//      if(!keyChangeFlg && currentKeyNum()!=3) {
//        systemState = ALARM_ON;
//      } else {
//        systemState = ALARM_OFF;
//      }
//    } else {
//      doorState = CLOSED;
//    }
//  } else if(ALARM_ON == systemState) {
//    if(keyChangeFlg) {
//      turnOffAlarm();
//      delay(500); //deley 1 sec to reset the system
//      
//      // reset system
//      preKeyNum = keyNum;
//      keyChangeFlg = false;
//      systemState = ALARM_OFF;
//      doorState = CLOSED;
//    } else {
//      if(!checkKeyNum()) {
//        
//        for (int j = 0; j<20; j++) { // delay before alarm is set 
//          delay(500);
//          if(checkKeyNum()) { // if key placed on hook before delay over; short beep occurs needs fix
//            j += 20;
//          } 
//        }
//        activateAlarm();
//        
//
//        for (int j = 0; j<20; j++) { // delay before alarm is off 
//          delay(500);
//          if(checkKeyNum()) { // if key placed on hook before delay over;
//            j += 10;
//          }
//        }
//        turnOffAlarm();
//        systemState = ALARM_OFF;
//
//      } else {
//        turnOffAlarm();
//        systemState = ALARM_OFF;
//        doorState = CLOSED;
//      }
//    }
//  } 
//  keyChangeFlg = checkKeyNum();
//  delay(50);
//  selfTest();


///////////////////////////////////
// Detect door activity
//

void motionFound() {
  if (!keyTaken) {
    openTrigger = true;
  }
}

boolean doorOpen() {
  sensorPIR = digitalRead(PIR_1);
  Serial.println("[PIR PIN" + String(IN[3]) +"]" + digitalRead(IN[3]));
  if(HIGH == sensorPIR) {  // motion detected
    if(CLOSED == doorState) {
      doorState = OPEN;
      return true;
    }
  }
  return false;
}

///////////////////////////////////
// Detect if the key number has changed
//
boolean keyChanged() {
  keyNum = currentKeyNum();
  if(keyNum == preKeyNum) {
    return false;
  }
  delay(50);  // eliminate noise
  if(currentKeyNum() == keyNum) {
    preKeyNum = keyNum;
    return true;
  }
  return false;
}

boolean keyAdded() {
  keyNum = currentKeyNum();
  delay(50);
  if (keyNum > preKeyNum) {
    preKeyNum = keyNum;
    return true;
  }
  return false;
}

boolean keyTaken() {
  keyNum = currentKeyNum();
  delay(50);
  if (keyNum < preKeyNum) {
    preKeyNum = keyNum;
    return true;
  } 
  return false;
}

///////////////////////////////////
// Detect if the key number has changed
//
boolean checkKeyNum() {
  keyNum = currentKeyNum();
  if (keyNum < preKeyNum) {
    return false;
  } 
  delay(50);
  if(keyNum >= preKeyNum && keyNum != 3) {
    preKeyNum = keyNum;
    return true;
  }
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
  buzzerOff();
  setLEDStatus();
}

void activateAlarm(){
  buzzerOn();
//  for (int i = 0; i < (sizeof(LED) / sizeof(LED[0])); i++) {
//    ledOn(LED[i]);
//    delay(50);
//    ledOff(LED[i]);
//    delay(50);
//  }
}

void selfTest() {
/*
  for (int i = 0; i < (sizeof(LED) / sizeof(LED[0])); i++) {
    ledOn(LED[i]);
    delay(50);
    ledOff(LED[i]);
    delay(50);
  }
  
    buzzerOn();
    delay(50);
    buzzerOff();
    delay(50);
*/

  for (int i = 0; i < (sizeof(IN) / sizeof(IN[0])); i++) {
  delay(50);
  }
}


///////////////////////////////////
//Turn an LED on
//
boolean keyStatus(int key) {
  Serial.println("[KEY "+ String(key)  +" STATUS]:" + digitalRead(KEY[key])) ;
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
