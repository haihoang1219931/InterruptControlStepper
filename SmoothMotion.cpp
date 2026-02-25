#include "SmoothMotion.h"

SmoothMotion::SmoothMotion(uint32_t id, int enablePin, int dirPin, int stepPin):
  m_id(id),
  m_enablePin(enablePin),
  m_dirPin(dirPin),
  m_stepPin1(stepPin),
  m_motorType(MOTOR_TYPE_STEPPER_2_WIRES),
  m_stateControl(MOTOR_EXECUTE_WAIT_COMMAND)
{

}

SmoothMotion::SmoothMotion(uint32_t id, int stepPin1, int stepPin2, int stepPin3, int stepPin4):
  m_id(id),
  m_stepPin1(stepPin1),
  m_stepPin2(stepPin2),
  m_stepPin3(stepPin3),
  m_stepPin4(stepPin4),
  m_motorType(MOTOR_TYPE_STEPPER_4_WIRES),
  m_stateControl(MOTOR_EXECUTE_WAIT_COMMAND)
{
  pinMode(stepPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(stepPin3, OUTPUT);
  pinMode(stepPin4, OUTPUT);
}
// void setAcc(float acceleration) {
//   if(acceleration <= 0){return;}
//   first_step_delay =1000*(sqrt(2*1.8)/acceleration);

//   Serial.print("Acceleration ->  ");
//   Serial.println(acceleration);
// }

void SmoothMotion::setupTarget(
  uint32_t stepsAccel, uint32_t stepsCruise, uint32_t stepsDecel, 
  int direction, bool isAccel,  uint32_t accelStartWaitPulse, uint32_t minWaitPulse) {
  m_numStepAccel = stepsAccel;
  m_numStepCruise = stepsCruise;
  m_numStepDecel = stepsDecel;
  m_targetDirection = direction;
  m_isAccel = isAccel;
  m_numWaitPulse = (float)accelStartWaitPulse;
  m_minWaitPulse = minWaitPulse;

  m_totalPulse = 0;
  m_pulseCount = 0;
  m_stepCountAccel = 0;
  m_stepCountCruise = 0;
  m_stepCountDecel = 0;
  Serial.print(" direction:");
  Serial.print(m_targetDirection);
  Serial.print(" m_numWaitPulse:");
  Serial.print(m_numWaitPulse);
  Serial.print(" m_numStepAccel:");
  Serial.print(m_numStepAccel);
  Serial.print(" m_numStepCruise:");
  Serial.print(m_numStepCruise);
  Serial.print(" m_numStepDecel:");
  Serial.println(m_numStepDecel);
  changeStateControl(m_isAccel? MOTOR_EXECUTE_INCREASE_SPEED : MOTOR_EXECUTE_CRUISE_SPEED);
  m_enableLogPulse = false;
}
// #define DEBUG_COUNT_STEP
float SmoothMotion::delayAccel(float stepCount, float delayCur) {
  // Serial.print("d[");
  // Serial.print(delayCur);
  // Serial.print("]");
  // Serial.print(" s[");
  // Serial.print(stepCount);
  // Serial.println("] T:");
#ifndef DEBUG_COUNT_STEP
  float nextDelay = delayCur * (4.0f*stepCount - 1.0f) / (4.0f*stepCount + 1.0f);
  // nextDelay = delayCur-1;
  return nextDelay > m_minWaitPulse ? nextDelay: m_minWaitPulse;
#else
  return delayCur;
#endif
}

float SmoothMotion::delayDecel(float stepCount, float delayCur) {
  // Serial.print("d[");
  // Serial.print(delayCur);
  // Serial.print("]");
  // Serial.print(" s[");
  // Serial.print(stepCount);
  // Serial.println("] T:");
#ifndef DEBUG_COUNT_STEP
  float nextDelay = delayCur * (4.0f*stepCount + 1.0f) / (4.0f*stepCount - 1.0f);
  // nextDelay = delayCur+1;
  return nextDelay;
#else
  return delayCur;
#endif
}

void SmoothMotion::changeStateControl(int newState)
{
  if(m_stateControl != newState) {
    m_stateControl = newState;
    if(m_stateControl != MOTOR_EXECUTE_DONE) {
      m_statePulse = STATE_COMMAND1;
    }
    // switch(m_stateControl){
    //   case MOTOR_EXECUTE_INCREASE_SPEED: Serial.println("Accel");
    //   break;
    //   case MOTOR_EXECUTE_CRUISE_SPEED: Serial.println("Cruise");
    //   break;
    //   case MOTOR_EXECUTE_DECREASE_SPEED: Serial.println("Decel");
    //   break;
    // }
  }
}

void SmoothMotion::motionControlLoop() {
  switch (m_stateControl) {
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
      Serial.print("Accel: ");
      Serial.print(m_stepCountAccel);
      Serial.print(" Cruise: ");
      Serial.print(m_stepCountCruise);
      Serial.print(" Decel: ");
      Serial.println(m_stepCountDecel);
      m_stateControl = MOTOR_EXECUTE_WAIT_COMMAND;
    }
    break;
  }
}

