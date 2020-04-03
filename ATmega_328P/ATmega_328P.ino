#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "Adafruit_SHT31.h"
#include <amt1001_ino.h>
#include <dht.h>

#define DTR_PIN 2
#define HC_CTRL_PIN 9
#define RAIN_SENSOR_PIN 3
#define DHT22_PIN 8
#define SD_CHIP_SELECT_PIN 10
#define LIGHT_SENSOR_ANALOG_PIN A2
#define RAIN_SENSOR_ANALOG_PIN A1
#define HUM_SENSOR_ANALOG_PIN A3

#define RAIN_SENSOR_DEBOUNCE_DELAY 20
#define DHT_SAMPLING_INTERVAL 2000
#define REAL_TIME_SAMPLING_INTERVAL 50
#define REAL_TIME_SESNORS_SAMPLING_INTERVAL 30
dht DHT;
Adafruit_BMP085 bmp;
Adafruit_SHT31 sht31 = Adafruit_SHT31();


volatile byte rainState = LOW;
boolean _detectReset = false;
boolean sht31Enabled = true;
boolean bmpEnabled = true;
boolean dht22Enabled = true;
boolean sdReaderEnabled = true;

unsigned long dhtTimer= 0;
unsigned long realTimeTimer =0;
unsigned long realTimeSensorsTimer =0;

int light =0;
int analogRain = 0;
int analogHum = 0;
int rain = 0;

int lightCpy =0;
int analogRainCpy = 0;
int analogHumCpy = 0;


float shtTemp = 0;
float shtHum = 0;
float dhtTemp =0;
float dhtHum =0;
float bmpTemp =0;
float bmpPres = 0;


float shtTempCpy = 0;
float shtHumCpy= 0;
float dhtTempCpy =0;
float dhtHumCpy =0;
float bmpTempCpy =0;
float bmpPresCpy = 0;

const char filename[] = "demo.txt";
File txtFile;
String buffer;
unsigned long lastMillis = 0;

void setup() {
  pinMode(DTR_PIN,OUTPUT);
  digitalWrite(DTR_PIN,HIGH);
  delay(2000);
  Serial.begin(115200);
  secureBoot();
  Serial.begin(9600);


//  delay(700);
//  Serial.println("entering command mode");
 // pinMode(HC_CTRL_PIN,OUTPUT);
 //  digitalWrite(HC_CTRL_PIN,LOW);
//   delay(40);
//   Serial.flush();  


 // pinMode(HC_CTRL_PIN,OUTPUT);
  //digitalWrite(HC_CTRL_PIN,LOW);
  pinMode(RAIN_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RAIN_SENSOR_PIN), detectRain, RISING);
  

  sendText(F("reset"));
  
  if(DHT.read22(DHT22_PIN) != DHTLIB_OK){
    sendError((byte)DHT.read22(DHT22_PIN));
    dht22Enabled = false;
  }
  if (!sht31.begin(0x44)) {   
    sendText(F("SHT31 error"));
    sht31Enabled = false;
  }
  if (!bmp.begin()) {
    sendText(F("BMP085 error"));
    bmpEnabled = false;
  }
  if(!SD.begin(SD_CHIP_SELECT_PIN)){
    sendText(F("SD error"));
    sdReaderEnabled = false;
  }
  buffer.reserve(1024);
  txtFile = SD.open(filename, FILE_WRITE);
  if (!txtFile) {
    sendText("SD File error");
    sdReaderEnabled = false;
  }
  handleRealTimeSensors();
  writeRealTimeDataToBuff();
}
void secureBoot(){
  const int bootTime = millis();
  const int bootDelay = 5500;
  while(millis() <= bootTime + bootDelay){
      detectReset();
  }
}

