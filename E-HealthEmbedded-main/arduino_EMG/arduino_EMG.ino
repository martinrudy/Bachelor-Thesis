//include libraries 
#include <SoftwareSerial.h>   //Software Serial Port
#include <ArduinoJson.h>
#include <Wire.h>
#include <Time.h>
#include <Adafruit_MLX90614.h>
/*
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
*/
//define constants
#define RxD 10
#define TxD 11
#define LIGHT 13
unsigned long t;

int values[20];
int iterator = 0;

boolean measure = false;
//StaticJsonDocument<400> doc;


int loops = 10;
int postpone = 50;
SoftwareSerial blueToothSerial(RxD,TxD);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
String recievedData = "";

void(* resetFunc) (void) = 0;

void setup() 
{ 
  Serial.begin(9600);
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  pinMode(LIGHT, OUTPUT);
  digitalWrite(LIGHT, LOW);
  setupBlueToothConnection(); 
  mlx.begin();
} 

 
void loop(){   
  char recvChar;
  
  while(blueToothSerial.available()){
     recvChar = blueToothSerial.read();
     if(recvChar == '$'){   
       digitalWrite(LIGHT, HIGH);     
       //t = millis();       
       processMessage(recievedData);        
       digitalWrite(LIGHT, LOW);
       recievedData = "";
     }
     else
      recievedData.concat(recvChar);
    
  }
  
  if(measure){
    getValues();
  }
  
} 

void processMessage(String data){
    StaticJsonDocument<100> doc;
    DeserializationError error = deserializeJson(doc, data);
    if(error) {
        Serial.println("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }
    Serial.println(data);
    String data_request = doc["data_request"];
    String configure = doc["config"];
    //Serial.print("data: ");
    //Serial.println(data_request);
    //Serial.print("config: ");
    //Serial.println(configure);

        
    if(data_request == "true" && configure == "true"){
      Serial.println("true true");      
      configResponse();
      return;
    }
    
    if(data_request == "true" && configure == "false"){ 
      Serial.println("true false"); 
      measure=true;     
      //sendTemp();
      return;
    }

    if(data_request == "false" && configure == "true"){  
      Serial.println("false true");     
      loops = doc["loops"];
      postpone = doc["delay"];      
      return;
    }
   
    if(data_request == "false" && configure == "false"){
      Serial.println("false false"); 
      measure=false;
      String con = doc["connection"];
      if(con == "true"){
        delay(2000);      
        resetFunc(); 
      }     
       
      return;
    }  
}




void configResponse(){
  DynamicJsonDocument root(256);
  root["data"] = false;
  root["sensor"] = "EMG";
  root["time"] = postpone;
  root["loops"] = loops;
  
  serializeJson(root, blueToothSerial);        
  blueToothSerial.println();
  serializeJson(root, Serial);        
  Serial.println();
  
  }

void sendData(){
  
  StaticJsonDocument<300> doc;
  Serial.println(iterator);
  doc.clear();
  String ar="";
  doc["data"] = true;
  doc["sensor"] = "EMG";
  ar.concat("[");
  for(int j=0;j < iterator; j++){
    char snum[6];

    // convert 123 to string [buf]
    itoa(values[j], snum, 10);
    delay(100);
    //Serial.print("itoa output: ");
    //Serial.print(snum);
    // print our string
    ar.concat(snum);
    delay(10);
    if(j < loops-1)
    ar.concat(",");
    //Serial.println(values[j]);
    //Serial.println(ar);
  }
  ar.concat("]");
  //Serial.println(iterator + " " + ar);
  doc["values"] = ar;
  Serial.println(ar);
  /*JsonArray valuesjson = doc.createNestedArray("values");
  if(valuesjson){
    Serial.println("nic podozrive");
    }
    else
    Serial.println("tuto je fuckup");
  for(int j=0;j < iterator; j++){
    Serial.print(values[j]);
      //value0["time"] = values[j].timeStamp;
      //valuesjson.add(values[j]);
      
      
      if(valuesjson.add(values[j]))        
        Serial.println("  good");
       else
        Serial.println("  bad");
      
  }
 
*/ 
  
  iterator=0;

  serializeJson(doc, Serial);
  Serial.println();
  serializeJson(doc, blueToothSerial);        
  blueToothSerial.println();
  doc.clear();
  //Serial.println(millis() - t); 
  //serializeJson(root, Serial);        
  //Serial.println();
  

}


void getValues(){
  for(int j = 0; j < loops; j++){
    values[iterator] = analogRead(A0);
    Serial.println(iterator);
    Serial.println(values[j]);
    
    iterator++;
    
    delay(postpone);   
 }
 sendData();
}
 
void setupBlueToothConnection()
{
  // zdroj: https://rlx.sk/sk/bluetooth/3577-arduino-bluetooth-shield-masterslave-er-mcs01108s-v20edr-3mbps-24ghz-.html
  
  blueToothSerial.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
/*  blueToothSerial.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
  blueToothSerial.print("\r\n+STNA=tepelnySenzor\r\n"); //set the bluetooth name as "tepelnySenzor"
  blueToothSerial.print("\r\n+STPIN=0000\r\n");//Set SLAVE pincode"0000"
  blueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  blueToothSerial.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here
  delay(2000); // This delay is required.
  blueToothSerial.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable 
  Serial.println("The slave bluetooth is inquirable!");
  delay(2000); // This delay is required.
  blueToothSerial.flush();
*/
}
