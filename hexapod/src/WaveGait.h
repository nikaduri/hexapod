#ifndef WAVEGAIT_H
#define WAVEGAIT_H

#include "Gait.h"

class WaveGait : public Gait {
public:
    WaveGait(LX16ABus& bus, LX16AServo** servoArray);

    void move() override;
    
    void moveBackward() override;

private:

};

#endif
