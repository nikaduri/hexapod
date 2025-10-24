#ifndef GAIT_H
#define GAIT_H

#include <Arduino.h>
#include <lx16a-servo.h>
#include <Constants.h>



class Gait {
public:

    Gait(LX16ABus& bus, LX16AServo** servoArray)
        : servoBus(bus), servos(servoArray) {}

    virtual ~Gait() {}

    virtual void move();


    protected:
    LX16ABus& servoBus;
    LX16AServo** servos;

    
    bool isRightSide(int base) {
        return base == 0 || base == 3 || base == 6;  // Right side legs
    }

    void moveLeg(int base, int32_t coxa, int32_t femur, int32_t tibia, int time = MOVE_TIME) {
        servos[base]->move_time(coxa, time);
        servos[base+1]->move_time(femur, time);
        servos[base+2]->move_time(tibia, time);
    }

    // Helper function to get switch index from leg base servo ID
    int getSwitchIndex(int legBase) {
        // SWITCH_PINS order: leg0, leg3, leg6, leg9, leg12, leg15
        switch(legBase) {
            case 0:  return 0;
            case 3:  return 1;
            case 6:  return 2;
            case 9:  return 3;
            case 12: return 4;
            case 15: return 5;
            default: return -1;
        }
    }

    // Check if a single leg is on the ground
    bool isLegOnGround(int legBase) {
        int switchIndex = getSwitchIndex(legBase);
        if (switchIndex < 0) return false;
        
        int switchPin = SWITCH_PINS[switchIndex];
        int reading = digitalRead(switchPin);
        return reading == 1;  // 1 means switch is pressed (leg is on ground)
    }

    // Debounced switch reading for reliable ground detection
    // Uses majority voting instead of requiring all consecutive readings
    bool readSwitchDebounced(int legBase, int debounceCount = 5) {
        int switchIndex = getSwitchIndex(legBase);
        if (switchIndex < 0) return false;
        
        int switchPin = SWITCH_PINS[switchIndex];
        int pressedCount = 0;
        
        // Take multiple readings with longer delays for mechanical switch stability
        for (int i = 0; i < debounceCount; i++) {
            if (digitalRead(switchPin) == 1) {
                pressedCount++;
            }
            if (i < debounceCount - 1) {
                delay(2);  // 2ms delay between reads for better mechanical switch stability
            }
        }
        
        // Require majority (at least 60%) of readings to be HIGH
        return pressedCount >= (debounceCount * 3 / 5);
    }

    // Adaptively lower a single leg until it touches the ground
    // Returns true if ground contact was made, false if timeout
    bool lowerLegUntilGrounded(int legBase, int32_t targetCoxa, int timeout = 3000) {
        const int32_t STEP_SIZE = 100;  // Servo units per step
        const int STEP_DELAY = 30;      // ms between steps
        
        unsigned long startTime = millis();
        
        // Read current positions
        int32_t currentFemur = servos[legBase+1]->pos_read();
        int32_t currentTibia = servos[legBase+2]->pos_read();
        
        // Set coxa to target position immediately
        servos[legBase]->move_time(targetCoxa, 150);
        
        Serial.print("Adaptively lowering leg ");
        Serial.print(legBase);
        Serial.print(" from femur=");
        Serial.print(currentFemur);
        Serial.print(", tibia=");
        Serial.println(currentTibia);
        
        // Gradually lower until ground contact or limits reached
        while ((millis() - startTime < timeout)) {
            // Check if already on ground (with debouncing)
            if (readSwitchDebounced(legBase)) {
                Serial.print("Leg ");
                Serial.print(legBase);
                Serial.print(" grounded at femur=");
                Serial.print(currentFemur);
                Serial.print(", tibia=");
                Serial.println(currentTibia);
                return true;
            }
            
            // Calculate next positions (lower means smaller values)
            int32_t nextFemur = currentFemur - STEP_SIZE;
            int32_t nextTibia = currentTibia - STEP_SIZE;
            
            // Clamp to minimum safe values (don't go below FEMUR_DOWN/TIBIA_DOWN)
            if (nextFemur < FEMUR_DOWN) nextFemur = FEMUR_DOWN;
            if (nextTibia < TIBIA_DOWN) nextTibia = TIBIA_DOWN;
            
            // If we've reached the limits, stop
            if (nextFemur == FEMUR_DOWN && nextTibia == TIBIA_DOWN && 
                currentFemur == FEMUR_DOWN && currentTibia == TIBIA_DOWN) {
                Serial.print("Leg ");
                Serial.print(legBase);
                Serial.println(" reached full extension without ground contact");
                return false;
            }
            
            // Move to next position
            servos[legBase+1]->move_time(nextFemur, STEP_DELAY);
            servos[legBase+2]->move_time(nextTibia, STEP_DELAY);
            
            currentFemur = nextFemur;
            currentTibia = nextTibia;
            
            delay(STEP_DELAY);
        }
        
        Serial.print("Timeout lowering leg ");
        Serial.println(legBase);
        return false;
    }
    
