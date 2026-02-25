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
  // For 2 wires stepper
  STATE_HIGH,
  STATE_LOW,

  // For 4 wires stepper
  STATE_COMMAND1,
  STATE_COMMAND2,
  STATE_COMMAND3,
  STATE_COMMAND4,
  STATE_COMMAND5,
  STATE_COMMAND6,
  STATE_COMMAND7,
  STATE_COMMAND8,

  // Wait state
  STATE_WAIT1,
  STATE_WAIT2,
  STATE_WAIT3,
  STATE_WAIT4,
  STATE_WAIT5,
  STATE_WAIT6,
  STATE_WAIT7,
  STATE_WAIT8,

  // Done state
  STATE_DONE,
} STATE_PULSE;

// typedef enum {
//   MOTION_INIT,
//   MOTION_PULSE,
//   MOTION_UPDATE_DELAY,
//   MOTION_CHECK_COUNTER,
//   MOTION_DONE
// } MOTION_STATE;

typedef enum {
  MOTOR_TYPE_STEPPER_2_WIRES,
  MOTOR_TYPE_STEPPER_4_WIRES,
} MOTOR_TYPE;
class SmoothMotion {
  public:
    SmoothMotion(uint32_t id, int enablePin, int dirPin, int stepPin);
    SmoothMotion(uint32_t id, int stepPin1, int stepPin2, int stepPin3, int stepPin4);

    void init();
    void setupTarget(int stepsAccel, int stepsCruise, int stepsDecel, 
      int direction, bool isAccel, uint32_t accelStartWaitPulse, uint32_t minWaitPulse);
    void motionControlLoop();
    void pulseLoop();
    void increaseSpeed();
    void cruiseSpeed();
    void decreaseSpeed();
    void goHome();
    float delayAccel(float stepCount, float delayCur);
    float delayDecel(float stepCount, float delayCur);
    uint32_t getCurrentSteps();
    void changeStateControl(int newState);
    uint32_t m_totalPulse;
    bool m_enableLogPulse;

  private:
    uint32_t m_id;
    uint32_t m_minWaitPulse;
    uint8_t m_statePulse;
    uint32_t m_numWaitPulse;
    uint32_t m_pulseCount;

    uint8_t m_stateControl;
    
    uint32_t m_numStepAccel;
    uint32_t m_numStepCruise;
    uint32_t m_numStepDecel;

    uint32_t m_stepCountAccel;
    uint32_t m_stepCountCruise;
    uint32_t m_stepCountDecel;

    // uint8_t m_motionAccelState;
    // uint8_t m_motionCruiseState;    
    // uint8_t m_motionDecelState;
    
    int m_targetStep;
    int m_targetDirection;
    bool m_isAccel;

    // pin numbers
    MOTOR_TYPE m_motorType;
    int m_enablePin;
    int m_dirPin;
    int m_stepPin1;
    int m_stepPin2;
    int m_stepPin3;
    int m_stepPin4;
};

#endif // SMOOTHMOTION_H