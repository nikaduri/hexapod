#include "ICM_20948.h"
#include <Arduino.h>
#include <lx16a-servo.h>

#define WIRE_PORT Wire
#define AD0_VAL 0

ICM_20948_I2C myICM;
LX16ABus servoBus;
LX16AServo* servos[18];

// Joint positions (0–24000 range)
const int32_t COXA_DEFAULT  = 12000;
const int32_t COXA_FORWARD  = 15000;
const int32_t COXA_BACKWARD = 9000;

const int32_t FEMUR_DOWN    = 22000;
const int32_t FEMUR_UP      = 13000;   // Higher lift for carpet
const int32_t TIBIA_DOWN    = 3000;
const int32_t TIBIA_UP      = 16000;   // Higher knee bend for clearance

const int TRIPOD1_LEGS[] = {15, 12, 6};  // Front Left, Middle Right, Back Left
const int TRIPOD2_LEGS[] = {0, 3, 9};    // Front Right, Middle Left, Back Right

void setup() {
    Serial.begin(115200);
    while (!Serial) {}

    WIRE_PORT.begin();
    WIRE_PORT.setClock(400000);

    servoBus.beginOnePinMode(&Serial2, 15);  // Only RX used
    servoBus.debug(false);
    Serial.println("Hexapod Starting...");

    for (int i = 0; i < 18; i++) {
        servos[i] = new LX16AServo(&servoBus, i + 1);
    }

    for (int i = 0; i < 18; i += 3) {
        servos[i]->move_time(COXA_DEFAULT, 500);
        servos[i+1]->move_time(FEMUR_DOWN, 500);
        servos[i+2]->move_time(TIBIA_DOWN, 500);
    }

    delay(1000);
}

bool isRightSide(int base) {
    return base >= 9;
}

void moveLeg(int base, int32_t coxa, int32_t femur, int32_t tibia, int time = 300) {
    servos[base]->move_time(coxa, time);
    servos[base+1]->move_time(femur, time);
    servos[base+2]->move_time(tibia, time);
}

void liftLegForward(int base, int time = 300) {
    int32_t coxa_angle = isRightSide(base) ? COXA_BACKWARD : COXA_FORWARD;
    moveLeg(base, coxa_angle, FEMUR_UP, TIBIA_UP, time);
}

void lowerLegToGround(int base, int time = 300) {
    int32_t coxa_angle = isRightSide(base) ? COXA_BACKWARD : COXA_FORWARD;
    moveLeg(base, coxa_angle, FEMUR_DOWN, TIBIA_DOWN, time);
}

void pushBackLeg(int base, int time = 300) {
    int32_t coxa_angle = isRightSide(base) ? COXA_FORWARD : COXA_BACKWARD;
    moveLeg(base, coxa_angle, FEMUR_DOWN, TIBIA_DOWN, time);
}

void returnToNeutral(int base, int time = 300) {
    moveLeg(base, COXA_DEFAULT, FEMUR_DOWN, TIBIA_DOWN, time);
}

void loop() {
    Serial.println("Phase 1: Tripod 1 swings, Tripod 2 pushes");

    for (int i = 0; i < 3; i++) {
        liftLegForward(TRIPOD1_LEGS[i]);
    }

    for (int i = 0; i < 3; i++) {
        pushBackLeg(TRIPOD2_LEGS[i]);
    }

    delay(500);

    for (int i = 0; i < 3; i++) {
        lowerLegToGround(TRIPOD1_LEGS[i]);
    }

    delay(300);

    Serial.println("Phase 2: Tripod 2 swings, Tripod 1 pushes");

    for (int i = 0; i < 3; i++) {
        liftLegForward(TRIPOD2_LEGS[i]);
    }

    for (int i = 0; i < 3; i++) {
        pushBackLeg(TRIPOD1_LEGS[i]);
    }

    delay(500);

    for (int i = 0; i < 3; i++) {
        lowerLegToGround(TRIPOD2_LEGS[i]);
    }

    delay(300);

    for (int i = 0; i < 18; i += 3) {
        returnToNeutral(i);
    }

    delay(400);
}
