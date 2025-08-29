#include "Constants.h"

const char* SSID = "hex";
const char* PASSWORD = "krolika123";

const int batteryPin = 33;

const int FEMUR_STANCE_ROTATE = 18000; // Crouches the robot lower
const int TIBIA_STANCE_ROTATE = 6000;  // Pushes the feet further out
const int OVERLAP_DELAY = 50;

const int32_t COXA_FORWARD  = 13500;
const int32_t COXA_BACKWARD = 11500;
const int32_t COXA_DEFAULT  = 12000;

const int32_t FEMUR_UP   = 18500;
const int32_t FEMUR_DOWN = 17000;

const int32_t TIBIA_UP   = 10000;
const int32_t TIBIA_DOWN = 7000;

const int TRIPOD1_LEGS[3] = {15, 12, 6};
const int TRIPOD2_LEGS[3] = {0, 3, 9};

const int MOVE_TIME   = 180;  
const int LIFT_TIME   = 120;  
const int PUSH_TIME   = 180;  
const int LOWER_TIME  = 140;  

const int SHORT_DELAY = 20; 

const int WAVE_ORDER[] = {15, 3, 6, 9, 12, 0}; 
const int BODY_PUSH_DELTA = 800;

const int FAST_LIFT_TIME  = 100;
const int FAST_MOVE_TIME  = 120;
const int FAST_LOWER_TIME = 100;
const int FAST_PUSH_TIME  = 100;
const int FAST_DELAY      = 30;
