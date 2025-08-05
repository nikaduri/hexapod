// TripodGait.h
#ifndef TRIPOD_GAIT_H
#define TRIPOD_GAIT_H

#include "Gait.h"

class TripodGait : public Gait {
public:
    TripodGait(LX16ABus& bus, LX16AServo** servoArray);

    void move() override;

private:
    
};

#endif // TRIPOD_GAIT_H
