#ifndef NETWORK_PLAYER_CONTROLLER_HPP
#define NETWORK_PLAYER_CONTROLLER_HPP

#include "karts/controller/controller.hpp"

class AbstractKart;
class Player;

class NetworkPlayerController : public Controller
{
protected:
    int            m_steer_val, m_steer_val_l, m_steer_val_r;
    int            m_prev_accel;
    bool           m_prev_brake;
    bool           m_prev_nitro;

    float          m_penalty_time;

    void           steer(float, int);
public:
    NetworkPlayerController  (AbstractKart *kart,
                       StateManager::ActivePlayer *_player);
    virtual ~NetworkPlayerController  ();
    void           update            (float);
    void           action            (PlayerAction action, int value);
    void           handleZipper      (bool play_sound);
    void           collectedItem     (const Item &item, int add_info=-1,
                                      float previous_energy=0);
    virtual void   skidBonusTriggered();
    virtual void   setPosition       (int p);
    virtual bool   isPlayerController() const { return true; }
    virtual bool   isLocalPlayerController() const { return false; }
    virtual void   reset             ();
    void           resetInputState   ();
    virtual void   finishedRace      (float time);
    virtual void   crashed           (const AbstractKart *k) {}
    virtual void   crashed           (const Material *m) {}
    // ------------------------------------------------------------------------
    /** Callback whenever a new lap is triggered. Used by the AI
     *  to trigger a recomputation of the way to use, not used for players. */
    virtual void  newLap(int lap) {}
    // ------------------------------------------------------------------------
    /** Player will always be able to get a slipstream bonus. */
    virtual bool  disableSlipstreamBonus() const { return false; }

};

#endif // NETWORK_PLAYER_CONTROLLER_HPP
