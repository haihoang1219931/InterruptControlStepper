// #define STEPPER_MINI
#define MOTION_TEST
#ifdef MOTION_TEST
#include "SmoothMotion.h"
#define enPin 8
#define stepXPin 2 //X.STEP
#define dirXPin 5 // X.DIR
#define stepYPin 4 //Y.STEP
#define dirYPin 7 // Y.DIR

#define limitX 13 // X.LIMIT
#define limitY 10 // Y.LIMIT

#define miniStepperUpdownPin1 34
#define miniStepperUpdownPin2 36
#define miniStepperUpdownPin3 38
#define miniStepperUpdownPin4 40

#define miniStepperGripperPin1 24
#define miniStepperGripperPin2 26
#define miniStepperGripperPin3 28
#define miniStepperGripperPin4 30

#define limitUpdown 11
#define limitGripper 62 // A8

SmoothMotion motionDriver1(1,enPin, dirXPin, stepXPin);
SmoothMotion motionDriver2(2,enPin, dirYPin, stepYPin);
SmoothMotion motionUpdown(5,miniStepperUpdownPin1,miniStepperUpdownPin2,
  miniStepperUpdownPin3,miniStepperUpdownPin4);
SmoothMotion motionGripper(0,miniStepperGripperPin1,miniStepperGripperPin2,
  miniStepperGripperPin3,miniStepperGripperPin4);
float dZero(float acceleration) {
  float _c0 = 0.676 * sqrt(2.0 / acceleration) * 1000000.0; // Equation 15
  return _c0;
}
void enableMotionTask(bool enable)
{
  if(enable)
    TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  else
    TIMSK1 &= ~(1 << OCIE1A);
}
uint32_t ratio1 ;
uint32_t totalSteps1;
uint32_t startDelay1;
uint32_t minDelay1;
uint32_t ratio2;
uint32_t totalSteps2;
uint32_t startDelay2;
uint32_t minDelay2;
uint32_t ratio3;
uint32_t totalSteps3;
uint32_t startDelay3;
uint32_t minDelay3;
int dir =1;
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
  digitalWrite(dirXPin, LOW);
  digitalWrite(stepXPin, HIGH);
  digitalWrite(dirYPin, LOW);
  digitalWrite(stepYPin, HIGH);

  // initialize timer1
  float samplerate = 20000.0f;
  noInterrupts(); // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 16000000.0f / samplerate; // compare match register for IRQ with selected samplerate
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS10); // no prescaler
  interrupts(); // enable all interrupts
  
  Serial.println("===Start===");
  float d0 = dZero(50000000.0f);
  d0 = 1;
  Serial.print("dZero: ");
  Serial.println(d0);
  
  totalSteps1 = 800;
  startDelay1 = 24;
  minDelay1 = 2;

  ratio2 = 40;
  totalSteps2 = 78;
  startDelay2 = 246;
  minDelay2 = 2;

  ratio3 = 14;
  totalSteps3 = 229;
  startDelay3 = 21;
  minDelay3 = 21;

  motionDriver1.setupTarget(totalSteps1 * 0 / 10,
                            totalSteps1 * 10 / 10,
                            totalSteps1 * 0 / 10,
                            1,false,startDelay1,minDelay1);
  motionDriver2.setupTarget(totalSteps2 * 0 / 10,
                            totalSteps2 * 10 / 10,
                            totalSteps2 * 0 / 10,
                            1,false,startDelay2,minDelay2);
  motionUpdown.setupTarget(totalSteps3 * 0 / 10,
                            totalSteps3 * 10 / 10,
                            totalSteps3 * 0 / 10,
                            1,false,startDelay3,minDelay3);
  enableMotionTask(true);
  // while(motionDriver1.getCurrentSteps() < totalSteps1)
  // for(int i=0;i<183280;i++)
  // {
  //   motionDriver1.motionControlLoop();
  //   delayMicroseconds(25);
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
    // while(motionUpdown.getCurrentSteps() < totalSteps3)
  // for(uint32_t i=0;i<72960;i++)
  // {
  //   motionUpdown.motionControlLoop();
  //   // motionUpdown.pulseLoop();
  //   delayMicroseconds(25);
  // }
  // Serial.print("M3 totalPulse");
  // Serial.println(motionUpdown.m_totalPulse);
  // Serial.print("M3 countStep: ");
  // Serial.print(motionUpdown.getCurrentSteps());
  // Serial.print("/");
  // Serial.println(totalSteps3);
  
}
void loop()
{
  if(motionDriver1.getCurrentSteps() >= totalSteps1 && 
    motionDriver2.getCurrentSteps() >= totalSteps2 &&
    motionUpdown.getCurrentSteps() >= totalSteps3) {
    enableMotionTask(false);
    dir=-dir;
    digitalWrite(dirXPin, dir > 0 ? LOW:HIGH);
    digitalWrite(dirYPin, dir > 0 ? LOW:HIGH);
    motionDriver1.setupTarget(totalSteps1 * 0 / 10,
                            totalSteps1 * 10 / 10,
                            totalSteps1 * 0 / 10,
                            dir,false,startDelay1,minDelay1);
    motionDriver2.setupTarget(totalSteps2 * 0 / 10,
                            totalSteps2 * 10 / 10,
                            totalSteps2 * 0 / 10,
                            dir,false,startDelay2,minDelay2);
    motionUpdown.setupTarget(totalSteps3 * 0 / 10,
                            totalSteps3 * 10 / 10,
                            totalSteps3 * 0 / 10,
                            dir,false,startDelay3,minDelay3);
    delay(2000);
    enableMotionTask(true);                            
  }
  delay(1000);
}
ISR(TIMER1_COMPA_vect)
{
  motionDriver2.motionControlLoop();
  motionDriver1.motionControlLoop();
  motionUpdown.motionControlLoop();
  // motionGripper.motionControlLoop();
}
#elif defined PULSE_SAMPLE 
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
#elif defined STEPPER_MINI
// Code by: Gautam Gundap

