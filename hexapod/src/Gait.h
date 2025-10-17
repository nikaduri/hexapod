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
};

#endif // GAIT_H
