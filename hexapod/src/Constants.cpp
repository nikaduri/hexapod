// Constants.cpp
#include "Constants.h"

const char* SSID = "hex";
const char* PASSWORD = "krolika123";

const int batteryPin = 2;

const int32_t COXA_FORWARD  = 14500;
const int32_t COXA_BACKWARD = 9500;
const int32_t COXA_DEFAULT  = 12000;

const int32_t FEMUR_UP   = 18500;
const int32_t FEMUR_DOWN = 17000;

const int32_t TIBIA_UP   = 10000;
const int32_t TIBIA_DOWN = 7000;

const int TRIPOD1_LEGS[3] = {15, 12, 6};
const int TRIPOD2_LEGS[3] = {0, 3, 9};

const int MOVE_TIME   = 180;  // Reduced from 300ms for faster swing
const int LIFT_TIME   = 120;  // Reduced from 150ms for faster lift
const int PUSH_TIME   = 180;  // Match MOVE_TIME for symmetric gait
const int LOWER_TIME  = 140;  // Reduced from 250ms for faster lower

const int SHORT_DELAY = 20;   // Reduced from 50ms for less waiting

const int WAVE_ORDER[] = {15, 3, 6, 9, 12, 0}; 
const int BODY_PUSH_DELTA = 800;

const int FAST_LIFT_TIME  = 100;
const int FAST_MOVE_TIME  = 120;
const int FAST_LOWER_TIME = 100;
const int FAST_PUSH_TIME  = 100;
const int FAST_DELAY      = 30;
