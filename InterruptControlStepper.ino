
#include "SmoothMotion.h"
extern SmoothMotion motionControl;
void setup()
{
  Serial.begin(38400);
  Serial.println("======= Start =======");
  // uint32_t stepCountAccel = 0;
  // delayTime = 10000;
  // for(int i=0; i< 10; i++) {
  //   stepCountAccel ++;
  //   delayTime = delayAccel(stepCountAccel,delayTime);
  //   Serial.println(delayTime);
  // }  
  // stepCountAccel ++;
  // Serial.println("======= DECEL =======");
  // for(int i=0; i< 10; i++) {
  //   stepCountAccel --;
  //   delayTime = delayDecel(stepCountAccel,delayTime);
  //   Serial.println(delayTime);
  // }
  delay(2000);
  motionControl.init(); 
}

void loop()
{
  // Serial.println(delayTime);
  // digitalWrite(stepPinZ, HIGH);
  // delayMicroseconds((int)60);
  // digitalWrite(stepPinZ, LOW);
  // delayMicroseconds((int)60);
}