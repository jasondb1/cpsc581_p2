
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
  while(keyCheck < 3){  // Eliminate noise
    keyNum = currentKeyNum();
    if(keyNum == preKeyNum)keyCheck ++;
    else{
      preKeyNum = keyNum;
      keyCheck = 0;
    }
    delay(50);
  }
}

void loop() {
  //keyChangeFlg = keyChanged(); 
  //if (
  
  // put your main code here, to run repeatedly:
  if(ALARM_OFF == systemState){
    if(doorOpen()){
      if(!keyChangeFlg){
        systemState = ALARM_ON;
      }else{
        
      }
    }else{
      doorState = CLOSED;
    }
  }else if(ALARM_ON == systemState){
    if(keyChangeFlg){
      turnOffAlarm();
      delay(5000); //deley 5 sec to reset the system
      // reset system
      preKeyNum = keyNum;
      keyChangeFlg = false;
      systemState = ALARM_OFF;
      doorState = CLOSED;
    }else{
      activateAlarm();
    }
  }
  keyChangeFlg = keyChanged(); 
  delay(50);
}


///////////////////////////////////
// Detect door activity
//
boolean doorOpen() {
  sensorPIR = digitalRead(PIR_1);
  if(HIGH == sensorPIR){  // motion detected
    if(CLOSED == doorState){
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
  if(keyNum == preKeyNum)return false;
  delay(50);  // eliminate noise
  if(currentKeyNum() == keyNum){
    preKeyNum = keyNum;
    return true;
  }
  return false;
}

///////////////////////////////////
// Check current number of keys
//
int currentKeyNum() {
  int cnt=0; 
  for(int i=0; i<(sizeof(KEY) / sizeof(KEY[0])); i++){
    if(!digitalRead(KEY[i]))cnt ++;
  }
  return cnt;
}

void turnOffAlarm(){
  buzzerOff();
  for (int i = 0; i < (sizeof(LED) / sizeof(LED[0])); i++) {
    ledOff(LED[i]);
  }
}

void activateAlarm(){
  buzzerOn();
  for (int i = 0; i < (sizeof(LED) / sizeof(LED[0])); i++) {
    ledOn(LED[i]);
    delay(50);
    ledOff(LED[i]);
    delay(50);
  }
}

void selfTest() {

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
