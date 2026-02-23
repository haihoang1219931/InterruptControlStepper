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

typedef enum {
    MOTOR_EXECUTE_WAIT_COMMAND,
    MOTOR_EXECUTE_INCREASE_SPEED,
    MOTOR_EXECUTE_CRUISE_SPEED,
    MOTOR_EXECUTE_DECREASE_SPEED,
    MOTOR_EXECUTE_HOME,
    MOTOR_EXECUTE_DONE,
} MOTOR_CONTROL_STATE;

typedef enum {
  STATE_HIGH,
  STATE_WAIT1,
  STATE_LOW,
  STATE_WAIT2,
  STATE_DONE,
} STATE_PULSE;

uint8_t statePulse = STATE_HIGH;
uint8_t numWaitPulse = 2;
uint8_t pulseCount = 0;

float d0 = 200;
float delayTime = d0;
uint8_t stateControl;
float delayAccel(float stepCount, float delayCur) {
  // Serial.print("d[");
  // Serial.print(delayCur);
  // Serial.print("]");
  // Serial.print(" s[");
  // Serial.print(stepCount);
  // Serial.print("] T:");
  return delayCur * (4.0f*stepCount - 1.0f) / (4.0f*stepCount + 1.0f);
}

float delayDecel(float stepCount, float delayCur) {
  // Serial.print("d[");
  // Serial.print(delayCur);
  // Serial.print("]");
  // Serial.print(" s[");
  // Serial.print(stepCount);
  // Serial.print("] T:");
  return delayCur * (4.0f*stepCount + 1.0f) / (4.0f*stepCount - 1.0f);
}
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


// #define RUN_FAST
// timer 1 interrupt
void motionControlLoop();
void pulseLoop();
ISR(TIMER1_COMPA_vect)
{
  motionControlLoop();
}
void increaseSpeed();
void cruiseSpeed();
void decreaseSpeed();
void goHome();
void motionControlLoop() {
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


typedef enum {
  MOTION_INIT,
  MOTION_PULSE,
  MOTION_UPDATE_DELAY,
  MOTION_CHECK_COUNTER,
  MOTION_DONE
} MOTION_STATE;
uint8_t motionAccelState = MOTION_INIT;
float stepCountAccel = 0;
void increaseSpeed() {
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
uint8_t motionCruiseState = MOTION_INIT;
uint32_t stepCountCruise = 0;
void cruiseSpeed() {
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
uint8_t motionDecelState = MOTION_INIT;
float stepCountDecel;
void decreaseSpeed() {
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

void goHome()
{

}
void pulseLoop()
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

void loop()
{
  // Serial.println(delayTime);
  // digitalWrite(stepPinZ, HIGH);
  // delayMicroseconds((int)60);
  // digitalWrite(stepPinZ, LOW);
  // delayMicroseconds((int)60);
}