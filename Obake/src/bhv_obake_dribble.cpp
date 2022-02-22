#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <rcsc/player/player_agent.h>
#include <rcsc/action/basic_actions.h>
#include <rcsc/param/server_param.h>
#include "obake_fuzzy_grade.h"
#include "bhv_obake_dribble.h"

bool Bhv_ObakeDribble::execute(rcsc::PlayerAgent * agent)
{
/*
    const rcsc::WorldModel & wm = agent->world();
    double dribble_power, degree_f_stamina, degree_n_l_stamina,
        degree_f_d_opp, degree_n_n_d_x_opp, degree_n_f_d_opp,
        dist_x_opp, dist_y_opp;
    dribble_power = rcsc::ServerParam::i().maxPower();
    const rcsc::PlayerPtrCont & opps = wm.opponentsFromSelf();
    const rcsc::PlayerObject * nearest_opp
        = ( opps.empty()
            ? static_cast< rcsc::PlayerObject * >( 0 )
            : opps.front() );
    dist_x_opp = (*nearest_opp).pos().x;
    dist_y_opp = (*opps.end())->pos().y;
    degree_f_stamina = Obake_FuzzyGrade().straightUp(wm.self().stamina(), 2800.0, 4000.0);
    degree_n_n_d_x_opp = 1.0 - Obake_FuzzyGrade().straightDown(dist_x_opp, 6.0, 8.5);
    return true;
*/

}


