#include "TripodGait.h"


TripodGait::TripodGait(LX16ABus& bus, LX16AServo** servoArray)
    : Gait(bus, servoArray) {}

void TripodGait::move() {
    // === PHASE 1: Tripod 1 swings, Tripod 2 pushes ===
    // ... (Lift and Swing/Push code is correct) ...

    // Step 1: Lift Tripod 1
    Serial.println("Lifting Tripod 1");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
    }
    delay(LIFT_TIME + SHORT_DELAY);

    // Step 2 (Combined): Swing and Push
    Serial.println("Swinging Tripod 1 & Pushing Tripod 2");
    for (int i = 0; i < 3; i++) { // Tripod 1 swing
        int leg = TRIPOD1_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        moveLeg(leg, coxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
    }
    for (int i = 0; i < 3; i++) { // Tripod 2 push
        int leg = TRIPOD2_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_BACKWARD : COXA_FORWARD;
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, MOVE_TIME);
    }
    delay(MOVE_TIME + SHORT_DELAY);
    
    // Step 3: Lower Tripod 1 to the ground
    Serial.println("Lowering Tripod 1");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_DOWN, TIBIA_DOWN, LOWER_TIME);
    }
    delay(LOWER_TIME);


    // === PHASE 2: Tripod 2 swings, Tripod 1 pushes ===
    // ... (Lift and Swing/Push code is correct) ...

    // Step 1: Lift Tripod 2
    Serial.println("Lifting Tripod 2");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
    }
    delay(LIFT_TIME + SHORT_DELAY);

    // Step 2 (Combined): Swing and Push
    Serial.println("Swinging Tripod 2 & Pushing Tripod 1");
    for (int i = 0; i < 3; i++) { // Tripod 2 swing
        int leg = TRIPOD2_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        moveLeg(leg, coxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
    }
    for (int i = 0; i < 3; i++) { // Tripod 1 push
        int leg = TRIPOD1_LEGS[i];
        int coxa = isRightSide(leg) ? COXA_BACKWARD : COXA_FORWARD;
        moveLeg(leg, coxa, FEMUR_DOWN, TIBIA_DOWN, MOVE_TIME);
    }
    delay(MOVE_TIME + SHORT_DELAY);
    
    // Step 3: Lower Tripod 2 to the ground
    Serial.println("Lowering Tripod 2");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_DOWN, TIBIA_DOWN, LOWER_TIME);
    }
    delay(LOWER_TIME); 
}


void TripodGait::rotateInPlace(Direction dir) {
    const int32_t swingCoxa = (dir == LEFT) ? COXA_FORWARD + 1000 : COXA_BACKWARD - 1000;
    const int32_t pushCoxa  = (dir == LEFT) ? COXA_BACKWARD - 1000 : COXA_FORWARD + 1000;

    // === PHASE 1: Tripod 1 swings, Tripod 2 generates torque ===
    Serial.println("Rotate Phase 1: Tripod 1 swing, Tripod 2 push");

    // Stabilize Tripod 2
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_DOWN, TIBIA_DOWN, 60);
    }
    delay(60 + SHORT_DELAY);

    // Lift Tripod 1
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
    }
    delay(LIFT_TIME + SHORT_DELAY);

    // Swing Tripod 1 coxa while lifted (same extreme both sides per note above)
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        moveLeg(leg, swingCoxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
    }
    delay(MOVE_TIME + SHORT_DELAY);

    // Lower Tripod 1
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        moveLeg(leg, swingCoxa, FEMUR_DOWN, TIBIA_DOWN, LOWER_TIME);
    }
    delay(LOWER_TIME + SHORT_DELAY);

    // Stance push with Tripod 2 to create yaw torque
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        moveLeg(leg, pushCoxa, FEMUR_DOWN, TIBIA_DOWN, PUSH_TIME);
    }
    delay(PUSH_TIME + SHORT_DELAY);

    // === PHASE 2: Tripod 2 swings, Tripod 1 generates torque ===
    Serial.println("Rotate Phase 2: Tripod 2 swing, Tripod 1 push");

    // Stabilize Tripod 1
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_DOWN, TIBIA_DOWN, 60);
    }
    delay(60 + SHORT_DELAY);

    // Lift Tripod 2
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
    }
    delay(LIFT_TIME + SHORT_DELAY);

    // Swing Tripod 2 coxa while lifted
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        moveLeg(leg, swingCoxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
    }
    delay(MOVE_TIME + SHORT_DELAY);

    // Lower Tripod 2
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        moveLeg(leg, swingCoxa, FEMUR_DOWN, TIBIA_DOWN, LOWER_TIME);
    }
    delay(LOWER_TIME + SHORT_DELAY);

    // Stance push with Tripod 1
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        moveLeg(leg, pushCoxa, FEMUR_DOWN, TIBIA_DOWN, PUSH_TIME);
    }
    delay(PUSH_TIME + SHORT_DELAY);
}