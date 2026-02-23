#ifndef SMOOTHMOTION_H
#define SMOOTHMOTION_H
#include <Arduino.h>
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

typedef enum {
  MOTION_INIT,
  MOTION_PULSE,
  MOTION_UPDATE_DELAY,
  MOTION_CHECK_COUNTER,
  MOTION_DONE
} MOTION_STATE;
class SmoothMotion {
  public:
    SmoothMotion();
    void init();
    void motionControlLoop();
    void pulseLoop();
    void increaseSpeed();
    void cruiseSpeed();
    void decreaseSpeed();
    void goHome();
    float delayAccel(float stepCount, float delayCur);
    float delayDecel(float stepCount, float delayCur);
  private:
    uint8_t statePulse;
    uint8_t numWaitPulse = 2;
    uint8_t pulseCount = 0;

    float d0 = 200;
    float delayTime = d0;
    uint8_t stateControl;
    
    uint8_t motionAccelState;
    float stepCountAccel = 0;
    uint8_t motionCruiseState;
    uint32_t stepCountCruise = 0;
    uint8_t motionDecelState;
    float stepCountDecel;

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
};

#endif // SMOOTHMOTION_H