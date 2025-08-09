#include "HorizontalBalance.h"
#include <math.h>

// Static member definitions
const int HorizontalBalance::FRONT_LEGS[2] = {12, 15};
const int HorizontalBalance::BACK_LEGS[2] = {0, 6};
const int HorizontalBalance::LEFT_LEGS[3] = {0, 9, 12};
const int HorizontalBalance::RIGHT_LEGS[3] = {3, 6, 15};

HorizontalBalance::HorizontalBalance(ICM_20948_I2C& icm, LX16AServo** servoArray)
    : gyro(icm),
      servos(servoArray),
      balanceActive(false),
      initialized(false),
      currentRoll(0),
      currentPitch(0),
      rollOffset(0),
      pitchOffset(0),
      rollSetpoint(0),
      rollInput(0),
      rollOutput(0),
      pitchSetpoint(0),
      pitchInput(0),
      pitchOutput(0),
      Kp_roll(DEFAULT_KP),
      Ki_roll(DEFAULT_KI),
      Kd_roll(DEFAULT_KD),
      Kp_pitch(DEFAULT_KP),
      Ki_pitch(DEFAULT_KI),
      Kd_pitch(DEFAULT_KD),
      rollPID(&rollInput, &rollOutput, &rollSetpoint, DEFAULT_KP, DEFAULT_KI, DEFAULT_KD, DIRECT),
      pitchPID(&pitchInput, &pitchOutput, &pitchSetpoint, DEFAULT_KP, DEFAULT_KI, DEFAULT_KD, DIRECT)
{
    // Configure PID controllers
    rollPID.SetMode(MANUAL);  // Start in manual mode
    pitchPID.SetMode(MANUAL);
    
    // Set output limits to prevent excessive servo adjustments
    rollPID.SetOutputLimits(-MAX_COMPENSATION_ANGLE, MAX_COMPENSATION_ANGLE);
    pitchPID.SetOutputLimits(-MAX_COMPENSATION_ANGLE, MAX_COMPENSATION_ANGLE);
    
    // Set sample time for faster response (20ms = 50Hz)
    rollPID.SetSampleTime(20);
    pitchPID.SetSampleTime(20);
}

bool HorizontalBalance::begin() {
    Serial.println("Initializing HorizontalBalance system...");
    
    // Check if gyroscope is connected and working
    if (gyro.status != ICM_20948_Stat_Ok) {
        Serial.println("ERROR: Gyroscope not connected or not responding!");
        return false;
    }
    
    // Wait for gyroscope to stabilize
    delay(100);
    
    // Calibrate the level position
    calibrateLevel();

    // Start with conservative PID parameters
    setPIDParameters(DEFAULT_KP, DEFAULT_KI, DEFAULT_KD,
                     DEFAULT_KP, DEFAULT_KI, DEFAULT_KD);
    
    initialized = true;
    Serial.println("HorizontalBalance system initialized successfully!");
    return true;
}

void HorizontalBalance::calibrateLevel() {
    Serial.println("Calibrating level position - keep robot still on flat surface...");
    
    double rollSum = 0, pitchSum = 0;
    const int calibrationSamples = 50;
    
    for (int i = 0; i < calibrationSamples; i++) {
        if (gyro.dataReady()) {
            gyro.getAGMT();
            
            // Calculate roll and pitch from accelerometer data
            double accelX = gyro.accX();
            double accelY = gyro.accY(); 
            double accelZ = gyro.accZ();
            
            // Calculate tilt angles using accelerometer
            double roll = atan2(accelY, accelZ) * 180.0 / PI;
            double pitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / PI;
            
            rollSum += roll;
            pitchSum += pitch;
        }
        delay(20);
    }
    
    // Calculate average offsets
    rollOffset = rollSum / calibrationSamples;
    pitchOffset = pitchSum / calibrationSamples;
    
    Serial.printf("Calibration complete. Roll offset: %.2f°, Pitch offset: %.2f°\n", 
                  rollOffset, pitchOffset);
}

void HorizontalBalance::enterBalanceMode() {
    if (!initialized) {
        Serial.println("ERROR: HorizontalBalance not initialized!");
        return;
    }
    
    Serial.println("Entering horizontal balance mode...");
    
    // Set legs to default balance position
    setDefaultBalancePosition();
    delay(500); // Allow servos to reach position
    
    // Enable PID controllers
    rollPID.SetMode(AUTOMATIC);
    pitchPID.SetMode(AUTOMATIC);
    
    balanceActive = true;
    balanceStartMs = millis();
    if (autoTuneOnStart && !tuned) {
        Serial.println("Autotune on first start...");
        autotunePID();
        tuned = true;
    }
    Serial.println("Balance mode active!");
}

