// Enums.h
#pragma once

// Direction for movement/rotation
enum Direction {
    BACKWARD,
    LEFT,
    RIGHT
};

// Gait pattern options
enum GaitPattern {
    TRIPOD,
    WAVE,
    RIPPLE
};

// High-level robot operating modes
enum RobotMode {
    IDLE,           // Standing / not moving (also used for STAND command)
    MOVE_FORWARD,   // Continuous forward gait
    MOVE_BACKWARD,  // Backward (rotate 180 then forward step pattern)
    ROTATE_LEFT,    // In-place rotation to the left
    ROTATE_RIGHT,   // In-place rotation to the right
    LAY_DOWN,       // Initiate lay down posture (one-shot)
    LAID_DOWN,      // Persist laid down posture (hold position)
    STAND_UP,       // One-shot stand-up sequence from laid-down posture
    DANCE,          // Dance sequence (not yet implemented)
    BALANCE         // Horizontal balance mode using gyroscope
};