    // Adaptively lower all legs in a tripod until they touch ground
    void lowerTripodAdaptively(const int* tripodLegs) {
        Serial.print("Adaptively lowering tripod: legs ");
        for (int i = 0; i < 3; i++) {
            Serial.print(tripodLegs[i]);
            if (i < 2) Serial.print(", ");
        }
        Serial.println();
        
        // Lower all three legs simultaneously (but monitor each independently)
        bool allGrounded[3] = {false, false, false};
        int32_t currentFemur[3];
        int32_t currentTibia[3];
        
        // Initialize current positions (coxa stays unchanged during lowering)
        for (int i = 0; i < 3; i++) {
            int leg = tripodLegs[i];
            currentFemur[i] = servos[leg+1]->pos_read();
            currentTibia[i] = servos[leg+2]->pos_read();
        }
        
        const int32_t STEP_SIZE = 100;
        const int STEP_DELAY = 30;
        const int MAX_STEPS = 50;
        
        for (int step = 0; step < MAX_STEPS; step++) {
            bool allDone = true;
            
            // Check and update each leg
            for (int i = 0; i < 3; i++) {
                if (allGrounded[i]) continue;
                
                int leg = tripodLegs[i];
                
                // Check if this leg is now grounded (with debouncing)
                if (readSwitchDebounced(leg)) {
                    allGrounded[i] = true;
                    Serial.print("Leg ");
                    Serial.print(leg);
                    Serial.print(" grounded at step ");
                    Serial.println(step);
                    continue;
                }
                
                allDone = false;
                
                // Lower this leg more
                int32_t nextFemur = currentFemur[i] - STEP_SIZE;
                int32_t nextTibia = currentTibia[i] - STEP_SIZE;
                
                // Clamp to limits
                if (nextFemur < FEMUR_DOWN) nextFemur = FEMUR_DOWN;
                if (nextTibia < TIBIA_DOWN) nextTibia = TIBIA_DOWN;
                
                // Move to next position
                servos[leg+1]->move_time(nextFemur, STEP_DELAY);
                servos[leg+2]->move_time(nextTibia, STEP_DELAY);
                
                currentFemur[i] = nextFemur;
                currentTibia[i] = nextTibia;
            }
            
            if (allDone) {
                Serial.println("All legs in tripod grounded!");
                return;
            }
            
            delay(STEP_DELAY);
        }
        
        // Report any legs that didn't ground
        for (int i = 0; i < 3; i++) {
            if (!allGrounded[i]) {
                Serial.print("Warning: Leg ");
                Serial.print(tripodLegs[i]);
                Serial.print(" did not make ground contact - switch reading: ");
                int switchIndex = getSwitchIndex(tripodLegs[i]);
                Serial.println(digitalRead(SWITCH_PINS[switchIndex]));
            }
        }
    }
    
    // Wait for all legs in a tripod to be on the ground
    // Improved version with debouncing and better diagnostics
    void waitForTripodGrounded(const int* tripodLegs, int timeout = 3000) {
        unsigned long startTime = millis();
        bool allGrounded = false;
        
        Serial.print("Waiting for legs ");
        for (int i = 0; i < 3; i++) {
            Serial.print(tripodLegs[i]);
            if (i < 2) Serial.print(", ");
        }
        Serial.println(" to touch ground...");
        
        while (!allGrounded && (millis() - startTime < timeout)) {
            allGrounded = true;
            for (int i = 0; i < 3; i++) {
                int leg = tripodLegs[i];
                if (!readSwitchDebounced(leg, 5)) {  // Use more robust debounced reading
                    allGrounded = false;
                    int switchIndex = getSwitchIndex(leg);
                    int reading = digitalRead(SWITCH_PINS[switchIndex]);
                    Serial.print("Leg ");
                    Serial.print(leg);
                    Serial.print(" (pin ");
                    Serial.print(SWITCH_PINS[switchIndex]);
                    Serial.print("): ");
                    Serial.print(reading);
                    Serial.println(reading == 1 ? " [PRESSED]" : " [NOT PRESSED]");
                    break;
                }
            }
            if (!allGrounded) {
                delay(30);  // Shorter delay for more responsive checking
            }
        }
        
        if (!allGrounded) {
            Serial.println("Warning: Timeout waiting for tripod to ground");
            // Print final status of all legs
            for (int i = 0; i < 3; i++) {
                int leg = tripodLegs[i];
                int switchIndex = getSwitchIndex(leg);
                int reading = digitalRead(SWITCH_PINS[switchIndex]);
                Serial.print("Leg ");
                Serial.print(leg);
                Serial.print(" (pin ");
                Serial.print(SWITCH_PINS[switchIndex]);
                Serial.print("): ");
                Serial.print(reading);
                Serial.println(reading == 1 ? " [PRESSED]" : " [NOT PRESSED]");
            }
        } else {
            Serial.println("All legs grounded!");
        }
    }
};

#endif // GAIT_H
