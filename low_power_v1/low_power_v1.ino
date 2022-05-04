/* Upload this sketch into Crowduino and press reset*/
 
#include <SoftwareSerial.h>   //Software Serial Port
#include <SparkFun_Bio_Sensor_Hub_Library.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include <LowPower.h>
#include <Adafruit_MLX90614.h>

// No other Address options.
#define DEF_ADDR 0x55
#define RxD 6
#define TxD 7
#define CS_pin 4
// Reset pin, MFIO pin
const int resPin = 2;
const int mfioPin = 3;
int loops = 10;
int postpone = 50;
const unsigned long interval = 100000; // Change every 12 hours (12UL60UL60UL*1000UL)
unsigned long previousMillis = 100000; // will store last time updated


// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin); 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
bioData body; 

File myFile;
 
//SoftwareSerial blueToothSerial(RxD,TxD);
 
void setup() 
{ 
  Serial.begin(9600);

  mlx.begin();
  //intialization sd shield
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(CS_pin)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  //initialization oximeter
  Wire.begin();
  int result = bioHub.begin();
  if (!result)
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
  
  //pinMode(RxD, INPUT);
  //pinMode(TxD, OUTPUT);
  //setupBlueToothConnection();
  
   
}


void write_to_sd(DynamicJsonDocument root)
{
  serializeJson(root, Serial);        
  Serial.println();
  /*myFile = SD.open("test3.txt", FILE_WRITE);
  body = bioHub.readBpm();
  DynamicJsonDocument root(128);
  root["data"] = true;
  root["sensor"] = "Oxygen";
  root["heartRate"] = body.heartRate;
  root["oxygen"] = body.oxygen;
  root["tempObject"] = mlx.readObjectTempC();
  root["tempAmbient"] = mlx.readAmbientTempC();
  
  if (myFile) {
    Serial.print("Writing to test3.txt...");
    serializeJson(root, myFile);
    myFile.println();
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test3.txt");
  }
  delay(100);*/
  
}

void read_sd()
{
  myFile = SD.open("test3.txt");
  if (myFile) {
    Serial.println("test3.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char letter = myFile.read();
      Serial.print(letter);
      //blueToothSerial.print(letter);
      //delay(10);
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test3.txt");
  }
  delay(1000);
}


void measure(){
  DynamicJsonDocument root(128);
  root["data"] = true;
  root["sensor"] = "Temperature";
  root["tempObject"] = getObjTemperature();
  root["tempAmbient"] = getAmbTemperature();
  //delay(1000);
  body = bioHub.readBpm();
  root["heartRate"] = body.heartRate;
  root["oxygen"] = body.oxygen;

  write_to_sd(root);
  
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

void loop() 
{ 
  
  measure();

} 

/*void setupBlueToothConnection()
{
  blueToothSerial.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  blueToothSerial.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
  blueToothSerial.print("\r\n+STNA=RUDYHOBT\r\n"); //set the bluetooth name as "CrowBTSlave"
  blueToothSerial.print("\r\n+STPIN=0000\r\n");//Set SLAVE pincode"0000"
  blueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  blueToothSerial.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here
  delay(2000); // This delay is required.
  blueToothSerial.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable 
  Serial.println("The slave bluetooth is inquirable!");
  delay(2000); // This delay is required.
  blueToothSerial.flush();
}*/
