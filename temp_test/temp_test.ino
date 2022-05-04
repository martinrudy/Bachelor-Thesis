#include <ArduinoJson.h>
#include <Wire.h>
#include <Time.h>
#include <Adafruit_MLX90614.h>
#include <SparkFun_Bio_Sensor_Hub_Library.h>
#include <LowPower.h>


const int resPin = 2;
const int mfioPin = 3;

int loops = 10;
int postpone = 50;

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin); 
bioData body;

void setup() {
  Serial.begin(9600);
  mlx.begin();
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,LOW);

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

void loop() {
  digitalWrite(LED_BUILTIN,HIGH);
  DynamicJsonDocument root(128);
  root["data"] = true;
  root["sensor"] = "Temperature";
  root["tempObject"] = mlx.readObjectTempC();
  root["tempAmbient"] = mlx.readAmbientTempC();
  delay(2000);
  body = bioHub.readBpm();
  root["heartRate"] = body.heartRate;
  root["oxygen"] = body.oxygen;
  serializeJson(root, Serial);        
  Serial.println();
  delay(2000);

  Serial.println("Arduino:- I am going for a Nap");

  delay(200);

  //digitalWrite(LED_BUILTIN,LOW);

  //LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);

  Serial.println("Arduino:- Hey I just Woke up");

  Serial.println("");

  delay(2000);

}
