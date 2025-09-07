#include "TripodGait.h"


TripodGait::TripodGait(LX16ABus& bus, LX16AServo** servoArray)
    : Gait(bus, servoArray) {}

// Helper function to apply drift correction to coxa positions
int32_t TripodGait::applyCoxaOffset(int32_t basePosition, int leg) {
    int32_t offset = isRightSide(leg) ? RIGHT_SIDE_COXA_OFFSET : LEFT_SIDE_COXA_OFFSET;
    return basePosition + offset;
}

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
    // For TRUE rotation, ALL legs must work together to create angular movement
    // The key insight: legs move in a circular pattern around the robot's center
    
    // === PHASE 1: Tripod 1 swings, Tripod 2 pushes ===
    
    // Step 1: Lift Tripod 1
    Serial.println("Lifting Tripod 1 for rotation");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
    }
    delay(LIFT_TIME + SHORT_DELAY);

    // Step 2 (Combined): Swing and Push for rotation
    Serial.println("Rotating Tripod 1 & Pushing Tripod 2");
    
    // Tripod 1 swing - move to new rotational positions
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        int32_t targetCoxa;
        
        if (dir == LEFT) {
            // For counterclockwise rotation: ALL legs move clockwise relative to their current position
            targetCoxa = COXA_ROTATE_FORWARD;  // All legs to same extreme
        } else { // RIGHT
            // For clockwise rotation: ALL legs move counterclockwise relative to their current position  
            targetCoxa = COXA_ROTATE_BACKWARD; // All legs to same extreme
        }
        moveLeg(leg, targetCoxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
    }
    
    // Tripod 2 push - create rotational force by pushing in opposite direction
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        int32_t pushCoxa;
        
        if (dir == LEFT) {
            // Push in opposite direction to create rotation
            pushCoxa = COXA_ROTATE_BACKWARD;
        } else { // RIGHT
            pushCoxa = COXA_ROTATE_FORWARD;
        }
        moveLeg(leg, pushCoxa, FEMUR_STANCE_ROTATE, TIBIA_STANCE_ROTATE, MOVE_TIME);
    }
    delay(MOVE_TIME + SHORT_DELAY);
    
    // Step 3: Lower Tripod 1 to the ground
    Serial.println("Lowering Tripod 1");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_STANCE_ROTATE, TIBIA_STANCE_ROTATE, LOWER_TIME);
    }
    delay(LOWER_TIME);

    // === PHASE 2: Tripod 2 swings, Tripod 1 pushes ===
    
    // Step 1: Lift Tripod 2
    Serial.println("Lifting Tripod 2 for rotation");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, LIFT_TIME);
    }
    delay(LIFT_TIME + SHORT_DELAY);

    // Step 2 (Combined): Swing and Push for rotation
    Serial.println("Rotating Tripod 2 & Pushing Tripod 1");
    
    // Tripod 2 swing - continue rotational movement
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        int32_t targetCoxa;
        
        if (dir == LEFT) {
            targetCoxa = COXA_ROTATE_FORWARD;
        } else { // RIGHT
            targetCoxa = COXA_ROTATE_BACKWARD;
        }
        moveLeg(leg, targetCoxa, FEMUR_UP, TIBIA_UP, MOVE_TIME);
    }
    
    // Tripod 1 push - create rotational force
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD1_LEGS[i];
        int32_t pushCoxa;
        
        if (dir == LEFT) {
            pushCoxa = COXA_ROTATE_BACKWARD;
        } else { // RIGHT
            pushCoxa = COXA_ROTATE_FORWARD;
        }
        moveLeg(leg, pushCoxa, FEMUR_STANCE_ROTATE, TIBIA_STANCE_ROTATE, MOVE_TIME);
    }
    delay(MOVE_TIME + SHORT_DELAY);
    
    // Step 3: Lower Tripod 2 to the ground
    Serial.println("Lowering Tripod 2");
    for (int i = 0; i < 3; i++) {
        int leg = TRIPOD2_LEGS[i];
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_STANCE_ROTATE, TIBIA_STANCE_ROTATE, LOWER_TIME);
    }
    delay(LOWER_TIME); 
}