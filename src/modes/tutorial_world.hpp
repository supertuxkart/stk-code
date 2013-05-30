#ifndef HEADER_TUTORIAL_MODE_HPP
#define HEADER_TUTORIAL_MODE_HPP

#include "modes/standard_race.hpp"
#include "LinearMath/btTransform.h"

class TutorialWorld : public StandardRace
{
private:
    btTransform getClosestStartPoint(float currentKart_x, float currentKart_z);
public:

    TutorialWorld();

    virtual void moveKartAfterRescue(AbstractKart* kart) OVERRIDE;
};

#endif
