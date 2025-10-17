#include "Constants.h"

const char* SSID = "hex";
const char* PASSWORD = "krolika123";

const int batteryPin = 33;

const int FEMUR_STANCE_ROTATE = 18000;
const int TIBIA_STANCE_ROTATE = 6000;
const int OVERLAP_DELAY = 50;

const int32_t COXA_FORWARD  = 13500;
const int32_t COXA_BACKWARD = 11500;
const int32_t COXA_DEFAULT  = 12000;

const int32_t FEMUR_UP   = 18500;
const int32_t FEMUR_DOWN = 17000;

const int32_t TIBIA_UP   = 10000;
const int32_t TIBIA_DOWN = 7000;

const int TRIPOD1_LEGS[3] = {0, 12, 6};
const int TRIPOD2_LEGS[3] = {15, 3, 9};

const int MOVE_TIME   = 180;  
const int LIFT_TIME   = 120;  
const int PUSH_TIME   = 180;  
const int LOWER_TIME  = 140;  

// SLOW MOVE TIMES
// const int MOVE_TIME   = 500;  
// const int LIFT_TIME   = 400;
// const int PUSH_TIME   = 500; 
// const int LOWER_TIME  = 450; 

const int SHORT_DELAY = 20; 

const int WAVE_ORDER[] = {0, 3, 6, 9, 12, 15}; 
const int BODY_PUSH_DELTA = 800;

const int FAST_LIFT_TIME  = 100;
const int FAST_MOVE_TIME  = 120;
const int FAST_LOWER_TIME = 100;
const int FAST_PUSH_TIME  = 100;
const int FAST_DELAY      = 30;

const int32_t LEFT_SIDE_COXA_OFFSET  = 200;   // Offset for left side legs (0, 9, 12) - increased to counter right drift
const int32_t RIGHT_SIDE_COXA_OFFSET = -75;   // Offset for right side legs (3, 6, 15) - reduced to counter right drift 


// Rotation constants - more extreme positions for effective rotation
const int32_t COXA_ROTATE_FORWARD  = 15000;  // More extreme forward for rotation
const int32_t COXA_ROTATE_BACKWARD = 9000;   // More extreme backward for rotation