// Define the pins connected to the IN1-IN4 on the ULN2003 driver board
const int in1Pin = 34;
const int in2Pin = 36;
const int in3Pin = 38;
const int in4Pin = 40;
void moveStep(int dir, 
  int m_pin1,int m_pin2,int m_pin3,int m_pin4,
  int sleepTime);
void setup() {
  // Set the motor control pins as outputs
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);
}
int dir = -1;
void loop() {
  int stepsPerRevolution = 228;
  // Rotate the motor one full revolution in one direction
  for (int i = 0; i < stepsPerRevolution; i++) {
    // Move one step forward
    moveStep(dir,in1Pin,in2Pin,in3Pin,in4Pin,1200);
  }
  dir = -dir;
  delay(1000); // Pause for a second
}

void moveStep(int m_dir, 
  int m_pin1,int m_pin2,int m_pin3,int m_pin4,
  int sleepTime)
{
  if(m_dir < 0)
  {
    //1
    digitalWrite(m_pin1, HIGH);
    digitalWrite(m_pin2, LOW);
    digitalWrite(m_pin3, LOW);
    digitalWrite(m_pin4, LOW);
    delayMicroseconds(sleepTime);
    //2
    digitalWrite(m_pin1, HIGH);
    digitalWrite(m_pin2, HIGH);
    digitalWrite(m_pin3, LOW);
    digitalWrite(m_pin4, LOW);
    delayMicroseconds(sleepTime);
    //3
    digitalWrite(m_pin1, LOW);
    digitalWrite(m_pin2, HIGH);
    digitalWrite(m_pin3, LOW);
    digitalWrite(m_pin4, LOW);
    delayMicroseconds(sleepTime);
    //4
    digitalWrite(m_pin1, LOW);
    digitalWrite(m_pin2, HIGH);
    digitalWrite(m_pin3, HIGH);
    digitalWrite(m_pin4, LOW);
    delayMicroseconds(sleepTime);
    //5
    digitalWrite(m_pin1, LOW);
    digitalWrite(m_pin2, LOW);
    digitalWrite(m_pin3, HIGH);
    digitalWrite(m_pin4, LOW);
    delayMicroseconds(sleepTime);
    //6
    digitalWrite(m_pin1, LOW);
    digitalWrite(m_pin2, LOW);
    digitalWrite(m_pin3, HIGH);
    digitalWrite(m_pin4, HIGH);
    delayMicroseconds(sleepTime);
    //7
    digitalWrite(m_pin1, LOW);
    digitalWrite(m_pin2, LOW);
    digitalWrite(m_pin3, LOW);
    digitalWrite(m_pin4, HIGH);
    delayMicroseconds(sleepTime);
    //8
    digitalWrite(m_pin1, HIGH);
    digitalWrite(m_pin2, LOW);
    digitalWrite(m_pin3, LOW);
    digitalWrite(m_pin4, HIGH);
    delayMicroseconds(sleepTime);
  } 
  
  else{
    //1
    digitalWrite(m_pin1, HIGH);
    digitalWrite(m_pin2, LOW);
    digitalWrite(m_pin3, LOW);
    digitalWrite(m_pin4, HIGH);
    delayMicroseconds(sleepTime);
    //2
    digitalWrite(m_pin1, LOW);
    digitalWrite(m_pin2, LOW);
    digitalWrite(m_pin3, LOW);
    digitalWrite(m_pin4, HIGH);
    delayMicroseconds(sleepTime);
    //3
    digitalWrite(m_pin1, LOW);
    digitalWrite(m_pin2, LOW);
    digitalWrite(m_pin3, HIGH);
    digitalWrite(m_pin4, HIGH);
    delayMicroseconds(sleepTime);
    //4
    digitalWrite(m_pin1, LOW);
    digitalWrite(m_pin2, LOW);
    digitalWrite(m_pin3, HIGH);
    digitalWrite(m_pin4, LOW);
    delayMicroseconds(sleepTime);
    //5
    digitalWrite(m_pin1, LOW);
    digitalWrite(m_pin2, HIGH);
    digitalWrite(m_pin3, HIGH);
    digitalWrite(m_pin4, LOW);
    delayMicroseconds(sleepTime);
    //6
    digitalWrite(m_pin1, LOW);
    digitalWrite(m_pin2, HIGH);
    digitalWrite(m_pin3, LOW);
    digitalWrite(m_pin4, LOW);
    delayMicroseconds(sleepTime);
    //7
    digitalWrite(m_pin1, HIGH);
    digitalWrite(m_pin2, HIGH);
    digitalWrite(m_pin3, LOW);
    digitalWrite(m_pin4, LOW);
    delayMicroseconds(sleepTime);
    //8
    digitalWrite(m_pin1, HIGH);
    digitalWrite(m_pin2, LOW);
    digitalWrite(m_pin3, LOW);
    digitalWrite(m_pin4, LOW);
    delayMicroseconds(sleepTime);
  }
}
#else
#include <AccelStepper.h>