void HorizontalBalance::exitBalanceMode() {
    Serial.println("Exiting balance mode...");
    
    balanceActive = false;
    
    // Disable PID controllers
    rollPID.SetMode(MANUAL);
    pitchPID.SetMode(MANUAL);
    
    // Return to standard standing position
    setDefaultBalancePosition();
    
    Serial.println("Balance mode disabled.");
}

bool HorizontalBalance::updateBalance() {
    if (!balanceActive || !initialized) {
        return false;
    }
    
    // Read current orientation
    if (!readGyroData()) {
        return false;
    }
    
    // Update PID inputs
    rollInput = currentRoll;
    pitchInput = currentPitch;

    // Soft-limit setpoints to zero (we want to keep level). If future uses set non-zero, they remain.
    rollSetpoint = 0.0;
    pitchSetpoint = 0.0;
    
    double t = (millis() - balanceStartMs) / 1000.0;
    double ramp = t < enableRampSec ? (t / enableRampSec) : 1.0;

    bool rollComputed = rollPID.Compute();
    bool pitchComputed = pitchPID.Compute();
    if (rollComputed || pitchComputed) {
        applyBalanceCompensation(rollOutput * ramp, pitchOutput * ramp);
    }

    // Debug output (comment out for production)
    static unsigned long lastDebugTime = 0;
    if (millis() - lastDebugTime > 500) { // Debug every 500ms
        Serial.printf("Balance: Roll=%.2f° (out=%.2f), Pitch=%.2f° (out=%.2f)\n",
                     currentRoll, rollOutput, currentPitch, pitchOutput);
        lastDebugTime = millis();
    }
    
    return true;
}

bool HorizontalBalance::readGyroData() {
    // Read latest accel/gyro regardless of dataReady flag to avoid stalling
    gyro.getAGMT();
    
    // Calculate roll and pitch from accelerometer data
    double accelX = gyro.accX();
    double accelY = gyro.accY();
    double accelZ = gyro.accZ();
    
    // Calculate tilt angles and apply calibration offsets
    currentRoll = atan2(accelY, accelZ) * 180.0 / PI - rollOffset;   // left(+)/right(-) tilt
    currentPitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / PI - pitchOffset; // forward(+)/back(-)
    
    // Apply light low-pass filter to reduce noise while maintaining responsiveness
    static double filteredRoll = 0, filteredPitch = 0;
    const double alpha = 0.4; // earlier lighter filtering
    
    filteredRoll = alpha * currentRoll + (1.0 - alpha) * filteredRoll;
    filteredPitch = alpha * currentPitch + (1.0 - alpha) * filteredPitch;
    
    currentRoll = filteredRoll;
    currentPitch = filteredPitch;
    return true;
}

void HorizontalBalance::applyBalanceCompensation(double rollCompensation, double pitchCompensation) {
    // Deadbands to avoid hunting around zero
    if (fabs(rollCompensation) < ROLL_DEADBAND_DEG) rollCompensation = 0.0;
    if (fabs(pitchCompensation) < PITCH_DEADBAND_DEG) pitchCompensation = 0.0;

    // Clamp compensation angles to safe limits
    rollCompensation = constrain(rollCompensation, -MAX_COMPENSATION_ANGLE, MAX_COMPENSATION_ANGLE);
    pitchCompensation = constrain(pitchCompensation, -MAX_COMPENSATION_ANGLE, MAX_COMPENSATION_ANGLE);

    // Convert angles to servo adjustments (use gentler scaling)
    const double femurUnitsPerDeg = 80.0; // empirically closer to geometry for LX16A
    const double tibiaUnitsPerDeg = 50.0;

    auto toUnits = [](double angle, double unitsPerDeg) -> int32_t {
        return (int32_t)(angle * unitsPerDeg);
    };

    // Compute per-leg combined effects: femur height is proportional to both roll and pitch.
    // Signs per leg: left(+roll), right(-roll), front(+pitch), back(-pitch)
    struct LegEffect { int base; int sRoll; int sPitch; };
    const LegEffect legs[] = {
        {0,   +1, -1}, // back-left
        {3,   -1, -1}, // back-right
        {6,   -1, -1}, // back-right-middle
        {9,   +1, +1}, // front-left-middle
        {12,  +1, +1}, // front-left
        {15,  -1, +1}  // front-right
    };

    for (const auto &leg : legs) {
        // Combined desired height delta in degrees
        double femurDeg = leg.sRoll * rollCompensation + leg.sPitch * pitchCompensation;
        int32_t femurAdj = toUnits(femurDeg, femurUnitsPerDeg);
        int32_t tibiaAdj = toUnits(femurDeg * 0.5, tibiaUnitsPerDeg); // smaller tibia contribution

        int32_t femurPos = BALANCE_FEMUR_DEFAULT + femurAdj;
        int32_t tibiaPos = BALANCE_TIBIA_DEFAULT - tibiaAdj; // opposite to femur to keep foot flat-ish

        femurPos = constrain(femurPos, FEMUR_DOWN - 600, FEMUR_UP);
        tibiaPos = constrain(tibiaPos, TIBIA_DOWN, TIBIA_UP + 500);

        servos[leg.base + 1]->move_time(femurPos, 35);
        servos[leg.base + 2]->move_time(tibiaPos, 35);
    }
}

