#ifndef HORIZONTAL_BALANCE_H
#define HORIZONTAL_BALANCE_H

#include <Arduino.h>
#include <lx16a-servo.h>
#include "ICM_20948.h"
#include <PID_v1.h>
#include "Constants.h"

/**
 * HorizontalBalance class handles automatic balance control using gyroscope feedback.
 * 
 * The class uses two PID controllers:
 * - Roll PID: Controls balance around the front-back axis (left-right tilt)
 * - Pitch PID: Controls balance around the left-right axis (front-back tilt)
 * 
 * Balance compensation works by adjusting femur and tibia servo angles:
 * - When robot tilts forward: raise front legs, lower back legs
 * - When robot tilts backward: lower front legs, raise back legs  
 * - When robot tilts left: raise left legs, lower right legs
 * - When robot tilts right: lower left legs, raise right legs
 * 
 * Leg arrangement (viewed from top):
 *     Front
 *   12 ---- 15
 *  /          \
 * 9            3  
 *  \          /
 *   0 ---- 6
 *     Back
 */
class HorizontalBalance {
public:
    /**
     * Constructor
     * @param icm Reference to ICM_20948 gyroscope sensor
     * @param servoArray Array of 18 servo pointers (6 legs × 3 servos per leg)
     */
    HorizontalBalance(ICM_20948_I2C& icm, LX16AServo** servoArray);
    
    /**
     * Initialize the balance system
     * @return true if initialization successful, false otherwise
     */
    bool begin();
    
    /**
     * Enter balance mode - starts active balance control
     */
    void enterBalanceMode();
    
    /**
     * Exit balance mode - returns to normal standing position
     */
    void exitBalanceMode();
    
    /**
     * Update balance control loop - call this regularly in main loop
     * @return true if balance is active and updated, false if not in balance mode
     */
    bool updateBalance();
    
    /**
     * Check if currently in balance mode
     * @return true if in balance mode
     */
    bool isBalanceActive() const { return balanceActive; }
    
    /**
     * Get current roll angle in degrees
     */
    double getRoll() const { return currentRoll; }
    
    /**
     * Get current pitch angle in degrees  
     */
    double getPitch() const { return currentPitch; }
    
    /**
     * Calibrate the gyroscope (call when robot is on level surface)
     */
    void calibrateLevel();
    
    /**
     * Print debug information about current balance state
     */
    void printDebugInfo() const;
    
    /**
     * Test servo compensation by applying manual tilt values
     * @param testRoll Test roll angle in degrees
     * @param testPitch Test pitch angle in degrees
     */
    void testCompensation(double testRoll, double testPitch);
    
    /**
     * Autotune PID parameters for optimal response
     * @return true if tuning was successful
     */
    bool autotunePID();
    
    /**
     * Set PID parameters manually
     */
    void setPIDParameters(double kp_roll, double ki_roll, double kd_roll,
                         double kp_pitch, double ki_pitch, double kd_pitch);

private:
    // Hardware references
    ICM_20948_I2C& gyro;
    LX16AServo** servos;
    
    // Balance state
    bool balanceActive;
    bool initialized;
    unsigned long balanceStartMs;   // for smooth ramp-in
    double enableRampSec = 1.2;     // seconds to reach full authority
    bool tuned = false;             // has PID been tuned
    bool autoTuneOnStart = true;    // run autotune once on first enter
    
    // Sensor data
    double currentRoll;
    double currentPitch;
    double rollOffset;
    double pitchOffset;  // Calibration offsets
    
    // PID setpoints and inputs
    double rollSetpoint;
    double rollInput;
    double rollOutput;
    double pitchSetpoint;
    double pitchInput;
    double pitchOutput;
    
    // PID tuning parameters - will be set by autotuning
    double Kp_roll;
    double Ki_roll;
    double Kd_roll;
    double Kp_pitch;
    double Ki_pitch;
    double Kd_pitch;
    
    // PID controllers
    PID rollPID;
    PID pitchPID;
    
    // Default PID values (used if autotuning fails) - conservative to reduce wobble
    static constexpr double DEFAULT_KP = 1.2;
    static constexpr double DEFAULT_KI = 0.02;
    static constexpr double DEFAULT_KD = 0.3;
    
    // Balance limits
    static constexpr double MAX_COMPENSATION_ANGLE = 15.0;  // degrees
    static constexpr int MAX_SERVO_ADJUSTMENT = 1000;       // servo units
    static constexpr double ROLL_DEADBAND_DEG = 0.6;        // ignore tiny tilts
    static constexpr double PITCH_DEADBAND_DEG = 0.6;       // ignore tiny tilts
    
    // Leg groupings for balance compensation
    static const int FRONT_LEGS[2];    // Front legs: 12, 15
    static const int BACK_LEGS[2];     // Back legs: 0, 6  
    static const int LEFT_LEGS[3];     // Left legs: 0, 9, 12
    static const int RIGHT_LEGS[3];    // Right legs: 3, 6, 15
    
    // Default servo positions for balance mode
    static const int32_t BALANCE_FEMUR_DEFAULT = 17000;
    static const int32_t BALANCE_TIBIA_DEFAULT = 7000;

    // (no simplified control fields)
    
    /**
     * Read gyroscope data and update current roll/pitch angles
     * @return true if data read successfully
     */
    bool readGyroData();
    
    /**
     * Apply balance compensation to servo positions
     * @param rollCompensation Compensation angle for roll axis (-MAX_COMPENSATION_ANGLE to +MAX_COMPENSATION_ANGLE)
     * @param pitchCompensation Compensation angle for pitch axis (-MAX_COMPENSATION_ANGLE to +MAX_COMPENSATION_ANGLE)
     */
    void applyBalanceCompensation(double rollCompensation, double pitchCompensation);
    
    /**
     * Convert compensation angle to servo position adjustment
     * @param compensationAngle Angle in degrees
     * @return Servo position adjustment in servo units
     */
    int32_t angleToServoAdjustment(double compensationAngle);
    
    /**
     * Set all legs to default balance position
     */
    void setDefaultBalancePosition();
    
    /**
     * Utility function to check if a leg index is valid
     * @param legBase Base servo index for leg (0, 3, 6, 9, 12, or 15)
     * @return true if valid leg index
     */
    bool isValidLegIndex(int legBase);
};

#endif // HORIZONTAL_BALANCE_H
