#include <TH02_dev.h>
#include "Arduino.h"
#include "Wire.h" 
#include "Ultrasonic.h"
#include <ArduinoJson.h>

Ultrasonic ultrasonic_back(3);
Ultrasonic ultrasonic_front(2);
StaticJsonDocument<500> object;


void setup() {
  Serial.begin(9600);
  while (!Serial) continue;
  
  object["gas"] = 0;
  object["frontObject"] = false;
  object["backObject"] = false; 
  object["temperature"] = -1;
  object["humidity"] = 0;

 
  
  /* Power up,delay 150ms,until voltage is stable */
  delay(150);
  /* Reset HP20x_dev */
  TH02.begin();
  delay(100);
  
  /* Determine TH02_dev is available or not */
  Serial.println("TH02_dev is available.\n"); 
  
 
  
}

void loop() {
  float R0 = -0.1;
  float sensor_volt;
  float RS_gas; // Get value of RS in a gas
  float ratio; // Get ratio RS_gas/RS_air
  float ppm; // get ppm of air at an instant
  float ratio_CO; // get value of ratio from ppm
  float ratio_methane;
  float ratio_hydrogen; 
  float ratio_LPG;
  float temp_ratio;
  long RangeInInches_front;
  long RangeInCentimeters_front;
  long RangeInInches_back;
  long RangeInCentimeters_back;
 
  int sensorValue = analogRead(A0);

  sensor_volt=(float)sensorValue/1024*5.0;
  ppm = 500 * sensor_volt;
  RS_gas = (5.0-sensor_volt)/sensor_volt; // omit *RL
 
 
  ratio = RS_gas/R0;  // ratio = RS/R0 
 
  Serial.print("raw_value = ");
  Serial.print(sensorValue);
  Serial.print("sensor_volt = ");
  Serial.println(sensor_volt);
  Serial.print("RS_ratio = ");
  Serial.println(RS_gas);
  Serial.print("Rs/R0 = ");
  Serial.println(ratio);
  // test all the equations
  ratio_CO = 24.991*(pow(ppm,-0.180131)) -3.47894;
  ratio_methane = 15.7351*(pow(ppm,-0.29257)) -0.369972;
  ratio_hydrogen = -0.024143*(pow(ppm, 0.413498)) + 1.35648;
  ratio_LPG = 12.1828*(pow(ppm,-0.352812)) -0.290345;

  
  Serial.println("resultant ratios:");
  Serial.println(ratio_CO);
  Serial.println(ratio_methane);
  Serial.println(ratio_hydrogen);
  Serial.println(ratio_LPG);

  if (ratio >= (ratio_CO-0.05) && ratio <= (ratio_CO-0.05)){
     // send server "gas" = 1
     Serial.println("Carbon Monoxide");
     object["gas"] = 1;
     serializeJson(object["gas"], Serial);
    }
  
  else if (ratio >= (ratio_methane -0.05) && ratio <= (ratio_methane -0.05)){
     // send server CO detected
     Serial.println("Methane/Smoke/Alcohol");
     object["gas"] = 2;
    serializeJson(object["gas"], Serial);
    }
  else if (ratio >= (ratio_hydrogen -0.05) && ratio <= (ratio_hydrogen -0.05)){
     // send server CO detected
     Serial.println("Hydrogen");
     object["gas"] = 3;
     serializeJson(object["gas"], Serial);
    }
  else if (ratio >= (ratio_LPG -0.05) && ratio <= (ratio_LPG -0.05)){
     // send server CO detected
     Serial.println("LPG/Propane");
     object["gas"] = 4;
     serializeJson(object["gas"], Serial);
    }
  else {
    // send server air is polliution-free
    Serial.println("Air is clean");
    object["gas"] = 0;
  serializeJson(object["gas"], Serial);
    }

// test with functions to detect a gas
  Serial.print("\n\n");
  delay(1000);

   float temper = TH02.ReadTemperature(); 
   Serial.println("temperature: ");   
   Serial.print(temper);
   Serial.println("C\r\n");
   
   float humidity = TH02.ReadHumidity();
   Serial.println("humidity: ");
   Serial.print(humidity);
   Serial.println("%\r\n");
   delay(1000);

   // temperature: 0 (cold) 1 (normal) 2 (hot)

   if (temper <= 15){
    // send 0 to server
    Serial.print("Cold");
    object["temperature"] = 0;
  serializeJson(object["temperature"], Serial);
    }

   else if (temper >= 50){
   // send 2 to server  
   Serial.print("Hot");
   object["temperature"] = 2;
   serializeJson(object["temperature"], Serial);
   }

   else {
    // send 1 to server
    object["temperature"] = 1;
    serializeJson(object["temperature"], Serial);
    }

   if (humidity<=20){
    // send -1 to server
    Serial.print("Dry");
    object["humidity"] = -1;
   serializeJson(object["humidity"], Serial);
    
    }
    
   else if (humidity >= 70){
    // send 1 to receiver
    Serial.print("Wet");
    object["humidity"] = 1;
    serializeJson(object["humidity"], Serial);
    }
  
   else{
    // send 0 to server
    object["humidity"] = 0;
    serializeJson(object["humidity"], Serial);
    }

    Serial.println("The distance to obstacles in front is: ");
    RangeInCentimeters_front = ultrasonic_front.MeasureInCentimeters(); // two measurements should keep an interval
    Serial.print(RangeInCentimeters_front);//0~400cm
    Serial.println(" cm");
    delay(250);

    Serial.println("The distance to obstacles behind is: ");
    RangeInCentimeters_back = ultrasonic_back.MeasureInCentimeters(); // two measurements should keep an interval
    Serial.print(RangeInCentimeters_back);//0~400cm
    Serial.println(" cm");
    delay(250);

    if (RangeInCentimeters_front<=15){
      Serial.print("Object in front!");
      object["frontObject"] = true;
       serializeJson(object["frontObject"], Serial);
    }

    if (RangeInCentimeters_back<=15){
      Serial.print("Object behind!");
      object["backObject"] = true;
       serializeJson(object["backObject"], Serial);
    } 

   
  
}