int32_t HorizontalBalance::angleToServoAdjustment(double compensationAngle) {
    // Legacy helper no longer used for combined mapping; keep for compatibility
    return (int32_t)(compensationAngle * 18.0);
}

void HorizontalBalance::setDefaultBalancePosition() {
    Serial.println("Setting default balance position...");
    
    // Set all legs to default balance stance
    for (int leg = 0; leg < 18; leg += 3) {
        if (isValidLegIndex(leg)) {
            servos[leg]->move_time(COXA_DEFAULT, 200);     // Coxa to neutral
            servos[leg + 1]->move_time(BALANCE_FEMUR_DEFAULT, 200); // Femur
            servos[leg + 2]->move_time(BALANCE_TIBIA_DEFAULT, 200); // Tibia
        }
    }
}

bool HorizontalBalance::isValidLegIndex(int legBase) {
    return (legBase == 0 || legBase == 3 || legBase == 6 || 
            legBase == 9 || legBase == 12 || legBase == 15);
}

void HorizontalBalance::printDebugInfo() const {
    Serial.println("=== Balance System Debug Info ===");
    Serial.printf("Status: %s\n", balanceActive ? "ACTIVE" : "INACTIVE");
    Serial.printf("Initialized: %s\n", initialized ? "YES" : "NO");
    Serial.printf("Current Roll: %.2f° (offset: %.2f°)\n", currentRoll, rollOffset);
    Serial.printf("Current Pitch: %.2f° (offset: %.2f°)\n", currentPitch, pitchOffset);
    
    if (balanceActive) {
        Serial.printf("Roll PID - Input: %.2f, Output: %.2f, Setpoint: %.2f\n", 
                     rollInput, rollOutput, rollSetpoint);
        Serial.printf("Pitch PID - Input: %.2f, Output: %.2f, Setpoint: %.2f\n", 
                     pitchInput, pitchOutput, pitchSetpoint);
    }
    
    Serial.printf("Gyro Status: %s\n", 
                 gyro.status == ICM_20948_Stat_Ok ? "OK" : "ERROR");
    Serial.println("================================");
}

void HorizontalBalance::setPIDParameters(double kp_roll, double ki_roll, double kd_roll,
                                   double kp_pitch, double ki_pitch, double kd_pitch) {
    // Update stored parameters
    Kp_roll = kp_roll;
    Ki_roll = ki_roll;
    Kd_roll = kd_roll;
    Kp_pitch = kp_pitch;
    Ki_pitch = ki_pitch;
    Kd_pitch = kd_pitch;
    
    // Update PID controllers
    rollPID.SetTunings(Kp_roll, Ki_roll, Kd_roll);
    pitchPID.SetTunings(Kp_pitch, Ki_pitch, Kd_pitch);
    
    Serial.printf("Updated PID parameters:\n");
    Serial.printf("Roll: Kp=%.2f, Ki=%.2f, Kd=%.2f\n", Kp_roll, Ki_roll, Kd_roll);
    Serial.printf("Pitch: Kp=%.2f, Ki=%.2f, Kd=%.2f\n", Kp_pitch, Ki_pitch, Kd_pitch);
}

