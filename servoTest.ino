#include <Servo.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "";

char ssid[] = "";
char pass[] = "!";

Servo myservo;  

const int buttonPin = 14;
const int pirPin = 5;
const int servoPin = 13;
const int lightSensorPin = 0;  

bool lightPrevLow = true;
int lightLevel = 0;
int lightLevelAverage = 0;

int buttonState = 1; 
int prev = 0;

int pirPrevState = !digitalRead(pirPin);
int pirCurrState = digitalRead(pirPin);

bool motionOn = false;
bool lightSensingOn = false;

void setup() {
  myservo.attach(servoPin);  
  pinMode(buttonPin, INPUT);
  pinMode(pirPin, INPUT);
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  lightLevelAverage = calcLightAverage();
  switchOn(true);
}

int blynkRead = 0;
BLYNK_WRITE(V1)
{
  blynkRead = param.asInt();
  if(blynkRead == 2 && prev == 1) {
    prev = 0;
    switchOff(false);
  }
  if(blynkRead == 1 && prev == 0) {
    prev = 1; 
    switchOn(false);
  }
}

BLYNK_WRITE(V0) {
  blynkRead = param.asInt();
  if(blynkRead == 0)
    motionOn = false;
  else if(blynkRead == 1)
    motionOn = true;
}

BLYNK_WRITE(V2) {
  blynkRead = param.asInt();
  if(blynkRead == 0)
    lightSensingOn = false;
  else if(blynkRead == 1)
    lightSensingOn = true;
}

unsigned int prevTime = 0;
void loop() {
  Blynk.run();
  buttonState = digitalRead(buttonPin);
  lightLevel = analogRead(lightSensorPin);
  pirCurrState = digitalRead(pirPin);

  if(buttonState == HIGH && ((millis() - prevTime) > 1000)) {
    prevTime = millis();
    if(prev == 0) {
      prev = 1;
      switchOn(true);
    }
    else if(prev == 1) {
      prev = 0;
      switchOff(true);
    }
  } 
  
  if(motionOn && pirCurrState == HIGH && pirPrevState == LOW) {
    pirPrevState = HIGH;
    Serial.println("***LOW to HIGH***");
    switchOn(true);
  }
  else if(motionOn && pirCurrState == LOW && pirPrevState == HIGH){
    pirPrevState = LOW;
    Serial.println("***HIGH to LOW***");
    switchOff(true);
  }
  
  if(lightSensingOn && lightPrevLow && (lightLevel <= lightLevelAverage)) {
    Serial.println("###LOW LIGHT###");
    lightPrevLow = false;
    switchOn(true);
  }
  else if(lightSensingOn && !lightPrevLow && (lightLevel > lightLevelAverage)) {
    Serial.println("###HIGH LIGHT###");
    lightPrevLow = true;
    switchOff(true);
  }
  delay(15);                           
}

void switchOn(bool switchRemote) {
  Serial.println("ON");
  if(switchRemote)
    Blynk.virtualWrite(V1, 1);
  for(int i = 0; i <= 180; i++) {
    myservo.write(i); 
  }
}

void switchOff(bool switchRemote) {
  Serial.println("OFF");
  if(switchRemote) 
    Blynk.virtualWrite(V1, 2);
  for(int i = 180; i > 0; i--) {
    myservo.write(i); 
  }
}

int calcLightAverage() {
  int sum = 0;
  int sampleSize = 50;
  for(int i = 0; i < sampleSize; i++) {
    sum += analogRead(lightSensorPin);
    delay(1);
  }
  int average = sum/sampleSize;
  return(average - (.1 * average));
}
