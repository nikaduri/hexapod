// Constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Arduino.h>
#include <lx16a-servo.h>


// WiFi credentials
extern const char* SSID;
extern const char* PASSWORD;

// Gait constants
extern const int32_t COXA_FORWARD;
extern const int32_t COXA_BACKWARD;
extern const int32_t COXA_DEFAULT;

extern const int32_t FEMUR_UP;
extern const int32_t FEMUR_DOWN;

extern const int32_t TIBIA_UP;
extern const int32_t TIBIA_DOWN;

extern const int TRIPOD1_LEGS[3];
extern const int TRIPOD2_LEGS[3];

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

#endif
