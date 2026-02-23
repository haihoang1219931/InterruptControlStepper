#include "SmoothMotion.h"

SmoothMotion motionControl;
SmoothMotion::SmoothMotion() {

}

void SmoothMotion::init() {
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
  stateControl = MOTOR_EXECUTE_INCREASE_SPEED;

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

ISR(TIMER1_COMPA_vect)
{
  motionControl.motionControlLoop();
}

float SmoothMotion::delayAccel(float stepCount, float delayCur) {
  // Serial.print("d[");
  // Serial.print(delayCur);
  // Serial.print("]");
  // Serial.print(" s[");
  // Serial.print(stepCount);
  // Serial.print("] T:");
  return delayCur * (4.0f*stepCount - 1.0f) / (4.0f*stepCount + 1.0f);
}

float SmoothMotion::delayDecel(float stepCount, float delayCur) {
  // Serial.print("d[");
  // Serial.print(delayCur);
  // Serial.print("]");
  // Serial.print(" s[");
  // Serial.print(stepCount);
  // Serial.print("] T:");
  return delayCur * (4.0f*stepCount + 1.0f) / (4.0f*stepCount - 1.0f);
}
void SmoothMotion::motionControlLoop() {
  switch (stateControl) {
    case MOTOR_EXECUTE_INCREASE_SPEED:{
      increaseSpeed();
    }
    break;
    case MOTOR_EXECUTE_CRUISE_SPEED:{
      cruiseSpeed();
    }
    break;
    case MOTOR_EXECUTE_DECREASE_SPEED:{
      decreaseSpeed();
    }
    break;
    case MOTOR_EXECUTE_HOME: {
      goHome();
    }
    break;
    case MOTOR_EXECUTE_DONE: {
      Serial.println("Done");
      digitalWrite(enablePin, HIGH);
      stateControl = MOTOR_EXECUTE_WAIT_COMMAND;
    }
    break;
  }
}

void SmoothMotion::increaseSpeed() {
  switch(motionAccelState) {
    case MOTION_INIT: {
      Serial.println("Accel");
      statePulse = STATE_HIGH;
      numWaitPulse = d0;
      delayTime = d0;
      motionAccelState = MOTION_PULSE;
    }
    break;
    case MOTION_PULSE: {
      pulseLoop();
      if(statePulse == STATE_DONE) {
        motionAccelState = MOTION_UPDATE_DELAY;
      }
    }
    break;
    case MOTION_UPDATE_DELAY: {
      stepCountAccel ++;        
      delayTime = delayAccel(stepCountAccel,delayTime);
      numWaitPulse = delayTime;
      if(delayTime >= 2) {
        statePulse = STATE_HIGH;
        motionAccelState = MOTION_PULSE;
      } else {
        motionAccelState = MOTION_DONE;
      }
    }
    break;
    case MOTION_DONE: {
      stateControl = MOTOR_EXECUTE_CRUISE_SPEED;
      Serial.print("stepCountAccel:");
      Serial.println(stepCountAccel);
    }
    break;
  }
}

void SmoothMotion::cruiseSpeed() {
  switch(motionCruiseState) {
    case MOTION_INIT: {
      Serial.println("Cruise");
      statePulse = STATE_HIGH;
      numWaitPulse = 2;
      motionCruiseState = MOTION_PULSE;
    }
    break;
    case MOTION_PULSE: {
      pulseLoop();
      if(statePulse == STATE_DONE) {
        motionCruiseState = MOTION_CHECK_COUNTER;
      }
    }
    break;
    case MOTION_CHECK_COUNTER: {
      stepCountCruise ++;              
      if(stepCountCruise < (uint32_t)(200*8*1)) {
        statePulse = STATE_HIGH;
        motionCruiseState = MOTION_PULSE;
      } else {
        motionCruiseState = MOTION_DONE;
      }
    }
    break;
    case MOTION_DONE: {
      stateControl = MOTOR_EXECUTE_DECREASE_SPEED;
    }
    break;
  }
}

void SmoothMotion::decreaseSpeed() {
  switch(motionDecelState) {
    case MOTION_INIT: {
      Serial.println("Decel");
      statePulse = STATE_HIGH;
      numWaitPulse = 2;
      delayTime = 2; 
      stepCountDecel = stepCountAccel+1;
      motionDecelState = MOTION_PULSE;
    }
    break;
    case MOTION_PULSE: {
      pulseLoop();
      if(statePulse == STATE_DONE) {
        motionDecelState = MOTION_UPDATE_DELAY;
      }
    }
    break;
    case MOTION_UPDATE_DELAY: {
      stepCountDecel --;        
      delayTime = delayDecel(stepCountDecel,delayTime);
      numWaitPulse = delayTime;
      if(stepCountDecel > 0) {
        statePulse = STATE_HIGH;
        motionDecelState = MOTION_PULSE;
      } else {
        motionDecelState = MOTION_DONE;
      }
    }
    break;
    case MOTION_DONE: {
      stateControl = MOTOR_EXECUTE_DONE;
    }
    break;
  }
}

void SmoothMotion::goHome()
{

}

void SmoothMotion::pulseLoop()
{
  switch(statePulse){
    case STATE_HIGH: {
      digitalWrite(stepPinX,HIGH);
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
      statePulse = STATE_WAIT2;
      pulseCount = 0;
    }
    break;
    case STATE_WAIT2: {
      pulseCount++;
      if(pulseCount >= numWaitPulse) statePulse = STATE_DONE;
    }
    break;
  }
}