void loop() {
  if(_detectReset){
   detectReset();
  }
  handleRainSensor();
  handleDHTsensor();
//  handleRealTimeSensors();
  handleRealTime();
 // receive();
}
void handleRealTimeSensors(){
//  if(millis() > realTimeSensorsTimer + REAL_TIME_SESNORS_SAMPLING_INTERVAL){
    if(sht31Enabled){
    shtTemp = sht31.readTemperature();
    shtHum = sht31.readHumidity();
  }else if(!sht31Enabled||isnan(shtTemp)){
    if(!bmpEnabled){
      bmpEnabled =  bmp.begin();
    }else{
      shtTemp = bmp.readTemperature();
    }
  }
  analogHum = analogRead(HUM_SENSOR_ANALOG_PIN);
  analogRain = analogRead(RAIN_SENSOR_ANALOG_PIN);
  light = analogRead(LIGHT_SENSOR_ANALOG_PIN);

 //   realTimeSensorsTimer =millis();
 // }
}

void handleDHTsensor(){
  if(millis() > dhtTimer + DHT_SAMPLING_INTERVAL&&dht22Enabled){
    int resp = DHT.read22(DHT22_PIN);
    if(resp == DHTLIB_OK){
    //dhtTemp = DHT.temperature;
    dhtHum = DHT.humidity;
    }else{
     // sendError((byte)resp);
    }
    dhtTimer = millis();
  }
}
void handleRainSensor(){
  if(rainState){
    sendRain();
    sendRain();
    rainState = false;
    delay(RAIN_SENSOR_DEBOUNCE_DELAY);
  }
}
void detectRain() {
   rainState = true;
}
void detectReset(){
    if(Serial.available()){
    digitalWrite(DTR_PIN,LOW);
  }
}
void handleRealTime(){
 if(millis() > realTimeTimer +REAL_TIME_SAMPLING_INTERVAL){
  shtTempCpy = shtTemp;
  shtHumCpy= shtHum;
  dhtHumCpy =dhtHum;
  lightCpy =light;
  analogRainCpy = analogRain;
  analogHumCpy = analogHum;

  realTimeTimer = millis();
  
  sendRealTimeData();
  sendRealTimeData();
  
  handleRealTimeSensors();
  writeRealTimeDataToBuff();
 }
}


void testSesnors(){
  Serial.print("Light ");
  Serial.println(analogRead(A2));
  Serial.print("Rain Analog");
  Serial.println(analogRead(A1));
  Serial.print("Hum Analog ");
  uint16_t step = analogRead(A3);
  double volt = (double)step * (5.0 / 1023.0);
  uint16_t humidity = amt1001_gethumidity(volt);
  Serial.println(humidity);
  Serial.print("rain state ");
  Serial.println(rainState);
  Serial.print("sht temp ");
  Serial.println(sht31.readTemperature());
  Serial.print("sht hum" );
  Serial.println(sht31.readHumidity());
  if(DHT.read22(8) == DHTLIB_OK){
  Serial.print("dht temp ");
  Serial.println(DHT.temperature);
  Serial.print("dht hum ");
  Serial.println(DHT.humidity);
  }else{
    Serial.println( "dht error");
  }
  Serial.print("bmp pres ");
  Serial.println(bmp.readPressure()/100.0f);
  Serial.print("bmp temp ");
  Serial.println(bmp.readTemperature());
  Serial.println("====");
}

//void setHcSettings(){
//  while(Serial.available()){
//    byte b = Serial.read();
//    if(b == 'w'){
//        Serial.println("W_");
//        hcDisableCommandMode();
//       break;
//    }else if(b == 'q'){
//      Serial.println("Q_");
//      hcEnableCommandMode();
//          break;
//    }
//    hcSerial.write(b);
//    yield();
//  }
//
//}
void hcEnableCommandMode(){
   pinMode(HC_CTRL_PIN,OUTPUT);
   digitalWrite(HC_CTRL_PIN,LOW);
   delay(40);
   Serial.flush();  
}

void hcDisableCommandMode(){
  pinMode(HC_CTRL_PIN,INPUT);
  digitalWrite(HC_CTRL_PIN,LOW);
}

//dht22 czas pomiaru
//max 5392 ~ 5,4ms
//min 1500 ~ 1,5 ms
//avg 5127.09 ~5,1 ms

//nigdy nie pozwol zeby caly czas bylo serial println itd bez zadnego delay w loopie zablokojue to mozliwosc wgrywania kodu poniewaz zaspamuje caly serial
