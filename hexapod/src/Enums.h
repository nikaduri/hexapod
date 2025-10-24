#pragma once

enum Direction {
    BACKWARD,
    LEFT,
    RIGHT
};

enum GaitPattern {
    TRIPOD,
    WAVE,
    RIPPLE
};

enum RobotMode {
    IDLE,
    MOVE_FORWARD,
    MOVE_BACKWARD,
    ROTATE_LEFT,
    ROTATE_RIGHT,
    LAY_DOWN,
    STAND_UP,
    DANCE,
    BALANCE,
    NONE
};