// Define pins for Stepper A and B
AccelStepper stepperA(AccelStepper::DRIVER, 2, 5); // (Type, Step, Dir)
AccelStepper stepperB(AccelStepper::DRIVER, 4, 7);

// Configuration for the "Master" (the motor with the longest move)
float masterMaxSpeed = 500000.0;
float masterAccel = 500000.0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Example move: Motor A moves 2000 steps, Motor B moves 500 steps
  moveToSynchronized(20000, 10000);
  
  delay(20000); // Wait before next move
  
  moveToSynchronized(0, 0); // Move back to start
  delay(2000);
}

void moveToSynchronized(long targetA, long targetB) {
  long distA = abs(targetA - stepperA.currentPosition());
  long distB = abs(targetB - stepperB.currentPosition());

  // Determine the Master (longest path) and the Ratio
  float ratio;
  if (distA >= distB) {
    // Motor A is Master
    ratio = (distA > 0) ? (float)distB / distA : 0;
    
    stepperA.setMaxSpeed(masterMaxSpeed);
    stepperA.setAcceleration(masterAccel);
    
    // Scale Motor B to match Motor A's timing
    stepperB.setMaxSpeed(masterMaxSpeed * ratio);
    stepperB.setAcceleration(masterAccel * ratio);
  } else {
    // Motor B is Master
    ratio = (distB > 0) ? (float)distA / distB : 0;
    
    stepperB.setMaxSpeed(masterMaxSpeed);
    stepperB.setAcceleration(masterAccel);
    
    // Scale Motor A to match Motor B's timing
    stepperA.setMaxSpeed(masterMaxSpeed * ratio);
    stepperA.setAcceleration(masterAccel * ratio);
  }

  stepperA.moveTo(targetA);
  stepperB.moveTo(targetB);

  // Run both until they reach the target
  while (stepperA.isRunning() || stepperB.isRunning()) {
    stepperA.run();
    stepperB.run();
  }
}
#endif