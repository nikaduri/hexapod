#include "ICM_20948.h" 
#include <Arduino.h>
#include <lx16a-servo.h>

#define WIRE_PORT Wire
#define AD0_VAL 0

ICM_20948_I2C myICM;
LX16ABus servoBus;
LX16AServo* servos[18];

// Joint positions
const int32_t COXA_DEFAULT  = 12000;
const int32_t COXA_FORWARD  = 13500;
const int32_t COXA_BACKWARD = 10500;

const int32_t FEMUR_DOWN    = 17000;
const int32_t FEMUR_UP      = 22000;

const int32_t TIBIA_DOWN    = 6000;
const int32_t TIBIA_UP      = 12000;

const int TRIPOD1_LEGS[] = {15, 12, 6};
const int TRIPOD2_LEGS[] = {0, 3, 9};

// Timing constants (smaller is faster)
const int MOVE_TIME  = 400;
const int LIFT_TIME  = 200;
const int PUSH_TIME  = 450;
const int LOWER_TIME = 350;

const int SHORT_DELAY = 100;


void setup() {
    Serial.begin(115200);
    while (!Serial) {}

    WIRE_PORT.begin();
    WIRE_PORT.setClock(400000);

    servoBus.beginOnePinMode(&Serial2, 15);
    servoBus.debug(false);

    Serial.println("Hexapod Starting...");

    for (int i = 0; i < 18; i++) {
        servos[i] = new LX16AServo(&servoBus, i + 1);
    }

    for (int i = 0; i < 18; i += 3) {
        servos[i]->move_time(COXA_DEFAULT, 1500);
        servos[i+1]->move_time(FEMUR_DOWN, 1500);
        servos[i+2]->move_time(TIBIA_DOWN, 1500);
    }

    delay(1500);
    Serial.println("Ready to walk!");
}

bool isRightSide(int base) {
    return base == 0 || base == 9 || base == 12;
}

void moveLeg(int base, int32_t coxa, int32_t femur, int32_t tibia, int time = MOVE_TIME) {
    servos[base]->move_time(coxa, time);
    servos[base+1]->move_time(femur, time);
    servos[base+2]->move_time(tibia, time);
}

void loop() {
    // PHASE 1
    for (int i = 0; i < 3; i++) {
        int32_t stance_coxa = isRightSide(TRIPOD2_LEGS[i]) ? COXA_FORWARD : COXA_BACKWARD;
        moveLeg(TRIPOD2_LEGS[i], stance_coxa, FEMUR_DOWN, TIBIA_DOWN, MOVE_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    for (int i = 0; i < 3; i++) {
        moveLeg(TRIPOD1_LEGS[i], servos[TRIPOD1_LEGS[i]]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    for (int i = 0; i < 3; i++) {
        int32_t fwd_coxa = isRightSide(TRIPOD1_LEGS[i]) ? COXA_FORWARD : COXA_BACKWARD;
        int32_t back_coxa = isRightSide(TRIPOD2_LEGS[i]) ? COXA_BACKWARD : COXA_FORWARD;

        moveLeg(TRIPOD1_LEGS[i], fwd_coxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
        moveLeg(TRIPOD2_LEGS[i], back_coxa, FEMUR_DOWN, TIBIA_DOWN, PUSH_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    for (int i = 0; i < 3; i++) {
        int32_t fwd_coxa = isRightSide(TRIPOD1_LEGS[i]) ? COXA_FORWARD : COXA_BACKWARD;
        moveLeg(TRIPOD1_LEGS[i], fwd_coxa, FEMUR_DOWN, TIBIA_DOWN, LOWER_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // PHASE 2
    for (int i = 0; i < 3; i++) {
        int32_t stance_coxa = isRightSide(TRIPOD1_LEGS[i]) ? COXA_FORWARD : COXA_BACKWARD;
        moveLeg(TRIPOD1_LEGS[i], stance_coxa, FEMUR_DOWN, TIBIA_DOWN, MOVE_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    for (int i = 0; i < 3; i++) {
        moveLeg(TRIPOD2_LEGS[i], servos[TRIPOD2_LEGS[i]]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    for (int i = 0; i < 3; i++) {
        int32_t fwd_coxa = isRightSide(TRIPOD2_LEGS[i]) ? COXA_FORWARD : COXA_BACKWARD;
        int32_t back_coxa = isRightSide(TRIPOD1_LEGS[i]) ? COXA_BACKWARD : COXA_FORWARD;

        moveLeg(TRIPOD2_LEGS[i], fwd_coxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
        moveLeg(TRIPOD1_LEGS[i], back_coxa, FEMUR_DOWN, TIBIA_DOWN, PUSH_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    for (int i = 0; i < 3; i++) {
        int32_t fwd_coxa = isRightSide(TRIPOD2_LEGS[i]) ? COXA_FORWARD : COXA_BACKWARD;
        moveLeg(TRIPOD2_LEGS[i], fwd_coxa, FEMUR_DOWN, TIBIA_DOWN, LOWER_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);
}
