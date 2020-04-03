#define DATA_ITERATION 2
#define END_OF_TRASMISSION_CODE 154
#define REAL_TIME_DATA_SIZE 23
#define DHT_DATA_SIZE 6
#define ERROR_CODE 201
#define STRING_TRASMISSION_CODE 202
#define TIMEOUT 10

#define SHT_ID 170
#define DHT_ID 171
#define LIGHT_ID 172
#define ANALOG_HUM_ID 173
#define ANALOG_RAIN_ID 174
#define RAIN_ID 175
#define BMP_ID 176
#define SHT_DATA_SIZE 8
byte reaTimeData[REAL_TIME_DATA_SIZE*DATA_ITERATION];
byte data[10];
byte floatBuff[5*DATA_ITERATION];
byte intBuff[3*DATA_ITERATION];

//byte *tmpPointer;
byte j;    
byte k;
byte index = 0;
unsigned long timeOutCtn;
unsigned long backupSendCtn;
byte rf_id;

void receive(){
  while(Serial.available()){
    rf_id = Serial.read();
    if(checkBackupSend()){return;}
    getID();
    if(rf_id == 0){
      Serial.flush(); 
      continue;
    }
    switch((int)rf_id){
      case STRING_TRASMISSION_CODE: readText(); break;
      default: Serial.flush();
    }
  }
}
void readText(){
  if(!checkSerial()) return;
  String result = Serial.readStringUntil('\n');
  if(result.equals("Programming mode")){
    sendText("Programming mode");
    _detectReset = true;   
  }
}

void getID(){
  boolean flag = false;
    for(j =0; j < DATA_ITERATION/2; j++){
        if(j!=0){
        rf_id = Serial.read();
        }
        if(!checkSerial()) flag = true;
        if(rf_id != Serial.read()) flag = true;
    }
    if(flag) rf_id = 0;
}
boolean checkBackupSend(){
   
  switch((int)rf_id){
      case SHT_ID: 
        if(checkBackupSendInterval()){
          sendSht(); 
        }
       return true;
      case ANALOG_HUM_ID: sendAnalogHum();
        if(checkBackupSendInterval()){
          sendAnalogHum(); 
        }
       return true;  
      case ANALOG_RAIN_ID: 
        if(checkBackupSendInterval()){
          sendAnalogRain(); 
        }
       return true; 
      case LIGHT_ID:
        if(checkBackupSendInterval()){
          sendLight(); 
        }
       return true; 
      case DHT_ID:
        if(checkBackupSendInterval()){
          sendDht(); 
        }
       return true; 
  }
  return false;
}
boolean checkBackupSendInterval(){
  if(millis() > backupSendCtn +REAL_TIME_SAMPLING_INTERVAL/5){
    backupSendCtn = millis();
    return true;
  }
   return false;
}
void sendSht(){
  index = 0;
  writeID(SHT_ID,index); 
  write(shtTempCpy,index);
  write(shtHumCpy,index);
  Serial.write(data,index+1);
}
void sendAnalogHum(){
  index = 0;
  writeID(ANALOG_HUM_ID,index);
  write(analogHumCpy,index);
  Serial.write(data,index+1);
}
void sendAnalogRain(){
  index = 0;
  writeID(ANALOG_RAIN_ID,index);
  write(analogRainCpy,index);
  Serial.write(data,index+1);
}
void sendLight(){
  index = 0;
  writeID(LIGHT_ID,index);
  write(lightCpy,index);
  Serial.write(data,index+1);
}
boolean checkSerial(){
    if(!Serial.available()){
      timeOutCtn = millis();
      while(millis() < timeOutCtn + TIMEOUT){
        if(Serial.available()){
          return true;
        }
      }
      return false;
    }
    return true;
}

//1+4+4+1+2+1+2+1+2+1+4
void writeRealTimeDataToBuff(){
  index = 0;
  writeIDR(SHT_ID,index); 
  writeR(shtTemp,index);
  writeR(shtHum,index);
  
  writeIDR(ANALOG_HUM_ID,index);
  writeR(analogHum,index);

  writeIDR(ANALOG_RAIN_ID,index);
  writeR(analogRain,index);
  
  writeIDR(LIGHT_ID,index);
  writeR(light,index);

  writeIDR(DHT_ID,index);
  writeR(dhtHum,index);
  // writeEnd(index);
}
void sendRealTimeData(){
  Serial.write(reaTimeData,REAL_TIME_DATA_SIZE*DATA_ITERATION);
}
void sendDht(){
  index = 0;
  writeID(DHT_ID,index);
  write(dhtHumCpy,index);
  Serial.write(data,DHT_DATA_SIZE*DATA_ITERATION);
}
void sendError(byte error){
  index = 0;
  writeID(ERROR_CODE,index);
  data[index] = error;
  Serial.write(data,DATA_ITERATION +1);
}
void sendText(String error){
  index = 0;
  writeID(STRING_TRASMISSION_CODE,index);
  Serial.write(data,DATA_ITERATION);
  Serial.println(error);
}
void sendRain(){
  index = 0;
  writeID(RAIN_ID,index);
  Serial.write(data,DATA_ITERATION);
}
void writeEnd(byte index){
  data[index] =END_OF_TRASMISSION_CODE;
}
void writeIDR(byte id,byte &index){
  for(j =0; j < DATA_ITERATION; j++){
    reaTimeData[j+index] = id;
  }
  index+=DATA_ITERATION;
}
void writeID(byte id,byte &index){
  for(j =0; j < DATA_ITERATION; j++){
    data[j+index] = id;
  }
  index+=DATA_ITERATION;
}
void write(float val,byte &index){
 for(k =0; k < DATA_ITERATION; k++){
 byte *tmpPointer = (byte *)&val;
  for(j = index; j < index +4; j++){
    data[j] = tmpPointer[j-index];
  }
  index+=4;
 }
}
void write(int val,byte &index){
 for(k =0; k < DATA_ITERATION; k++){
  data[index] = lowByte(val);
  index++;
  data[index] = highByte(val);
  index++;
 }
}
void writeR(float val,byte &index){
 for(k =0; k < DATA_ITERATION; k++){
 byte *tmpPointer = (byte *)&val;
  for(j = index; j < index +4; j++){
    reaTimeData[j] = tmpPointer[j-index];
  }
  index+=4;
 }
}
void writeR(int val,byte &index){
 for(k =0; k < DATA_ITERATION; k++){
  reaTimeData[index] = lowByte(val);
  index++;
  reaTimeData[index] = highByte(val);
  index++;
 }
}
