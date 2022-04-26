//include libraries 
#include <SoftwareSerial.h>   //Software Serial Port
#include <ArduinoJson.h>
#include <SparkFun_Bio_Sensor_Hub_Library.h>
#include <Wire.h>
#include <Time.h>

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

// Reset pin, MFIO pin
int resPin = 2;
int mfioPin = 3;

// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin); 
bioData body;

int loops = 10;
int postpone = 50;
SoftwareSerial blueToothSerial(RxD,TxD);
 

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

  // setup oxymeter
  Wire.begin();
  int result = bioHub.begin();
  if (result == 0) // Zero errors!
    Serial.println("Sensor started!");
  else
    Serial.println("Could not communicate with the sensor!!!");
 
  Serial.println("Configuring Sensor...."); 
  int error = bioHub.configBpm(MODE_ONE); // Configuring just the BPM settings. 
  if(error == 0){ // Zero errors!
    Serial.println("Sensor configured.");
  }
  else {
    Serial.println("Error configuring sensor.");
    Serial.print("Error: "); 
    Serial.println(error); 
  }

  // Data lags a bit behind the sensor, if you're finger is on the sensor when
  // it's being configured this delay will give some time for the data to catch
  // up. 
  Serial.println("Loading up the buffer with data....");
  delay(4000); 
  
  
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
      getData();
      return;
    }

    if(data_request == "false" && configure == "true"){      
      loops = doc["loops"];
      postpone = doc["delay"];      
      return;
    }
   
    if(data_request == "false" && configure == "false"){
      Serial.println("false false");
      delay(2000);      
      resetFunc();
      return;
    }  
}




void configResponse(){
  DynamicJsonDocument root(128);
  root["data"] = false;
  root["sensor"] = "Oxygen";
  root["time"] = postpone;
  root["loops"] = loops;
  
  serializeJson(root, blueToothSerial);        
  blueToothSerial.println();
  serializeJson(root, Serial);        
  Serial.println();
  
  }

void getData(){
  while(1){
    // Information from the readBpm function will be saved to our "body"
    // variable.  
    body = bioHub.readBpm();
    if(body.heartRate == 0 || body.oxygen == 0 || body.confidence < 60){
      Serial.println("Unsuccessful measurement");
      }else{
          DynamicJsonDocument root(128);
          root["data"] = true;
          root["sensor"] = "Oxygen";
          root["heartRate"] = body.heartRate;
          root["oxygen"] = body.oxygen;
          serializeJson(root, blueToothSerial);        
          blueToothSerial.println();
          
          serializeJson(root, Serial);        
          Serial.println();
          break;
          delay(2000);
        }
  } 
    // Slow it down or your heart rate will go up trying to keep up
    // with the flow of numbers
    delay(1000); 
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
  blueToothSerial.flush();
*/
}
