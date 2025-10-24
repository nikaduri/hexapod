#include "Constants.h"

const char* SSID = "hex";
const char* PASSWORD = "krolika123";

const int batteryPin = 33;

const int OVERLAP_DELAY = 50;

const int32_t COXA_FORWARD  = 11500;
const int32_t COXA_BACKWARD = 13500;
const int32_t COXA_DEFAULT  = 12000;

const int32_t FEMUR_UP   = 17200;
const int32_t FEMUR_DOWN = 12000;

const int32_t TIBIA_UP   = 12500;
const int32_t TIBIA_DOWN = 12000;

const int FEMUR_STANCE_ROTATE = 18600;
const int TIBIA_STANCE_ROTATE = 4300;

const int TRIPOD1_LEGS[3] = {0, 12, 6};
const int TRIPOD2_LEGS[3] = {15, 3, 9};

const int SWITCH_PINS[] = {25, 27, 14, 26, 35, 12};

const int MOVE_TIME   = 270;  
const int LIFT_TIME   = 140;  
const int PUSH_TIME   = 270;
const int LOWER_TIME  = 180;   

const int SHORT_DELAY = 20; 

const int WAVE_ORDER[] = {0, 3, 6, 9, 12, 15}; 
const int BODY_PUSH_DELTA = 800;

const int FAST_LIFT_TIME  = 100;
const int FAST_MOVE_TIME  = 120;
const int FAST_LOWER_TIME = 100;
const int FAST_PUSH_TIME  = 100;
const int FAST_DELAY      = 30;

const int32_t LEFT_SIDE_COXA_OFFSET  = 200;
const int32_t RIGHT_SIDE_COXA_OFFSET = -75;


const int32_t COXA_ROTATE_FORWARD  = 15000;
const int32_t COXA_ROTATE_BACKWARD = 9000;

const int ROTATE_MOVE_TIME  = 350;
const int ROTATE_LIFT_TIME  = 240;
const int ROTATE_LOWER_TIME = 260;
