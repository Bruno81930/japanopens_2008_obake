#ifndef BHV_OBAKE_DRIBBLE_H
#define BHV_OBAKE_DRIBBLE_H
#include<rcsc/player/soccer_action.h>

class Bhv_ObakeDribble : public rcsc::SoccerBehavior{
private:
    const rcsc::Vector2D M_home_pos;
public:
    Bhv_ObakeDribble(){}
    bool execute(rcsc::PlayerAgent *agent);
};


#endif
