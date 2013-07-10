#ifndef HEADER_TUTORIAL_MODE_HPP
#define HEADER_TUTORIAL_MODE_HPP

#include "modes/standard_race.hpp"
#include "LinearMath/btTransform.h"

class TutorialWorld : public StandardRace
{
public:

    TutorialWorld();
    virtual unsigned int getNumberOfRescuePositions() const OVERRIDE
    {
        // Don't use LinearWorld's function, but WorldWithRank, since the 
        // latter is based on rescuing to start positions
        return WorldWithRank::getNumberOfRescuePositions();
    }
    // ------------------------------------------------------------------------
    /** Determines the rescue position index of the specified kart. */
    virtual unsigned int getRescuePositionIndex(AbstractKart *kart) OVERRIDE
    {
        // Don't use LinearWorld's function, but WorldWithRank, since the 
        // latter is based on rescuing to start positions
        return WorldWithRank::getRescuePositionIndex(kart);
    }
    // ------------------------------------------------------------------------
    /** Returns the bullet transformation for the specified rescue index. */
    virtual btTransform getRescueTransform(unsigned int index) const OVERRIDE
    {
        // Don't use LinearWorld's function, but WorldWithRank, since the 
        // latter is based on rescuing to start positions
        return WorldWithRank::getRescueTransform(index);
    }

};   // class TutorialWorld

#endif
