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
    // IMPORTANT:
    // COXA_FORWARD/BACKWARD are servo extremes, not world directions.
    // Due to mirrored servo orientations, achieving opposite world motion on left vs right
    // requires setting the SAME servo extreme on both sides.
    // To rotate LEFT (CCW): swing = COXA_FORWARD, push = COXA_BACKWARD (both sides)
    // To rotate RIGHT (CW): swing = COXA_BACKWARD, push = COXA_FORWARD (both sides)
    const int32_t swingCoxa = (dir == LEFT) ? COXA_FORWARD : COXA_BACKWARD;
    const int32_t pushCoxa  = (dir == LEFT) ? COXA_BACKWARD : COXA_FORWARD;

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


