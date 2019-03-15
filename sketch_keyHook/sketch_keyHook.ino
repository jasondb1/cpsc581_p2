
//pin setup
//output pins
#define LED_R1 30
#define LED_G1 31
#define LED_R2 32
#define LED_G2 33
#define LED_R3 34
#define LED_G3 35
#define BUZZER 22
#define LED_BOARD = LED_BUILTIN //This could be used for error signalling

const int LED[] = {LED_R1, LED_G1, LED_R2, LED_G2, LED_R3, LED_G3};
const int LED_RED[] = {LED_R1, LED_R2, LED_R3};
const int LED_GRN[] = {LED_G1, LED_G2, LED_G3};

//input pins
#define KEY_1 51
#define KEY_2 52
#define KEY_3 53

#define PIR_1 24
#define BTN_1 40



const int IN[] = {KEY_1, KEY_2, KEY_3, PIR_1, BTN_1};
const int KEY[] = {KEY_1, KEY_2, KEY_3};

enum state{ALARM_OFF, ALARM_ON, CLOSED, OPEN};

int sensorPIR = 0;
state doorState = CLOSED;
state systemState = ALARM_OFF;
int preKeyNum = 0;
int keyNum = 0;
boolean keyChangeFlg = false;

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
  /*
  boolean setAlarm = false;
  if(doorOpen() && currentKeyNum()!=3 && !keyChangeFlg) {
      setAlarm = true;
      int i = 0;
      boolean timerExpired = false;
      while(!timerExpired){
          if (keyChanged()) { //button has to be added too
              setAlarm = false;
          }
          delay(1000);
          if (i<=30) {
              i++;
          }
      }
  }
  if(setAlarm) {
      activateAlarm();
      for (int j = 0; j<60; j++) {
          delay(1000);
          //if button pressed or key changed
          //j+60;
      }
      turnOffAlarm();
  }
  */
  
  // put your main code here, to run repeatedly:

  unsigned long currAlarmDuration;
  unsigned long currAlarmWait;
  
  if(ALARM_OFF == systemState) {
    if(doorOpen()) {
      if(!keyChangeFlg || currentKeyNum()!=3) {
        systemState = ALARM_ON;
      } else {
        systemState = ALARM_OFF;
      }
    } else {
      doorState = CLOSED;
    }
  } else if(ALARM_ON == systemState) {
    if(keyChangeFlg) {
      turnOffAlarm();
      delay(1000); //deley 1 sec to reset the system
      
      // reset system
      preKeyNum = keyNum;
      keyChangeFlg = false;
      systemState = ALARM_OFF;
      doorState = CLOSED;
    } else {
      if(!checkKeyNum()) {
//        currAlarmWait = millis();
//        if (currAlarmWait - preAlarmWait < setWait) {
//          preAlarmWait = currAlarmWait;
//        } else {
//          activateAlarm();
//        }
        
        for (int j = 0; j<10; j++) { // delay before alarm is set 
          delay(1000);
          if(checkKeyNum()) { // if key placed on hook before delay over; short beep occurs needs fix
            j += 10;
          }
        }
        activateAlarm();
//        currAlarmDuration = millis();
//        if (currAlarmDuration - preAlarmDuration != setDuration) {
//          preAlarmDuration = currAlarmDuration;
//          if (preAlarmDuration >= setDuration) {
//            turnOffAlarm();
//            systemState = ALARM_OFF;
//          }
//        }

        for (int j = 0; j<10; j++) { // delay before alarm is off 
          delay(1000);
          if(checkKeyNum()) { // if key placed on hook before delay over;
            j += 10;
          }
        }
        turnOffAlarm();
        systemState = ALARM_OFF;

      } else {
        turnOffAlarm();
        systemState = ALARM_OFF;
        doorState = CLOSED;
      }
    }
  }
  //keyChangeFlg = keyChanged(); 
  keyChangeFlg = checkKeyNum();
  delay(50);
  selfTest();
}


///////////////////////////////////
// Detect door activity
//
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
  Serial.println("Current Key Count = " + String(cnt));
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
    Serial.println("[INPUT " + String(i) + " PIN" + String(IN[i]) +"]" + digitalRead(IN[i]));
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