void SmoothMotion::increaseSpeed() {
  pulseLoop();
  if(m_statePulse != STATE_DONE) return;
  m_statePulse = STATE_COMMAND1;
  m_stepCountAccel ++;
  m_numWaitPulse = delayAccel(m_stepCountAccel,m_numWaitPulse);
#ifdef DEBUG_ACCEL
  Serial.print(" Accel cur: ");
  Serial.print(m_stepCountAccel);
  Serial.print("/");
  Serial.println(m_numStepAccel);
  if(m_stepCountAccel >= 29) m_enableLogPulse = true;
#endif
  if(m_stepCountAccel >= m_numStepAccel){
    // Serial.print(" ID: ");
    // Serial.print(m_id);
    // Serial.print(" Accel: ");
    // Serial.print(m_stepCountAccel);
    // Serial.print(" totalPulse: ");
    // Serial.println(m_totalPulse);
    changeStateControl(MOTOR_EXECUTE_CRUISE_SPEED);
  }
}

void SmoothMotion::cruiseSpeed() {
  pulseLoop();
  if(m_statePulse != STATE_DONE) return;
  m_statePulse = STATE_COMMAND1;
  m_stepCountCruise ++;
  if(m_stepCountCruise >= m_numStepCruise){
    // Serial.print(" ID: ");
    // Serial.print(m_id);
    // Serial.print(" Cruise: ");
    // Serial.print(m_stepCountCruise);
    // Serial.print(" totalPulse: ");
    // Serial.println(m_totalPulse);
    changeStateControl(m_isAccel? MOTOR_EXECUTE_DECREASE_SPEED:MOTOR_EXECUTE_DONE);
  }
}

void SmoothMotion::decreaseSpeed() {
  pulseLoop();
  if(m_statePulse != STATE_DONE) return;
  m_statePulse = STATE_COMMAND1;
  m_stepCountDecel ++;
  m_numWaitPulse = delayDecel(m_numStepDecel - m_stepCountDecel,m_numWaitPulse);  
  if(m_stepCountDecel >= m_numStepDecel) {
    // Serial.print(" ID: ");
    // Serial.print(m_id);
    // Serial.print(" Decel: ");
    // Serial.print(m_stepCountDecel);
    // Serial.print(" totalPulse: ");
    // Serial.println(m_totalPulse);
    changeStateControl(MOTOR_EXECUTE_DONE);
  }
}

