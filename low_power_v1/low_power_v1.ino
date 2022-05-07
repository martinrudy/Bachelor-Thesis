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
#define resPin 2
#define mfioPin 3
#define myFileName "testik.txt"

unsigned long interval = 10000; // Change every 12 hours (12UL60UL60UL*1000UL)
unsigned long prevMillis = 0;
unsigned long actMillis = 0;

unsigned long interval_sleep = 2000;
int iterator = 2000;


File myFile;
// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin); 
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
bioData body; 

SoftwareSerial blueToothSerial(RxD,TxD);

void setup() 
{ 
  Serial.begin(9600);

  //init temperature
  mlx.begin();
  init_sd();
  init_oxi();


  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  setupBlueToothConnection();
   
}


void init_sd(){
  //intialization sd shield
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  //Serial.print("Initializing SD card...");

  if (!SD.begin(CS_pin)) {
    //Serial.println("initialization failed!");
    while (1);
  }
  //Serial.println("initialization done.");
  
}

//initialization oximeter
void init_oxi(){
  Wire.begin();
  int result = bioHub.begin();
  /*if (!result)
    Serial.println("Sensor started!");
  else
    Serial.println("Could not communicate with the sensor!!!");

  Serial.println("Configuring Sensor...."); */
  int error = bioHub.configBpm(MODE_ONE); // Configuring just the BPM settings. 
  /*if(error == 0){ // Zero errors!
    Serial.println("Sensor configured.");
  }
  else {
    Serial.println("Error configuring sensor.");
    Serial.print("Error: "); 
    Serial.println(error); 
  }*/
}

void measure_and_write()
{
  myFile = SD.open(myFileName, FILE_WRITE);
  body = bioHub.readBpm();

  StaticJsonDocument<64> root;
  root["tempObject"] = getObjTemperature();
  root["tempAmbient"] = getAmbTemperature();
  root["heartRate"] = body.heartRate;
  root["o"] = body.oxygen; 
  
  if (myFile) {
    //Serial.print("Writing to test3.txt...");
    serializeJson(root, myFile);
    myFile.println();
    // close the file:
    myFile.close();
    root.clear();
    //Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    //Serial.println("error opening test3.txt");
  }
  
}

void read_sd()
{
  myFile = SD.open(myFileName);
  if (myFile) {
    //Serial.println(myFileName);

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      char letter = myFile.read();
      Serial.print(letter);
      //blueToothSerial.print(letter);
      delay(10);
    }
    // close the file:
    myFile.close();

    SD.remove(myFileName);
  } else {
    // if the file didn't open, print an error:
    //Serial.println("error opening file");
  }
  delay(1000);
}


float getAmbTemperature(){
  float sum = 0;
  for(int j = 0; j < 10; j++){
    float temp = mlx.readAmbientTempC();
    if(temp < 1000 && temp > - 100)
      sum += temp;
    else
      j--;  
  }
  sum = sum/10;
  
  return (float(round(sum*100)) / 100);
}

float getObjTemperature(){
  float sum = 0;
  for(int j = 0; j < 10; j++){
    float temp = mlx.readObjectTempC();
    if(temp < 1000 && temp > -50)
      sum += temp;
    else
      j--;
  }
  sum = sum/10;
  return (float(round(sum*100)) / 100);
}

void power_sleep(long time_to_sleep){
  while(time_to_sleep > 0)
  { 
      if(time_to_sleep > iterator) 
      { 
          time_to_sleep -= iterator; 
          LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF); 
      } 
      else 
      { 
          LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
          break; 
      } 
  } 
  
}

void loop() 
{ 
  Serial.println(prevMillis);
  Serial.println(actMillis);
  Serial.println(interval);
  measure_and_write();
  if(actMillis - prevMillis >= interval){
    read_sd();
    prevMillis += interval;
  }
  Serial.println(millis());
  //actMillis += millis();
  actMillis += interval_sleep;
  power_sleep(interval_sleep);

} 

void setupBlueToothConnection()
{
  blueToothSerial.begin(38400); //Set BluetoothBee BaudRate to default baud rate 38400
  blueToothSerial.print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
  blueToothSerial.print("\r\n+STNA=RUDYHOBT\r\n"); //set the bluetooth name as "CrowBTSlave"
  blueToothSerial.print("\r\n+STPIN=0000\r\n");//Set SLAVE pincode"0000"
  blueToothSerial.print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
  blueToothSerial.print("\r\n+STAUTO=0\r\n"); // Auto-connection should be forbidden here
  delay(2000); // This delay is required.
  blueToothSerial.print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable 
  //Serial.println("BT");
  //digitalWrite(RxD, LOW);
  delay(2000); // This delay is required.
  blueToothSerial.flush();
}
