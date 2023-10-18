#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"
#define BLYNK_PRINT Serial

//define pin out
#define BUZZER 4
#define Gas_pin  36
#define CO_pin  34 
#define REDPin 21
#define GREENPin 18
#define BLUEPin 19
 
//define for PWM in ESP32
const int freq = 5000;
const int ledChannel0 = 0;
const int ledChannel1 = 1;
const int ledChannel2 = 2;
const int resolution = 12;

//cycle of led state
const int CYCLE = 800;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "PASS";

uint16_t Threshold = 200; //threshold for warning
uint64_t Time1; //time of counter 1 for led blink
bool isLedStatusOn = 0; 
const int Level1 = 80; //level for led status run
uint16_t Gas_vallue, CO_vallue;

BlynkTimer timer;
int timerID1,timerID2; 
bool buttonState = HIGH;
bool isRun = 1;
bool isWarning = 0;

//function for choose and map value to show RGB
void LedStatus();
void CalculateRGB(uint16_t);
void LedShow(uint16_t, uint16_t, uint16_t);
void GetVallue();
WidgetLED led(V0);
void handleTimerID1();
void handleTimerID2();
void Counter1();

void setup()
{
  Serial.begin(9600);
  psramInit();
  pinMode(REDPin, OUTPUT);
  pinMode(Gas_pin, INPUT);
  pinMode(CO_pin, INPUT);
  ledcSetup(ledChannel0, freq, resolution);
  ledcAttachPin(REDPin, ledChannel0);
  ledcSetup(ledChannel1, freq, resolution);
  ledcAttachPin(GREENPin, ledChannel1);
  ledcSetup(ledChannel2, freq, resolution);
  ledcAttachPin(BLUEPin, ledChannel2);
  timerID1 = timer.setInterval(1000L,handleTimerID1);
  Blynk.begin(auth, ssid, pass);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Time1 = millis();
}

void loop()
{
  Counter1();
  Blynk.run();
  timer.run();
  LedStatus();
}

void handleTimerID1(){
  GetVallue();
  Blynk.virtualWrite(V2, Gas_vallue);
  Blynk.virtualWrite(V2, CO_vallue);
  if(led.getValue()) {
    led.off();
  } else {
    led.on();
  }
  if(isRun == 1){
    if(Gas_vallue> Threshold){
      if(isWarning==0){
        isWarning=1;
        timerID2 = timer.setTimeout(60000L,handleTimerID2);
      }
      digitalWrite(BUZZER,HIGH);
      Blynk.virtualWrite(V1,HIGH);
      Serial.println("Warning on!");
      Blynk.logEvent("gas_alert", "GAS DETECTED");
    }else{
      digitalWrite(BUZZER,LOW);
      Blynk.virtualWrite(V1,LOW);
      Serial.println("Warning off!");
    }
    if(CO_vallue > Threshold){
      Blynk.logEvent("co_alert", "CO DETECTED");
    }
  }else{
    digitalWrite(BUZZER,LOW);
    Blynk.virtualWrite(V1,LOW);
    Serial.println("Warning on!");
  }
}
void handleTimerID2(){
  canhbaoState=0;
}
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V1,V4);
}
BLYNK_WRITE(V3) {
  Threshold = param.asInt();
}
BLYNK_WRITE(V4) {
  runMode = param.asInt();
}

void LedStatus(){
  if(Gas_vallue < Threshold && CO_vallue < Threshold){
    LedShow(0,4096,0);
  }
  if(Gas_vallue > Level1 || CO_vallue > Level1){
    uint16_t Vallue = (Gas_vallue>CO_vallue)?Gas_vallue:CO_vallue;
    uint16_t Val_Handle = map(4095-Vallue,4095,0, 5000, 0);
    CalculateRGB(Val_Handle);
  }
}

void LedShow(uint16_t R_vallue, uint16_t G_vallue, uint16_t B_vallue){
  if(isLedStatusOn){
    ledcWrite(ledChannel0, 4095 - R_vallue);
    ledcWrite(ledChannel1, 4095 - G_vallue);
    ledcWrite(ledChannel2, 4095 - B_vallue);
  }
  else{
    ledcWrite(ledChannel0, 4095);
    ledcWrite(ledChannel1, 4095);
    ledcWrite(ledChannel2, 4095);
  }
}

void CalculateRGB(uint16_t color){
	uint16_t redPWM;
	uint16_t greenPWM;
	uint16_t bluePWM;
	if (color <= 4080)     
	{
		redPWM = 4080 - color;  
		greenPWM = color;        
		bluePWM = 0;             
	}
	else if (color <= 8176)   
	{
		redPWM = 0;                     
		greenPWM = 4080 - (color - 4096); 
		bluePWM = (color - 4096);       
	}
	else
	{
		redPWM = (color - 8192);        
		greenPWM = 0;              
		bluePWM = 8176 - (color - 8192);
	}
	LedShow(redPWM, greenPWM, bluePWM);
}

void GetVallue(){
  CO_vallue = analogRead(CO_pin);
  Gas_vallue = analogRead(Gas_pin);
}
void Counter1(){
  if(millis() - Time1 > CYCLE){
    if(isLedStatusOn){
      isLedStatusOn = 0;
      Time1 = millis()+400;
    }
    else{
      isLedStatusOn = 1;
      Time1 = millis()-500;
    }
  }
}