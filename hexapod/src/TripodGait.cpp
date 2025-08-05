#include "TripodGait.h"
#include "Constants.h"

TripodGait::TripodGait(LX16ABus& bus, LX16AServo** servoArray)
    : Gait(bus, servoArray) {}

void TripodGait::move() {
    // === PHASE 1: Tripod 1 swings forward, Tripod 2 in stance ===
    Serial.println("Phase 1: Tripod 1 swings, Tripod 2 stance");

    // Step 1: Lift Tripod 1
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        Serial.printf("Lifting Leg %d\n", leg);
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // Step 2: Swing Tripod 1 forward
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        Serial.printf("Swinging Forward Leg %d\n", leg);
        moveLeg(leg, coxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // Step 3: Lower Tripod 1
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        Serial.printf("Lowering Leg %d\n", leg);
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, LOWER_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // Step 4: Push Tripod 2 backward
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_BACKWARD : COXA_FORWARD;
        Serial.printf("Pushing Leg %d\n", leg);
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, PUSH_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // === PHASE 2: Tripod 2 swings forward, Tripod 1 in stance ===
    Serial.println("Phase 2: Tripod 2 swings, Tripod 1 stance");

    // Step 1: Lift Tripod 2
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        Serial.printf("Lifting Leg %d\n", leg);
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // Step 2: Swing Tripod 2 forward
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        Serial.printf("Swinging Forward Leg %d\n", leg);
        moveLeg(leg, coxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // Step 3: Lower Tripod 2
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        Serial.printf("Lowering Leg %d\n", leg);
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, LOWER_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);

    // Step 4: Push Tripod 1 backward
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_BACKWARD : COXA_FORWARD;
        Serial.printf("Pushing Leg %d\n", leg);
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, PUSH_TIME);
        delay(SHORT_DELAY);
    }
    delay(SHORT_DELAY);
}
