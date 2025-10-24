#include "Constants.h"

// 25 - 0 adc
// 27 - 3 adc
// 14 - 6 adc
// 26 - 9 adc
// 19 - 12 
// 23 - 15

const char* SSID = "hex";
const char* PASSWORD = "krolika123";

const int batteryPin = 33;

const int OVERLAP_DELAY = 50;

const int32_t COXA_FORWARD  = 11500;
const int32_t COXA_BACKWARD = 13500;
const int32_t COXA_DEFAULT  = 12000;

const int32_t FEMUR_UP   = 17200;  // Much lower for much higher leg lift when walking (was 18000)
const int32_t FEMUR_DOWN = 12000; // Increased for higher stance (was 18200)

const int32_t TIBIA_UP   = 12500;  // Much higher for extreme leg tuck when walking (was 11000)
const int32_t TIBIA_DOWN = 12000;   // Decreased for higher stance (was 5000)

const int FEMUR_STANCE_ROTATE = 18600;  // Match FEMUR_DOWN for proper stance
const int TIBIA_STANCE_ROTATE = 4300;   // Match TIBIA_DOWN to keep all legs on ground

const int TRIPOD1_LEGS[3] = {0, 12, 6};
const int TRIPOD2_LEGS[3] = {15, 3, 9};

// 0 - 25
// 3 - 27
// 6 - 14
// 9 - 26
// 12 - 35
// 15 - 12

const int SWITCH_PINS[] = {25, 27, 14, 26, 35, 12};

const int MOVE_TIME   = 270;  
const int LIFT_TIME   = 140;  
const int PUSH_TIME   = 270;
const int LOWER_TIME  = 180;   

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

// Rotation timing - slower for smoother rotation
const int ROTATE_MOVE_TIME  = 350;  // Slower than walking (was 280ms)
const int ROTATE_LIFT_TIME  = 240;  // Slower than walking (was 190ms)
const int ROTATE_LOWER_TIME = 260;  // Slower than walking (was 210ms)
