#ifndef SIMPLE_PULSE
#include "SmoothMotion.h"
#define enPin 8
#define stepXPin 2 //X.STEP
#define dirXPin 5 // X.DIR
#define stepYPin 4 //Y.STEP
#define dirYPin 7 // Y.DIR

#define limitX 13 // X.LIMIT
#define limitY 10 // Y.LIMIT
SmoothMotion motionDriver1(1,enPin, dirXPin, stepXPin);
SmoothMotion motionDriver2(2,enPin, dirYPin, stepYPin);
void enableMotionTask(bool enable)
{
  if(enable)
    TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  else
    TIMSK1 &= ~(1 << OCIE1A);
}
void setup()
{
  Serial.begin(38400);
  // init pins
  pinMode(enPin, OUTPUT);
  pinMode(dirXPin, OUTPUT);
  pinMode(stepXPin, OUTPUT);
  pinMode(dirYPin, OUTPUT);
  pinMode(stepYPin, OUTPUT);

  digitalWrite(enPin, LOW);
  digitalWrite(dirXPin, HIGH);
  digitalWrite(stepXPin, HIGH);
  digitalWrite(dirYPin, HIGH);
  digitalWrite(stepYPin, HIGH);

  // initialize timer1
  float samplerate = 40000.0f;
  noInterrupts(); // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 16000000.0f / samplerate; // compare match register for IRQ with selected samplerate
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS10); // no prescaler
  interrupts(); // enable all interrupts
  enableMotionTask(true);
  Serial.println("===Start===");
  uint32_t ratio1 = 1;
  uint32_t totalSteps1 = 10000 / ratio1;
  uint32_t startDelay1 = 100 * ratio1;
  uint32_t minDelay1 = 4 * ratio1;
  uint32_t ratio2 = 4;
  uint32_t totalSteps2 = 10000 / ratio2;
  uint32_t startDelay2 = 100 * ratio2;
  uint32_t minDelay2 = 4 * ratio2;
  motionDriver1.setupTarget(totalSteps1 * 2 / 10,
                            totalSteps1 * 6 / 10,
                            totalSteps1 * 2 / 10,
                            1,true,startDelay1,minDelay1);
  motionDriver2.setupTarget(totalSteps2 * 2 / 10,
                            totalSteps2 * 6 / 10,
                            totalSteps2 * 2 / 10,
                            1,true,startDelay2,minDelay2);
  // while(motionDriver1.getCurrentSteps() < totalSteps1)
  // // for(int i=0;i<280000;i++)
  // {
  //   motionDriver1.motionControlLoop();
  // }
  // Serial.print("M1 totalPulse");
  // Serial.println(motionDriver1.m_totalPulse);
  // Serial.print("M1 countStep: ");
  // Serial.print(motionDriver1.getCurrentSteps());
  // Serial.print("/");
  // Serial.println(totalSteps1);

  // while(motionDriver2.getCurrentSteps() < totalSteps2)
  // {
  //   motionDriver2.motionControlLoop();
  // }
  // Serial.print("M2 totalPulse");
  // Serial.println(motionDriver2.m_totalPulse);
  // Serial.print("M2 countStep: ");
  // Serial.print(motionDriver2.getCurrentSteps());
  // Serial.print("/");
  // Serial.println(totalSteps2);
  
}
void loop()
{
  // delay(5000);
  // enableMotionTask(false);
}
ISR(TIMER1_COMPA_vect)
{
  motionDriver1.motionControlLoop();
  // motionDriver2.motionControlLoop();
}
#else
// pin numbers
const int analogPin = A0;
const int enablePin = 8;
const int ms1Pin = 41;
const int ms2Pin = 42;
const int ms3Pin = 43;
const int resetPin = 44;
const int sleepPin = 45;
const int stepPinX = 2;
const int dirPinX = 5;
const int stepPinZ = 4;
const int dirPinZ = 7;

// interrupt frequency
const float samplerate = 40000.0f;

void setup()
{
  Serial.begin(38400);
  // init pins
  pinMode(enablePin, OUTPUT);
  pinMode(ms1Pin, OUTPUT);
  pinMode(ms2Pin, OUTPUT);
  pinMode(ms3Pin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(sleepPin, OUTPUT);
  pinMode(stepPinX, OUTPUT);
  pinMode(dirPinX, OUTPUT);
  pinMode(stepPinZ, OUTPUT);
  pinMode(dirPinZ, OUTPUT);

  digitalWrite(enablePin, LOW);
  digitalWrite(ms1Pin, LOW);
  digitalWrite(ms2Pin, LOW);
  digitalWrite(ms3Pin, LOW);
  digitalWrite(resetPin, LOW);
  digitalWrite(sleepPin, HIGH);
  digitalWrite(stepPinX, HIGH);
  digitalWrite(dirPinX, HIGH);
  digitalWrite(stepPinZ, HIGH);
  digitalWrite(dirPinZ, HIGH);

  delay(100);
  digitalWrite(resetPin, HIGH);

  // initialize timer1
  noInterrupts(); // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 16000000.0f / samplerate; // compare match register for IRQ with selected samplerate
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS10); // no prescaler
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  interrupts(); // enable all interrupts
}

uint32_t wait = 0;
int spd = 0xFffFFF;
int clk = 0;
uint32_t stepCount = 0;
int direction = 1;
enum {
  STATE_HIGH,
  STATE_WAIT1,
  STATE_LOW,
  STATE_WAIT2,
  STATE_DONE,
} STATE_PULSE;
uint8_t statePulse = STATE_HIGH;
uint8_t numWaitPulse = 3;
uint8_t pulseCount = 0;
// #define RUN_FAST
// timer 1 interrupt
ISR(TIMER1_COMPA_vect)
{
  switch(statePulse){
    case STATE_HIGH: {
      digitalWrite(stepPinX,HIGH);
      digitalWrite(stepPinZ,HIGH);
      statePulse = STATE_WAIT1;
      pulseCount = 0;
    }
    break;
    case STATE_WAIT1: {
      pulseCount++;
      if(pulseCount >= numWaitPulse) statePulse = STATE_LOW;
    }
    break;
    case STATE_LOW: {
      digitalWrite(stepPinX,LOW);
      digitalWrite(stepPinZ,LOW);
      statePulse = STATE_WAIT2;
      pulseCount = 0;
    }
    break;
    case STATE_WAIT2: {
      pulseCount++;
      if(pulseCount >= numWaitPulse) statePulse = STATE_HIGH;
    }
    break;
  }
}

void loop()
{
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  delay(1000);
  TIMSK1 &= ~(1 << OCIE1A);
  long startTime = millis();
  delay(1000);
  long currentTime = millis();
  Serial.println(currentTime - startTime);
}
#endif