bool HorizontalBalance::autotunePID() {
    Serial.println("Starting PID autotuning...");
    
    // Save current mode and disable balance
    bool wasActive = balanceActive;
    balanceActive = false;
    
    // Parameters for autotuning
    const int testDuration = 2000;  // 2 seconds per test
    const int numTests = 5;         // Number of tests per axis
    const double testAmplitude = 5.0; // 5 degrees test movement
    
    double bestKp_roll = DEFAULT_KP;
    double bestKi_roll = DEFAULT_KI;
    double bestKd_roll = DEFAULT_KD;
    double bestError_roll = 1000000;
    
    double bestKp_pitch = DEFAULT_KP;
    double bestKi_pitch = DEFAULT_KI;
    double bestKd_pitch = DEFAULT_KD;
    double bestError_pitch = 1000000;
    
    // Test parameters for roll axis
    Serial.println("Tuning roll axis...");
    for (int test = 0; test < numTests; test++) {
        // Generate test parameters
        double kp = DEFAULT_KP * (0.5 + (test / (double)numTests) * 1.5);
        double ki = DEFAULT_KI * (0.5 + (test / (double)numTests) * 1.5);
        double kd = DEFAULT_KD * (0.5 + (test / (double)numTests) * 1.5);
        
        // Configure PID
        rollPID.SetTunings(kp, ki, kd);
        rollPID.SetMode(AUTOMATIC);
        
        unsigned long startTime = millis();
        double totalError = 0;
        int samples = 0;
        
        // Run test
        while (millis() - startTime < testDuration) {
            // Generate sine wave reference
            double setpoint = testAmplitude * sin(2 * PI * (millis() - startTime) / 1000.0);
            rollSetpoint = setpoint;
            
            if (readGyroData()) {
                rollInput = currentRoll;
                if (rollPID.Compute()) {
                    applyBalanceCompensation(rollOutput, 0);
                    totalError += abs(rollSetpoint - rollInput);
                    samples++;
                }
            }
            delay(20);
        }
        
        double avgError = totalError / samples;
        Serial.printf("Roll test %d: Kp=%.2f, Ki=%.2f, Kd=%.2f, Error=%.2f\n",
                     test, kp, ki, kd, avgError);
        
        if (avgError < bestError_roll) {
            bestError_roll = avgError;
            bestKp_roll = kp;
            bestKi_roll = ki;
            bestKd_roll = kd;
        }
    }
    
    // Test parameters for pitch axis
    Serial.println("Tuning pitch axis...");
    for (int test = 0; test < numTests; test++) {
        // Generate test parameters
        double kp = DEFAULT_KP * (0.5 + (test / (double)numTests) * 1.5);
        double ki = DEFAULT_KI * (0.5 + (test / (double)numTests) * 1.5);
        double kd = DEFAULT_KD * (0.5 + (test / (double)numTests) * 1.5);
        
        // Configure PID
        pitchPID.SetTunings(kp, ki, kd);
        pitchPID.SetMode(AUTOMATIC);
        
        unsigned long startTime = millis();
        double totalError = 0;
        int samples = 0;
        
        // Run test
        while (millis() - startTime < testDuration) {
            // Generate sine wave reference
            double setpoint = testAmplitude * sin(2 * PI * (millis() - startTime) / 1000.0);
            pitchSetpoint = setpoint;
            
            if (readGyroData()) {
                pitchInput = currentPitch;
                if (pitchPID.Compute()) {
                    applyBalanceCompensation(0, pitchOutput);
                    totalError += abs(pitchSetpoint - pitchInput);
                    samples++;
                }
            }
            delay(20);
        }
        
        double avgError = totalError / samples;
        Serial.printf("Pitch test %d: Kp=%.2f, Ki=%.2f, Kd=%.2f, Error=%.2f\n",
                     test, kp, ki, kd, avgError);
        
        if (avgError < bestError_pitch) {
            bestError_pitch = avgError;
            bestKp_pitch = kp;
            bestKi_pitch = ki;
            bestKd_pitch = kd;
        }
    }
    
    // Apply best parameters
    setPIDParameters(bestKp_roll, bestKi_roll, bestKd_roll,
                    bestKp_pitch, bestKi_pitch, bestKd_pitch);
    
    // Reset modes
    rollPID.SetMode(MANUAL);
    pitchPID.SetMode(MANUAL);
    balanceActive = wasActive;
    
    Serial.println("PID autotuning complete!");
    return true;
}

void HorizontalBalance::testCompensation(double testRoll, double testPitch) {
    Serial.printf("Testing compensation with Roll=%.2f°, Pitch=%.2f°\n", testRoll, testPitch);
    
    // Override PID outputs for testing
    rollOutput = testRoll;
    pitchOutput = testPitch;
    
    applyBalanceCompensation(rollOutput, pitchOutput);
    
    Serial.println("Test compensation applied. Send STAND to return to normal.");
    
    // Note: Don't restore outputs here - let the user return to normal with STAND command
}
