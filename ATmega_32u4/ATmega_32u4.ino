#include <SoftwareSerial.h>
#define HC_CTRL_PIN 7
#define HC_RX_PIN 9
#define HC_TX_PIN 8
//#define HC_RX_PIN 10
//#define HC_TX_PIN 11
#include <dht.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;
dht DHT;
#define RESET_PIN 10
SoftwareSerial hcSerial(HC_RX_PIN, HC_TX_PIN);
//komendy at w hc12 dzialaja niezaleznie czy jest na koncu znak nowej lini itp
//domyslny bound rate w at mode to 9600
#include "Adafruit_SHT31.h"
Adafruit_SHT31 sht31 = Adafruit_SHT31();
// dht pin 6
//dht mietowy-> nieb data dht
// fht czerw ->pom 
//dht ->bial -> bial 
boolean uploadMode = false;
boolean uartMode = false;

void setup() {
  // put your setup code here, to run once:

 //pinMode(HC_CTRL_PIN,OUTPUT);
 //digitalWrite(HC_CTRL_PIN,HIGH);

 // Serial.begin(9600);
 // hcSerial.begin(9600);

//    if (!bmp.begin()) {
//  Serial.println("Could not find a valid BMP085 sensor, check wiring!");
//  while (1) {}
//  }
}
boolean rf_programming_mode = false;
void loop() {
  if(!rf_programming_mode){
  if (UDADDR & _BV(ADDEN)){
    if(!uartMode){
      uartInit();
      uartMode = true;
    }
    handleUartMode();
  }else{
    if(uartMode){
      setup();
      uartMode = false;
    }
  }
  }
}

void hcSetProgrammingBaud(){
  Serial.begin(9600);
  pinMode(HC_CTRL_PIN,OUTPUT);
 digitalWrite(HC_CTRL_PIN,HIGH);

  Serial.begin(9600);
  delay(40);
  //enable programming mode
   digitalWrite(HC_CTRL_PIN,LOW);
   delay(40);
   for(int k=0; k < 10; k++){
    Serial.print(F("AT+B115200"));
    delay(40);
   }
   Serial.flush();
}
void setHcSettings(){
   while(hcSerial.available()){
    Serial.write(hcSerial.read());
  }
  while(Serial.available()){
    byte b = Serial.read();
    if(b == 'w'){
        Serial.println("W_");
       digitalWrite(HC_CTRL_PIN,HIGH);
       break;
    }else if(b == 'q'){
      Serial.println("Q_");
     // pinMode(HC_CTRL_PIN,OUTPUT);
            digitalWrite(HC_CTRL_PIN,LOW);
          //       pinMode(HC_CTRL_PIN,OUTPUT);
          //  digitalWrite(HC_CTRL_PIN,LOW);
            
            delay(40);
          break;
    }
    hcSerial.write(b);
  }
}
void hcReset(){
  hcSerial.print(F("AT+FU3"));
  delay(40);
  hcSerial.print(F("AT+B38400"));
  delay(40);
  hcSerial.print(F("AT+P8"));
}

void hcFactoryReset(){
  hcSerial.print(F("AT+DEFAULT"));
  delay(40);
}
void hcEnableProgrammingMode(){
  hcFactoryReset();
  hcSerial.print(F("AT+B115200"));
  Serial.begin(115200);
  hcSerial.begin(115200);
  uploadMode =true;
}

//AT++B57600
