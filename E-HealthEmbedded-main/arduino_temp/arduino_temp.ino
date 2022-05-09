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
  
  if(blueToothSerial.available()){
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
} 

void processMessage(String data){
    StaticJsonDocument<100> doc;
    DeserializationError error = deserializeJson(doc, data);
    if(error) {
        Serial.println("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }
    String data_request = doc["data_request"];
    String configure = doc["config"];
    //Serial.print("data: ");
    //Serial.println(data_request);
    //Serial.print("config: ");
    //Serial.println(configure);
  
        
    if(data_request == "true" && configure == "true"){      
      configResponse();
      return;
    }
    
    if(data_request == "true" && configure == "false"){      
      sendTemp();
      return;
    }

    if(data_request == "false" && configure == "true"){      
      loops = doc["loops"];
      postpone = doc["delay"];      
      return;
    }
   
    if(data_request == "false" && configure == "false"){
      delay(2000);      
      resetFunc();
      return;
    }  
}




void configResponse(){
  DynamicJsonDocument root(128);
  root["data"] = false;
  root["sensor"] = "Temperature";
  root["time"] = postpone;
  root["loops"] = loops;
  
  serializeJson(root, blueToothSerial);        
  blueToothSerial.println();
  serializeJson(root, Serial);        
  Serial.println();
  
  }

void sendTemp(){
  DynamicJsonDocument root(128);
  root["data"] = true;
  root["sensor"] = "Temperature";
  root["tempObject"] = getObjTemperature();
  root["tempAmbient"] = getAmbTemperature();
  serializeJson(root, blueToothSerial);        
  blueToothSerial.println();
  //Serial.println(millis() - t); 
  //serializeJson(root, Serial);        
  //Serial.println();
  
}



float getObjTemperature(){
  float sum = 0;
  for(int j = 0; j < loops; j++){
    float temp = mlx.readObjectTempC();
    if(temp < 1000 && temp > -50)
      sum += temp;
    else
      j--;  
    delay(postpone);
  }
  sum = sum/loops;
  return (float(round(sum*100)) / 100);
}

float getAmbTemperature(){
  float sum = 0;
  for(int j = 0; j < loops; j++){
    float temp = mlx.readAmbientTempC();
    if(temp < 1000 && temp > - 100)
      sum += temp;
    else
      j--;  
    delay(postpone);
  }
  sum = sum/loops;
  
  return (float(round(sum*100)) / 100);
}
 
void setupBlueToothConnection()
{
  // zdroj: https://rlx.sk/sk/bluetooth/3577-arduino-bluetooth-shield-masterslave-er-mcs01108s-v20edr-3mbps-24ghz-.html
  
  blueToothSerial.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  /*blueToothSerial.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
  blueToothSerial.print("\r\n+STNA=tepelnySenzor\r\n"); //set the bluetooth name as "tepelnySenzor"
  blueToothSerial.print("\r\n+STPIN=0000\r\n");//Set SLAVE pincode"0000"
  blueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  blueToothSerial.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here
  delay(2000); // This delay is required.
  blueToothSerial.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable 
  Serial.println("The slave bluetooth is inquirable!");
  delay(2000); // This delay is required.
  blueToothSerial.flush();*/

}
