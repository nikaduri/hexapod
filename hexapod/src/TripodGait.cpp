#include "TripodGait.h"


TripodGait::TripodGait(LX16ABus& bus, LX16AServo** servoArray)
    : Gait(bus, servoArray) {}

void TripodGait::move() {
    // === PHASE 1: Tripod 1 swings forward, Tripod 2 in stance ===
    Serial.println("Phase 1: Tripod 1 swings, Tripod 2 stance");

    // Step 0: Ensure Tripod 2 has good stance for traction
    Serial.println("Stabilizing Tripod 2 stance");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_DOWN, TIBIA_DOWN, 50);
    }
    delay(60); // Short stabilization time

    // Step 1: Lift Tripod 1 (all legs simultaneously)
    Serial.println("Lifting Tripod 1");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
    }
    delay(LIFT_TIME + SHORT_DELAY);

    // Step 2: Swing Tripod 1 forward (all legs simultaneously)
    Serial.println("Swinging Tripod 1 forward");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        moveLeg(leg, coxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
    }
    delay(MOVE_TIME + SHORT_DELAY);

    // Step 3: Lower Tripod 1 (all legs simultaneously)
    Serial.println("Lowering Tripod 1");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, LOWER_TIME);
    }
    delay(LOWER_TIME + SHORT_DELAY);

    // Step 4: Push Tripod 2 backward (all legs simultaneously) - SYMMETRIC TIMING
    Serial.println("Pushing Tripod 2 backward");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_BACKWARD : COXA_FORWARD;
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, PUSH_TIME);
    }
    delay(PUSH_TIME + SHORT_DELAY);

    // === PHASE 2: Tripod 2 swings forward, Tripod 1 in stance ===
    Serial.println("Phase 2: Tripod 2 swings, Tripod 1 stance");

    // Step 0: Ensure Tripod 1 has good stance for traction  
    Serial.println("Stabilizing Tripod 1 stance");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_DOWN, TIBIA_DOWN, 50);
    }
    delay(60); // Short stabilization time

    // Step 1: Lift Tripod 2 (all legs simultaneously)
    Serial.println("Lifting Tripod 2");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
    }
    delay(LIFT_TIME + SHORT_DELAY);

    // Step 2: Swing Tripod 2 forward (all legs simultaneously)
    Serial.println("Swinging Tripod 2 forward");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        moveLeg(leg, coxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
    }
    delay(MOVE_TIME + SHORT_DELAY);

    // Step 3: Lower Tripod 2 (all legs simultaneously)
    Serial.println("Lowering Tripod 2");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, LOWER_TIME);
    }
    delay(LOWER_TIME + SHORT_DELAY);

    // Step 4: Push Tripod 1 backward (all legs simultaneously) - SYMMETRIC TIMING
    Serial.println("Pushing Tripod 1 backward");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_BACKWARD : COXA_FORWARD;
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, PUSH_TIME);
    }
    delay(PUSH_TIME + SHORT_DELAY);
}


void TripodGait::rotateInPlace(Direction dir) {
    if (dir != LEFT && dir != RIGHT) {
        Serial.println("Invalid direction for rotateInPlace");
        return;
    }

    bool clockwise = (dir == RIGHT);
    Serial.println(clockwise ? "Rotating Right (Clockwise)" : "Rotating Left (Counter-clockwise)");

    const int* tripods[2] = {TRIPOD1_LEGS, TRIPOD2_LEGS};

    for (int phase = 0; phase < 2; ++phase) {
        const int* currentTripod = tripods[phase];
        const int* supportTripod = tripods[1 - phase];

        // === Step 0: Make sure support tripod is down and stable ===
        for (int i = 0; i < 3; ++i) {
            int leg = supportTripod[i];
            moveLeg(leg, COXA_DEFAULT, FEMUR_DOWN, TIBIA_DOWN, 0);
        }
        delay(100); // let it take the weight

        // === Step 1: Lift tripod to swing (all legs simultaneously) ===
        for (int i = 0; i < 3; ++i) {
            int leg = currentTripod[i];
            moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, FAST_LIFT_TIME);
        }
        delay(FAST_LIFT_TIME + SHORT_DELAY);

        // === Step 2: Rotate Coxa of lifted tripod (all legs simultaneously) ===
        for (int i = 0; i < 3; ++i) {
            int leg = currentTripod[i];
            int coxaTarget = clockwise ? COXA_BACKWARD : COXA_FORWARD;
            moveLeg(leg, coxaTarget, FEMUR_UP, TIBIA_UP, FAST_MOVE_TIME);
        }
        delay(FAST_MOVE_TIME + SHORT_DELAY);

        // === Step 3: Lower legs back down (all legs simultaneously) ===
        for (int i = 0; i < 3; ++i) {
            int leg = currentTripod[i];
            moveLeg(leg, COXA_DEFAULT, FEMUR_DOWN, TIBIA_DOWN, FAST_LOWER_TIME);
        }
        delay(FAST_LOWER_TIME + SHORT_DELAY);

        // === Step 4: Push opposite tripod to generate torque ===
        for (int i = 0; i < 3; ++i) {
            int leg = supportTripod[i];
            int coxaPush = clockwise ? COXA_FORWARD : COXA_BACKWARD;
            Serial.printf("Pushing Support Leg %d\n", leg);
            moveLeg(leg, coxaPush, FEMUR_DOWN, TIBIA_DOWN, FAST_PUSH_TIME);
            delay(SHORT_DELAY);
        }

        delay(SHORT_DELAY);

        // === Step 5: Return support tripod coxa to neutral ===
        for (int i = 0; i < 3; ++i) {
            int leg = supportTripod[i];
            moveLeg(leg, COXA_DEFAULT, FEMUR_DOWN, TIBIA_DOWN, FAST_MOVE_TIME);
        }

        delay(FAST_DELAY);
    }
}


