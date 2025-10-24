#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Arduino.h>
#include <lx16a-servo.h>


extern const char* SSID;
extern const char* PASSWORD;

extern const int batteryPin;

extern const int32_t COXA_FORWARD;
extern const int32_t COXA_BACKWARD;
extern const int32_t COXA_DEFAULT;


extern const int32_t FEMUR_UP;
extern const int32_t FEMUR_DOWN;

extern const int32_t TIBIA_UP;
extern const int32_t TIBIA_DOWN;

extern const int TRIPOD1_LEGS[3];
extern const int TRIPOD2_LEGS[3];

extern const int SWITCH_PINS[6];

extern const int MOVE_TIME;
extern const int LIFT_TIME;
extern const int PUSH_TIME;
extern const int LOWER_TIME;
extern const int SHORT_DELAY;


extern const int WAVE_ORDER[]; 
extern const int BODY_PUSH_DELTA;

extern const int FAST_LIFT_TIME;
extern const int FAST_MOVE_TIME;
extern const int FAST_LOWER_TIME;
extern const int FAST_PUSH_TIME;
extern const int FAST_DELAY;

extern const int FEMUR_STANCE_ROTATE;
extern const int TIBIA_STANCE_ROTATE;
extern const int OVERLAP_DELAY;

extern const int32_t LEFT_SIDE_COXA_OFFSET;
extern const int32_t RIGHT_SIDE_COXA_OFFSET;

extern const int32_t COXA_ROTATE_FORWARD;
extern const int32_t COXA_ROTATE_BACKWARD;

extern const int ROTATE_MOVE_TIME;
extern const int ROTATE_LIFT_TIME;
extern const int ROTATE_LOWER_TIME;

#endif
