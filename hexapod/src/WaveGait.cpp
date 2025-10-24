#include "WaveGait.h"
#include "Constants.h"

WaveGait::WaveGait(LX16ABus& bus, LX16AServo** servoArray)
    : Gait(bus, servoArray) {}

void WaveGait::move() {

    for (int step = 0; step < 6; step++) {
        int leg = WAVE_ORDER[step];

        // === LIFT ===
        Serial.printf("Lifting Leg %d\n", leg);
        moveLeg(leg, servos[leg]->pos_read(), FEMUR_UP, TIBIA_UP, FAST_LIFT_TIME);
        delay(FAST_LIFT_TIME + FAST_DELAY);

        // === SWING ===
        int coxa_target = isRightSide(leg) ? COXA_FORWARD : COXA_BACKWARD;
        Serial.printf("Swinging Forward Leg %d\n", leg);
        moveLeg(leg, coxa_target, FEMUR_UP, TIBIA_UP, FAST_MOVE_TIME);
        delay(FAST_MOVE_TIME + FAST_DELAY);

        // === LOWER ===
        Serial.printf("Lowering Leg %d\n", leg);
        moveLeg(leg, coxa_target, FEMUR_DOWN, TIBIA_DOWN, FAST_LOWER_TIME);
        delay(FAST_LOWER_TIME + FAST_DELAY);

        // === BODY PUSH ===
        Serial.println("Shifting body forward");
        for (int j = 0; j < 6; j++) {
            int support_leg = WAVE_ORDER[j];
            if (support_leg != leg) {
                int32_t current_coxa = servos[support_leg]->pos_read();
                int32_t shift = isRightSide(support_leg) ? BODY_PUSH_DELTA : -BODY_PUSH_DELTA;
                int32_t new_coxa = constrain(current_coxa + shift, COXA_FORWARD, COXA_BACKWARD);
                moveLeg(support_leg, new_coxa, FEMUR_DOWN, TIBIA_DOWN, FAST_PUSH_TIME);
            }
        }

        delay(FAST_PUSH_TIME + FAST_DELAY);
    }
}