void SmoothMotion::goHome()
{

}
// #define DEBUG_PULSE
void SmoothMotion::pulseLoop()
{
  m_totalPulse ++;
  // Serial.print("pulseLoop state:");
  // Serial.print(m_statePulse);
  // Serial.print(" P-");
  // Serial.println(m_totalPulse);
  if(m_motorType == MOTOR_TYPE_STEPPER_2_WIRES) {
    switch(m_statePulse){
      case STATE_COMMAND1: {
        m_pulseCount = 0;
        digitalWrite(m_stepPin1,HIGH);
        m_statePulse = STATE_WAIT1;
#ifdef DEBUG_PULSE
        if(m_enableLogPulse) {  
          Serial.print("PULSE HIGH: ");
          Serial.println(m_pulseCount);
        }
#endif
      }
      break;
      case STATE_WAIT1: {
        m_pulseCount++;
#ifdef DEBUG_PULSE
        if(m_enableLogPulse) {        
          Serial.print("PULSE STATE_WAIT1: ");
          Serial.print(m_pulseCount);
          Serial.print("/");
          Serial.println(m_numWaitPulse);
        }
#endif
        if(m_pulseCount >= (uint32_t)(m_numWaitPulse-1)) m_statePulse = STATE_COMMAND2;
      }
      break;
      case STATE_COMMAND2: {
        digitalWrite(m_stepPin1,LOW);
        m_pulseCount = 0;
#ifdef DEBUG_PULSE
        if(m_enableLogPulse) {
          Serial.print("PULSE LOW: ");
          Serial.println(m_pulseCount);
        }
#endif
        m_statePulse = STATE_WAIT2;
      }
      break;
      case STATE_WAIT2: {
        m_pulseCount++;
#ifdef DEBUG_PULSE
        if(m_enableLogPulse) {
          Serial.print("PULSE STATE_WAIT2: ");
          Serial.print(m_pulseCount);
          Serial.print("/");
          Serial.println(m_numWaitPulse);
        }
#endif        
        if(m_pulseCount >= (uint32_t)(m_numWaitPulse-1)) m_statePulse = STATE_DONE;
      }
      break;
    }
    
  } else if(m_motorType == MOTOR_TYPE_STEPPER_4_WIRES) {
    switch(m_statePulse){
      case STATE_COMMAND1: {
        digitalWrite(m_stepPin1, m_targetDirection < 0 ? HIGH:HIGH);
        digitalWrite(m_stepPin2, m_targetDirection < 0 ? LOW :LOW );
        digitalWrite(m_stepPin3, m_targetDirection < 0 ? LOW :LOW );
        digitalWrite(m_stepPin4, m_targetDirection < 0 ? LOW :HIGH);
        m_statePulse = STATE_WAIT1;
        m_pulseCount = 0;
      }
      break;
      case STATE_WAIT1: {
        m_pulseCount++;
        if(m_pulseCount >= (uint32_t)(m_numWaitPulse-1)) m_statePulse = STATE_COMMAND2;
      }
      break;
      case STATE_COMMAND2: {
        digitalWrite(m_stepPin1, m_targetDirection < 0 ? HIGH:LOW );
        digitalWrite(m_stepPin2, m_targetDirection < 0 ? HIGH:LOW );
        digitalWrite(m_stepPin3, m_targetDirection < 0 ? LOW :LOW );
        digitalWrite(m_stepPin4, m_targetDirection < 0 ? LOW :HIGH);
        m_statePulse = STATE_WAIT2;
        m_pulseCount = 0;
      }
      break;
      case STATE_WAIT2: {
        m_pulseCount++;
        if(m_pulseCount >= (uint32_t)(m_numWaitPulse-1)) m_statePulse = STATE_COMMAND3;
      }
      break;
      case STATE_COMMAND3: {
        digitalWrite(m_stepPin1, m_targetDirection < 0 ? LOW :LOW );
        digitalWrite(m_stepPin2, m_targetDirection < 0 ? HIGH:LOW );
        digitalWrite(m_stepPin3, m_targetDirection < 0 ? LOW :HIGH);
        digitalWrite(m_stepPin4, m_targetDirection < 0 ? LOW :HIGH);
        m_statePulse = STATE_WAIT3;
        m_pulseCount = 0;
      }
      break;
      case STATE_WAIT3: {
        m_pulseCount++;
        if(m_pulseCount >= (uint32_t)(m_numWaitPulse-1)) m_statePulse = STATE_COMMAND4;
      }
      break;
      case STATE_COMMAND4: {
        digitalWrite(m_stepPin1, m_targetDirection < 0 ? LOW :LOW );
        digitalWrite(m_stepPin2, m_targetDirection < 0 ? HIGH:LOW );
        digitalWrite(m_stepPin3, m_targetDirection < 0 ? HIGH:HIGH);
        digitalWrite(m_stepPin4, m_targetDirection < 0 ? LOW :LOW );
        m_statePulse = STATE_WAIT4;
        m_pulseCount = 0;
      }
      break;
      case STATE_WAIT4: {
        m_pulseCount++;
        if(m_pulseCount >= (uint32_t)(m_numWaitPulse-1)) m_statePulse = STATE_COMMAND5;
      }
      break;
      case STATE_COMMAND5: {
        digitalWrite(m_stepPin1, m_targetDirection < 0 ? LOW :LOW );
        digitalWrite(m_stepPin2, m_targetDirection < 0 ? LOW :HIGH);
        digitalWrite(m_stepPin3, m_targetDirection < 0 ? HIGH:HIGH);
        digitalWrite(m_stepPin4, m_targetDirection < 0 ? LOW :LOW );
        m_statePulse = STATE_WAIT5;
        m_pulseCount = 0;
      }
      break;
      case STATE_WAIT5: {
        m_pulseCount++;
        if(m_pulseCount >= (uint32_t)(m_numWaitPulse-1)) m_statePulse = STATE_COMMAND6;
      }
      break;
      case STATE_COMMAND6: {
        digitalWrite(m_stepPin1, m_targetDirection < 0 ? LOW :LOW );
        digitalWrite(m_stepPin2, m_targetDirection < 0 ? LOW :HIGH);
        digitalWrite(m_stepPin3, m_targetDirection < 0 ? HIGH:LOW );
        digitalWrite(m_stepPin4, m_targetDirection < 0 ? HIGH:LOW );
        m_statePulse = STATE_WAIT6;
        m_pulseCount = 0;
      }
      break;
      case STATE_WAIT6: {
        m_pulseCount++;
        if(m_pulseCount >= (uint32_t)(m_numWaitPulse-1)) m_statePulse = STATE_COMMAND7;
      }
      break;
      case STATE_COMMAND7: {
        digitalWrite(m_stepPin1, m_targetDirection < 0 ? LOW :HIGH);
        digitalWrite(m_stepPin2, m_targetDirection < 0 ? LOW :HIGH);
        digitalWrite(m_stepPin3, m_targetDirection < 0 ? LOW :LOW );
        digitalWrite(m_stepPin4, m_targetDirection < 0 ? HIGH:LOW );
        m_statePulse = STATE_WAIT7;
        m_pulseCount = 0;
      }
      break;
      case STATE_WAIT7: {
        m_pulseCount++;
        if(m_pulseCount >= (uint32_t)(m_numWaitPulse-1)) m_statePulse = STATE_COMMAND8;
      }
      break;
      case STATE_COMMAND8: {
        digitalWrite(m_stepPin1, m_targetDirection < 0 ? HIGH:HIGH);
        digitalWrite(m_stepPin2, m_targetDirection < 0 ? LOW :LOW );
        digitalWrite(m_stepPin3, m_targetDirection < 0 ? LOW :LOW );
        digitalWrite(m_stepPin4, m_targetDirection < 0 ? HIGH:LOW );
        m_statePulse = STATE_WAIT8;
        m_pulseCount = 0;
      }
      break;
      case STATE_WAIT8: {
        m_pulseCount++;
        if(m_pulseCount >= (uint32_t)(m_numWaitPulse-1)) m_statePulse = STATE_DONE;
      }
      break;
    }
  } else {
    m_statePulse = STATE_DONE;
  }
}

uint32_t SmoothMotion::getCurrentSteps()
{
  return m_stepCountAccel + m_stepCountCruise + m_stepCountDecel;
}