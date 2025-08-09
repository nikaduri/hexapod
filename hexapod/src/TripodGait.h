#ifndef TRIPOD_GAIT_H
#define TRIPOD_GAIT_H

#include "Gait.h"
#include "Constants.h"
#include "Enums.h"

class TripodGait : public Gait {
public:
    TripodGait(LX16ABus& bus, LX16AServo** servoArray);

    void move() override;

    void rotateInPlace(Direction dir);

private:
    
};

#endif // TRIPOD_GAIT_H
