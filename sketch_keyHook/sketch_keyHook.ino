// Key
//
//
//
//
//
//
//
//




//pin setup
//output pins
#define LED_R1 30
#define LED_G1 31
#define LED_R2 34
#define LED_G2 35
#define LED_R3 38
#define LED_G3 39
#define BUZZER 22
#define LED_BOARD = LED_BUILTIN //This could be used for error signalling

const int LED[] = {LED_R1, LED_G1, LED_R2, LED_G2, LED_R3, LED_G3};

//input pins
#define KEY_1 46
#define KEY_2 47
#define KEY_3 48
#define SW_1 50
#define SW_2 51
#define SW_3 52

#define PIR_1 42
#define PIR_2 43

const int IN[] = {KEY_1, KEY_2, KEY_3, SW_1, SW_2, SW_3, PIR_1, PIR_2};
const int KEY[] = {KEY_1, KEY_2, KEY_3};

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
  pinMode(SW_1, INPUT);
  pinMode(SW_2, INPUT);
  pinMode(SW_3, INPUT);
  pinMode(PIR_1, INPUT);
  pinMode(PIR_2, INPUT);

  //Runs a dignostic test to demonstrate leds / and buzzer working
  selfTest();
  

}




void loop() {
  // put your main code here, to run repeatedly:

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
