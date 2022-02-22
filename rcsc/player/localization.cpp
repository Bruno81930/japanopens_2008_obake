// -*-c++-*-

/*!
  \file localization.cpp
  \brief localization module Source File
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "localization.h"
#include "object_table.h"

#include <rcsc/common/logger.h>
#include <rcsc/geom/sector_2d.h>
#include <rcsc/random.h>

#include <algorithm>

using std::min;
using std::max;

namespace rcsc {

//! type of maerker map container
typedef std::map< MarkerID, Vector2D > MarkerMap;

/*!
  \struct LocalizeImpl
  \brief localization implementation
*/
class LocalizeImpl {

private:

    //! object distance table
    ObjectTable M_object_table;

    //! our side ID
    const SideID M_our_side;

    //! particle vector
    std::vector< Vector2D > M_particles;
public:
    /*!
      \brief create landmark map and object table
      \param ourside our team side
    */
    explicit
    LocalizeImpl( const SideID ourside )
        : M_object_table( ourside )
        , M_our_side( ourside )
      {
          M_particles.reserve( 1024 );
      }

    /*!
      \brief get object table
      \return const reference to the object table instance
    */
    const
    ObjectTable & objectTable() const
      {
          return M_object_table;
      }

    const
    std::vector< Vector2D > & particles() const
      {
          return M_particles;
      }

    // self localization

    /*!
      \brief update particles using seen markers
      \param markers seen marker container
      \param self_face agent's global face angle
      \param self_face_err agent's global face angle error
    */
    void updateParticlesByMarkers( const VisualSensor::MarkerCont & markers,
                                   const double & self_face,
                                   const double & self_face_err );

    /*!
      \brief update particles using seen markers
      \param behind_markers feeled behind marker container
      \param self_pos agent's global position
      \param self_face agent's global face angle
      \param self_face_err agent's global face angle error
    */
    void updateParticlesByBehindMarker( const VisualSensor::MarkerCont & behind_markers,
                                        const Vector2D & self_pos,
                                        const double & self_face,
                                        const double & self_face_err );

    /*!
      \brief update particles by one marker
      \param marker seen marker info
      \param id estimated marker's Id
      \param self_face agent's global face angle
      \param self_face_err agent's global face angle error
    */
    void updateParticlesBy( const VisualSensor::MarkerT & marker,
                            const MarkerID id,
                            const double & self_face,
                            const double & self_face_err );

    /*!
      \brief calculate average point and error with all particles.
      \param ave_pos pointer to the variable to store the averaged point
      \param ave_err pointer to the variable to store the averaged point error
    */
    void getAverageOfParticles( Vector2D * ave_pos,
                                Vector2D * ave_err );

    /*!
      \brief generate first particles using nearest marker
      \param marker seen marker info
      \param id estimated marker Id
      \param self_face agent's global face angle
      \param self_face_err agent's global face angle error
    */
    void generateParticles( const VisualSensor::MarkerT & marker,
                            const MarkerID id,
                            const double & self_face,
                            const double & self_face_err );

    // utility

    /*!
      \brief get nearest marker flag Id. include goal check
      \param objtype used only to detect 'goal' or 'flag'
      \param mypos estimated self coordinate
      \return marker ID

      This method is used to identify the behind marker object.
    */
    MarkerID getNearestMarker( const VisualSensor::ObjectType objtype,
                               const Vector2D & mypos ) const;

    /*!
      \brief get global angle & angle range
      \param seen_dir seen dir
      \param self_face agent's global face angle
      \param self_face_err agent's global face angle error
      \param average pointer to the variable to store the averaged angle
      \param err pointer to the variable to store the estimated angle error.
    */
    void getDirRange( const double & seen_dir,
                      const double & self_face,
                      const double & self_face_err,
                      double * average,
                      double * err );

    // get unquantized dist range
    // void getDistRange(const double &see_dist, const double &qstep,
    //                   double *average, double *range);


    /*!
      \brief estimate self global face angle from seen markers
      \param markers seen marker container
      \return self face angle. if failed, retun VisualSensor::DIR_ERR
    */
    double getFaceDirByMarkers( const VisualSensor::MarkerCont & markers );

    /*!
      \brief estimate self global face angle from seen lines
      \param lines seen line info
      \return self face angle. if failed, retun VisualSensor::DIR_ERR
    */
    double getFaceDirByLines( const VisualSensor::LineCont & lines );
};

/*-------------------------------------------------------------------*/
/*!

*/
MarkerID
LocalizeImpl::getNearestMarker( const VisualSensor::ObjectType objtype,
                                const Vector2D & mypos ) const
{
    // check closest behind goal
    if ( objtype == VisualSensor::Obj_Goal_Behind )
    {
        if ( mypos.x < 0.0 )
        {
            return ( M_our_side == LEFT ? Goal_L : Goal_R );
        }
        else
        {
            return ( M_our_side == LEFT ? Goal_R : Goal_L );
        }
    }

    // check nearest behind flag

    // Magic Number (related visible_distance and marker's space)
    //double mindist2 = 2.4 * 2.4
    double mindist2 = 3.0 * 3.0;

    MarkerID candidate = Marker_Unknown;

    const MarkerMap::const_iterator end = objectTable().landmarkMap().end();
    for ( MarkerMap::const_iterator it = objectTable().landmarkMap().begin();
          it != end;
          ++it )
    {
        double d2 = mypos.dist2( it->second );
        if ( d2 < mindist2 )
        {
            mindist2 = d2;
            candidate = it->first;
        }
    }

    dlog.addText( Logger::WORLD,
                  "localizer.getClosesetMarker. candidate = %d",
                  candidate );
    return candidate;
}

/*-------------------------------------------------------------------*/
/*!

*/
double
LocalizeImpl::getFaceDirByLines( const VisualSensor::LineCont & lines )
{
    if ( lines.empty() )
    {
        dlog.addText( Logger::WORLD,
                      "localizer.getFaceDirFromLines. no lines!!" );
        return VisualSensor::DIR_ERR;
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  lines must be sorted by distance from self.
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    double angle = lines.front().dir_;

#ifdef OLD_DIR_ROUND
    if ( angle > 0.0 ) angle += 0.5;
    if ( angle < 0.0 ) angle -= 0.5;
#endif

    if ( angle < 0.0 )
    {
        angle += 90.0;
    }
    else
    {
        angle -= 90.0;
    }

    switch ( lines.front().id_ ) {
    case Line_Left:
        angle = 180.0 - angle;
        break;
    case Line_Right:
        angle = 0.0 - angle;
        break;
    case Line_Top:
        angle = -90.0 - angle;
        break;
    case Line_Bottom:
        angle = 90.0 - angle;
        break;
    default:
        std::cerr << __FILE__ << ": " << __LINE__
                  << " Invalid line type " << lines.front().id_
                  << std::endl;
        return angle;
    }

    // out of field
    if ( lines.size() >= 2 )
    {
        angle += 180.0;
    }

    // right side
    if ( M_our_side == RIGHT )
    {
        angle += 180.0;
    }

    angle = AngleDeg::normalize_angle( angle );

    return angle;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LocalizeImpl::updateParticlesByMarkers( const VisualSensor::MarkerCont & markers,
                                        const double & self_face,
                                        const double & self_face_err )
{
    // must check marker container is NOT empty.

    const VisualSensor::MarkerCont::const_iterator end = markers.end();
    VisualSensor::MarkerCont::const_iterator marker = markers.begin();

    // start from second nearest marker,
    // because first marker is used for particle generation
    ++marker;

    int count = 0;
    for ( ; marker != end; ++marker )
    {
        // consider only 10 markers
        ++count;
        if ( count > 10 )   // Magic Number
        {
            break;
        }

        updateParticlesBy( *marker, marker->id_,
                           self_face, self_face_err );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LocalizeImpl::
updateParticlesByBehindMarker( const VisualSensor::MarkerCont & behind_markers,
                               const Vector2D & self_pos,
                               const double & self_face,
                               const double & self_face_err )
{
    ////////////////////////////////////////////////////////////////////
    // estimate mypos using CLOSE behind markers
    if ( behind_markers.empty() )
    {
        // nothing to do
        return;
    }

    // matching behind marker
    MarkerID markerid
        = getNearestMarker( behind_markers.front().object_type_, self_pos );

    if ( markerid == Marker_Unknown )
    {
        dlog.addText( Logger::WORLD,
                      "localizer.updatePartilesByBehindMarker. "
                      " failed to find  BEHIND marker Id" );
        return;
    }

    ////////////////////////////////////////////////////////////////////
    // update particles using closest behind marker's sector
    dlog.addText( Logger::WORLD,
                  "localizer.updatePartilesByBehindMarker."
                  " update by BEHIND marker" );

    updateParticlesBy( behind_markers.front(),
                       markerid,
                       self_face, self_face_err );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LocalizeImpl::updateParticlesBy( const VisualSensor::MarkerT & marker,
                                 const MarkerID id,
                                 const double & self_face,
                                 const double & self_face_err )
{
    ////////////////////////////////////////////////////////////////////
    // get marker global position
    MarkerMap::const_iterator it = objectTable().landmarkMap().find( id );
    if ( it == objectTable().landmarkMap().end() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " why cannot find nearest behind marker id ??"
                  << std::endl;
        dlog.addText( Logger::WORLD,
                      "localizer.updatePartilesByBehindMarker."
                      " why cannot find CLOSE behind marker id ??" );
        return;
    }

    const Vector2D & marker_pos = it->second;

    ////////////////////////////////////////////////////////////////////
    // get polar range info
    double ave_dist, dist_error;

    // get distance range info
    if ( ! objectTable().getStaticObjInfo( marker.dist_,
                                           &ave_dist,
                                           &dist_error ) )
    {
        std::cerr << "localizer.updateParticles. unexpected marker distance "
                  << marker.dist_ << std::endl;
        dlog.addText( Logger::WORLD,
                      "localizer.updateParticles. unexpected marker distance = %f",
                      marker.dist_ );
        return;
    }

    // get dir range info
    double ave_dir, dir_error;
    getDirRange( marker.dir_, self_face, self_face_err, &ave_dir, &dir_error );
    // reverse, because base point calculated in above function is marker point.
    ave_dir += 180.0;

    ////////////////////////////////////////////////////////////////////
    // create candidate sector
    const Sector2D sector( marker_pos, // base point
                           ave_dist - dist_error, // min dist
                           ave_dist + dist_error, // max dist
                           AngleDeg( ave_dir - dir_error ), // start left angle
                           AngleDeg( ave_dir + dir_error ) ); // end right angle

#if 0
    // display candidate area
    Vector2D
        v1( 1, sector.radiusMax(), sector.angleLeftStart() ),
        v2( 1, sector.radiusMax(), sector.angleRightEnd() ),
        v3( 1, sector.radiusMin(), sector.angleLeftStart() ),
        v4( 1, sector.radiusMin(), sector.angleRightEnd() );
    v1 += marker_pos;
    v2 += marker_pos;
    v3 += marker_pos;
    v4 += marker_pos;

    //dlog.addLine( v1, v2 );
    //dlog.addLine( v2, v4 );
    //dlog.addLine( v4, v3 );
    //dlog.addLine( v3, v1 );
#endif
#if 0
    dlog.addText( Logger::WORLD,
                  "localizer.updateParticles. marker_pos(%f, %f)"
                  " dist= %f, dist_range= %f"
                  " dir= %f, dir_range= %f",
                  marker_pos.x, marker_pos.y,
                  ave_dist, dist_error * 2.0,
                  ave_dir, dir_error * 2.0 );
#endif
    ////////////////////////////////////////////////////////////////////
    // check whether particles are within candidate sector
    // not contained particles are erased from container.
    M_particles.erase
        ( std::remove_if( M_particles.begin(),
                          M_particles.end(),
                          std::not1( Vector2D::IsWithin< Sector2D >( sector ) ) ),
          M_particles.end() );

    if ( M_particles.empty() )
    {
        dlog.addText( Logger::WORLD,
                      "localizer.updateParticles. no valid partilce??  marker_dist= %f"
                      " --> regenerate...",
                      marker.dist_ );
        generateParticles( marker, id, self_face, self_face_err );
        return;
    }

    ////////////////////////////////////////////////////////////////////
    // generate additional particles using valid particles coordinate
    // x & y are generated independently.
    // result may not be within current candidate sector
    std::size_t within_count = M_particles.size();
    if ( within_count < 30 )
    {
        UniformReal xy_rand( -0.04, 0.04 );
        if ( within_count == 1 )
        {
            dlog.addText( Logger::WORLD,
                          "localizer.updateParticles. only one particle. regenerate randomly" );
            for ( int i = 0; i < 9; ++i )
            {
                M_particles.push_back( M_particles[0]
                                       + Vector2D( xy_rand(), xy_rand() ) );
            }
        }
        else
        {
            UniformSmallInt idx_rand( 0, within_count - 1 );
            for ( int i = 0; i < 9; ++i )
            {
                M_particles.push_back( M_particles[idx_rand()]
                                       + Vector2D( xy_rand(), xy_rand() ) );
            }
        }
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
LocalizeImpl::getAverageOfParticles( Vector2D * ave_pos,
                                     Vector2D * ave_err )
{
    ave_pos->assign( 0.0, 0.0 );
    ave_err->assign( 0.0, 0.0 );

    if ( M_particles.empty() )
    {
        dlog.addText( Logger::WORLD,
                      "localizer.getAverageParticle. Empty!." );
        return;
    }

    dlog.addText( Logger::WORLD,
                  "localizer.getAverageParticle. rest %d particles.",
                  M_particles.size() );

    double max_x, min_x, max_y, min_y;

    max_x = min_x = M_particles.front().x;
    max_y = min_y = M_particles.front().y;

    int count = 0;
    const std::vector< Vector2D >::const_iterator end = M_particles.end();
    for ( std::vector< Vector2D >::const_iterator it = M_particles.begin();
          it != end;
          ++it, ++count )
    {
        *ave_pos += *it;
#if 0
        // display particles
        dlog..addLine( *it,
                       *it + Vector2D( 0.01, 0.0 ) );
#endif
        if ( it->x > max_x )
        {
            max_x = it->x;
        }
        else if ( it->x < min_x )
        {
            min_x = it->x;
        }

        if ( it->y > max_y )
        {
            max_y = it->y;
        }
        else if ( it->y < min_y )
        {
            min_y = it->y;
        }
    }

    *ave_pos /= static_cast< double >( count );

    dlog.addText( Logger::WORLD,
                  "localizer.getAverageParticle. self_pos=(%.3f, %.3f)"
                  "  err_x_range=(%.3f, %.3f)  err_y_range(%.3f, %.3f)",
                  ave_pos->x, ave_pos->y,
                  min_x, max_x, min_y, max_y );

    ave_err->x = ( max_x - min_x ) * 0.5;
    ave_err->y = ( max_y - min_y ) * 0.5;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
LocalizeImpl::generateParticles( const VisualSensor::MarkerT & marker,
                                 const MarkerID id,
                                 const double & self_face,
                                 const double & self_face_err )

{
    // marker must be closest one.

    ////////////////////////////////////////////////////////////////////
    // clear old particles
    M_particles.clear();


    ////////////////////////////////////////////////////////////////////
    // get closest marker info

    MarkerMap::const_iterator mapit = objectTable().landmarkMap().find( id );
    if ( mapit == objectTable().landmarkMap().end() )
    {
        std::cerr <<"localizer.generateParticles. cannot find marker id ??"
                  << std::endl;
        return;
    }

    const Vector2D marker_pos = mapit->second;

    ////////////////////////////////////////////////////////////////////
    // get sector range

    double ave_dist, dist_error;

    if ( ! objectTable().getStaticObjInfo( marker.dist_,
                                           &ave_dist, &dist_error ) )
    {
        std::cerr << "localizer.generateParticle. marker dist error"
                  << std::endl;
        return;
    }

    double ave_dir, dir_error;
    getDirRange( marker.dir_,
                 self_face, self_face_err,
                 &ave_dir, &dir_error );

    // reverse dir, because base point is marker point
    ave_dir += 180.0;

#if 1
    const double min_dist = ave_dist - dist_error;
    const double dist_range = dist_error * 2.0;
    const double dir_range = dir_error * 2.0;
    const double circum = 2.0 * ave_dist * M_PI * ( dir_range / 360.0 );
    const double dir_divs = std::min( 18.0, circum / 0.045 );
    const double dir_inc = dir_range / dir_divs;
    AngleDeg base_angle( ave_dir - dir_error ); // left first;

    for ( double add_dir = 0.0;
          add_dir < dir_range + 0.01;
          add_dir += dir_inc )
    {
        Vector2D base_vec = Vector2D::polar2vector( min_dist, base_angle );
        for ( double add_dist = 0.0;
              add_dist < dist_range + 0.08;
              add_dist += 0.045 )
        {
            M_particles.push_back( marker_pos
                                   + base_vec.setLength( min_dist + add_dist ) );
        }
        base_angle += dir_inc;
    }
    dlog.addText( Logger::WORLD,
                  "localizer.generateParticles. generate %d, dir_inc= %.1f, marker_dist= %.2f, "
                  "  dist_range= %.1f,  circum= %.2f,  dir_divs= %.1f",
                  M_particles.size(), dir_inc, ave_dist,
                  dist_range, circum, dir_divs );

#else
    const double min_dist = ave_dist - dist_error;
    const double dist_range = dist_error * 2.0;
    const double dir_range = dir_error * 2.0;
    const double circum = 2.0 * ave_dist * M_PI * ( dir_range / 360.0 );
    const double dir_inc = dir_range / ( circum / 0.03 );

    const AngleDeg left_first( ave_dir - dir_error );
    for ( double add_dir = 0.0; add_dir <= dir_range; add_dir += dir_inc )
    {
        Vector2D base_vec(1, min_dist, left_first + add_dir);
        for ( double add_dist = 0.0; add_dist <= dist_range; add_dist += 0.03 )
        {
            M_particles.push_back( marker_pos
                                   + base_vec.normalize( min_dist + add_dist ) );
        }
    }
    if ( M_particles.empty() )
    {
        dlog.addText( Logger::WORLD,
                      "localizer.generateParticles. no generation!! create only one" );
        M_particles.push_back( marker_pos
                               + Vector2D::polar2vector( ave_dist, AngleDeg( ave_dir ) ) );
    }
    dlog.addText( Logger::WORLD,
                  "localizer.generateParticles. generate %d,  dir_inc= %f",
                  M_particles.size(), dir_inc );
#endif

#if 0
    dlog.addText( Logger::WORLD,
                  "localizer.generateParticle. base_marker=(%f, %f)  dist= %f  range= %f"
                  "  dir= %f  range= %f",
                  marker_pos.x, marker_pos.y, ave_dist, dist_range,
                  ave_dir, dir_range );
    dlog.addText( Logger::WORLD,
                  "localizer.generateParticle. first particle (%f, %f)",
                  M_particles.front().x, M_particles.front().y );
#endif
}


/*-------------------------------------------------------------------*/
/*!

*/
void
LocalizeImpl::getDirRange( const double & seen_dir,
                           const double & self_face,
                           const double & self_face_err,
                           double * average,
                           double * err )
{
#ifdef OLD_DIR_ROUND

    if ( seen_dir == 0.0 ) *average = 0.0;
    if ( seen_dir > 0.0 )  *average = AngleDeg::normalize_angle(seen_dir + 0.5);
    if ( seen_dir < 0.0 )  *average = AngleDeg::normalize_angle(seen_dir - 0.5);

    *err = 0.5;
    if ( seen_dir == 0.0 ) *err = 1.0;

#else

    *average = seen_dir;
    *err = 0.5;

#endif

    *average += self_face;
    *err += self_face_err;
}

/*-------------------------------------------------------------------*/
/*!

*/
double
LocalizeImpl::getFaceDirByMarkers( const VisualSensor::MarkerCont & markers )
{
    double angle = VisualSensor::DIR_ERR;

    // get my face from two seen markers
    if ( markers.size() < 2 )
    {
        dlog.addText( Logger::WORLD,
                      "localizer.getFaceDirByMarkers. marker size less than 2."
                      " cannot get self face" );
        return angle;
    }

    dlog.addText( Logger::WORLD,
                  "localizer.getFaceDirByMarkers. try to get face from 2 markers" );

    MarkerMap::const_iterator it1 = objectTable().landmarkMap().find( markers.front().id_ );
    if ( it1 == objectTable().landmarkMap().end() )
    {
        dlog.addText( Logger::WORLD,
                      "localizer.getFaceDirByMarkers. cannot get marker1" );
        return angle;
    }

    MarkerMap::const_iterator it2 = objectTable().landmarkMap().find( markers.back().id_ );
    if ( it2 == objectTable().landmarkMap().end() )
    {
        dlog.addText( Logger::WORLD,
                      "localizer.getFaceDirByMarkers. cannot get marker2" );
        return angle;
    }

    double marker_dist1, marker_dist2, tmperr;

    if ( ! objectTable().getStaticObjInfo( markers.front().dist_,
                                           &marker_dist1,
                                           &tmperr ) )
    {
        dlog.addText( Logger::WORLD,
                      "localizer.getFaceDirByMarkers. cannot get face(3)" );
        return angle;
    }
    if ( ! objectTable().getStaticObjInfo( markers.back().dist_,
                                           &marker_dist2,
                                           &tmperr ) )
    {
        dlog.addText( Logger::WORLD,
                      "localizer.getFaceDirByMarkers. cannot get face(4)" );
        return angle;
    }

    Vector2D mrpos1
        = Vector2D::polar2vector( marker_dist1, markers.front().dir_ );
    Vector2D mrpos2
        = Vector2D::polar2vector( marker_dist2, markers.back().dir_ );
    Vector2D gap1 = mrpos1 - mrpos2;
    Vector2D gap2 = it1->second - it2->second;

    angle = ( gap2.th() - gap1.th() ).degree();

    dlog.addText( Logger::WORLD,
                  "localizer.getFaceDirByMarkers. get face from 2 flags. angle = %f",
                  angle );
    return angle;
}



#if 0
/*-------------------------------------------------------------------*/
/*
  get distance and distance range using rcssserver settings
*/
void
LocalizeImpl::getDistRange( const double & seen_dist,
                            const double & qstep,
                            double * average,
                            double * range )
{
    /*
      server quantize algorithm

      d1 = log( unq_dist + EPS )

      d2 = quantize( d1 , qstep )

      d3 = exp( d2 )

      quant_dist = quantize( d3, 0.1 )
    */

    /*
      unquantize (inverse quantize) algorithm

      min_d3 = (rint(quant_dist / 0.1) - 0.5) * 0.1
      max_d3 = (rint(quant_dist / 0.1) + 0.5) * 0.1

      min_d2 = log( min_d3 )
      max_d2 = log( max_d3 )

      min_d1 = (rint(min_d2 / qstep) - 0.5) * qstep
      max_d1 = (rint(min_d2 / qstep) + 0.5) * qstep

      min_d = exp( min_d1 ) - EPS
      max_d = exp( max_d1 ) - EPS

    */


    // first rint is not needed ;)

    // first +-0.5 is ignored,
    // because important error is occured in close distance case.

    double min_dist = ( std::round( std::log( seen_dist ) / qstep ) - 0.5 ) * qstep;
    min_dist = std::exp( min_dist ) - ObjectTable::SERVER_EPS;

    double max_dist = ( std::round( std::log( seen_dist ) / qstep ) + 0.5 ) * qstep;
    max_dist = std::exp( max_dist ) - ObjectTable::SERVER_EPS;

    *range = max_dist - min_dist;
    if ( *range < ObjectTable::SERVER_EPS )
    {
        *range = 0.05;
    }
    else if ( *range < 0.1 )
    {
        *range = 0.1;
    }

    *average = ( max_dist + min_dist ) * 0.5;
    if ( *average < ObjectTable::SERVER_EPS )
    {
        *average = *range * 0.5;
    }

    /*
      double tmp;
      if (min_dist)
      {
      //tmp = (rint(seen_dist / 0.1) - 0.5) * 0.1;
      //tmp = (seen_dist / 0.1 - 0.5) * 0.1;
      tmp = seen_dist;// - 0.05;
      if (tmp <= 0.0) tmp = SERVER_EPS;
      //tmp = log(tmp);
      tmp = (rint(log(tmp) / qstep) - 0.5) * qstep;
      *min_dist = exp(tmp) - SERVER_EPS;
      }

      if (max_dist)
      {
      //tmp = (rint(seen_dist / 0.1) + 0.5) * 0.1;
      //tmp = (seen_dist / 0.1 + 0.5) * 0.1;
      tmp = seen_dist;// + 0.05;
      //tmp = log(tmp);
      tmp = (rint(log(tmp) / qstep) + 0.5) * qstep;
      *max_dist = exp(tmp) - SERVER_EPS;
      }

      *range = *max_dist - *min_dist;
      if (*range < SERVER_EPS) *range = 0.05;
      else if (*range < 0.1) *range = 0.1;

      *average = (*max_dist + *min_dist) * 0.5;
      if (*average < SERVER_EPS) *average = *range * 0.5;
    */
}

#endif

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
Localization::Localization( const SideID ourside )
    : M_impl( new LocalizeImpl( ourside ) )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
Localization::~Localization()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
Localization::localizeSelf( const VisualSensor & see,
                            double * self_face,
                            double * self_face_err,
                            Vector2D * self_pos,
                            Vector2D * self_pos_err ) const
{
    // !!! NOTE !!!
    // markers must be sorted by distance from self

    dlog.addText( Logger::WORLD,
                  "localizer. start to localize self" );

    ////////////////////////////////////////////////////////////////////
    // initialize
    // self_pos must be assigned ERROR_VALUE
    self_pos->invalidate();
    self_pos_err->assign( 0.0, 0.0 );
    *self_face_err = 0.0;

    ////////////////////////////////////////////////////////////////////
    // get face dir & face error
    *self_face = M_impl->getFaceDirByLines( see.lines() );
    if ( *self_face == VisualSensor::DIR_ERR )
    {
        *self_face = M_impl->getFaceDirByMarkers( see.markers() );
        if ( *self_face == VisualSensor::DIR_ERR )
        {
            dlog.addText( Logger::WORLD,
                          "localizer.self. cannot get self face" );
            return;
        }
    }

#ifdef OLD_DIR_ROUND
    *self_face_err = ( *self_face == 0.0 ) ? 1.0 : 0.5;
#else
    *self_face_err = 0.5;
#endif

    ////////////////////////////////////////////////////////////////////
    // if no marker, we cannot estimate my position
    if ( see.markers().empty() )
    {
        dlog.addText( Logger::WORLD,
                      "localizer.self. no marker!" );
        return;
    }

    ////////////////////////////////////////////////////////////////////
    // generate particles using the nearest marker
    M_impl->generateParticles( see.markers().front(),
                               see.markers().front().id_,
                               *self_face,
                               *self_face_err );


    if ( M_impl->particles().empty() )
    {
        dlog.addText( Logger::WORLD,
                      "localizer.self. no particles! (1)" );
        return;
    }

    ////////////////////////////////////////////////////////////////////
    // update particles by known markers
    M_impl->updateParticlesByMarkers( see.markers(),
                                      *self_face,
                                      *self_face_err );

    // in order to estimate the Id of nearest behind marker,
    // it is necessary to calculate current estimation result,
    M_impl->getAverageOfParticles( self_pos, self_pos_err );

    // update particles by nearest behind marker
    M_impl->updateParticlesByBehindMarker( see.behindMarkers(),
                                           *self_pos, *self_face, *self_face_err );
    if ( M_impl->particles().empty() )
    {
        std::cerr << "localizeSelf: no particles!!" << std::endl;
        dlog.addText( Logger::WORLD,
                      "localizer.self. no particles!" );
        return;
    }

    // re-calculate average pos
    M_impl->getAverageOfParticles( self_pos, self_pos_err );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Localization::localizeBallRelative( const VisualSensor & see,
                                    const double & self_face,
                                    const double & self_face_err,
                                    Vector2D * rpos,
                                    Vector2D * rpos_err,
                                    Vector2D * rvel,
                                    Vector2D * rvel_err ) const
{
    if ( see.balls().empty() )
    {
        return;
    }

    const VisualSensor::BallT & ball = see.balls().front();

    ////////////////////////////////////////////////////////////////////
    // get polar range info
    double average_dist, dist_error;

    // dist range
    if ( ! M_impl->objectTable().getMovableObjInfo( ball.dist_,
                                                    &average_dist,
                                                    &dist_error ) )
    {
        std::cerr << "localizer.ball. unexpected ball distance "
                  << ball.dist_ << std::endl;
        dlog.addText( Logger::WORLD,
                      "localizer.ball. unexpected ball distance %f",
                      ball.dist_ );
        return;
    }

    // dir range
    double average_dir, dir_error;
    M_impl->getDirRange( ball.dir_,
                         self_face, self_face_err,
                         &average_dir, &dir_error );

    const double max_dist = average_dist + dist_error;
    const double min_dist = average_dist - dist_error;
    const AngleDeg max_ang = average_dir + dir_error;
    const AngleDeg min_ang = average_dir - dir_error;

    /*
      TRACEWM(DEBUG_STRM << "Ball seen  dist error = " << dist_error
      << "  dir error = " << dir_error << ENDL);
    */
    ////////////////////////////////////////////////////////////////////
    // get coordinate
    double ave_cos = AngleDeg::cos_deg( average_dir );
    double ave_sin = AngleDeg::sin_deg( average_dir );

    rpos->x = average_dist * ave_cos;
    rpos->y = average_dist * ave_sin;

    // get coordinate error
    double mincos = AngleDeg::cos_deg( average_dir - dir_error );
    double maxcos = AngleDeg::cos_deg( average_dir + dir_error );
    double minsin = AngleDeg::sin_deg( average_dir - dir_error );
    double maxsin = AngleDeg::sin_deg( average_dir + dir_error );


#if 0
    std::vector< double > xvec, yvec;
    xvec.push_back( max_dist * mincos );
    xvec.push_back( max_dist * maxcos );
    xvec.push_back( min_dist * mincos );
    xvec.push_back( min_dist * maxcos );

    yvec.push_back( max_dist * minsin );
    yvec.push_back( max_dist * maxsin );
    yvec.push_back( min_dist * minsin );
    yvec.push_back( min_dist * maxsin );

    rpos_err->x = ( *std::max_element( xvec.begin(), xvec.end() )
                    - *std::min_element( xvec.begin(), xvec.end() ) ) * 0.5;
    rpos_err->y = ( *std::max_element( yvec.begin(), xvec.end() )
                    - *std::min_element( yvec.begin(), yvec.end() ) ) * 0.5;
#else
    double x1 = max_dist * mincos; double x2 = max_dist * maxcos;
    double x3 = min_dist * mincos; double x4 = min_dist * maxcos;

    double y1 = max_dist * minsin; double y2 = max_dist * maxsin;
    double y3 = min_dist * minsin; double y4 = min_dist * maxsin;

    rpos_err->x = ( max( max( x1, x2 ), max( x3, x4 ) )
                    - min( min( x1, x2 ), min( x3, x4 ) ) ) * 0.5;
    rpos_err->y = ( max( max( y1, y2 ), max( y3, y4 ) )
                    - min( min( y1, y2 ), min( y3, y4 ) ) ) * 0.5;
#endif

    dlog.addText( Logger::WORLD,
                  "localize.Ball  Seen relative ball. ave_dist=%.1f ave_aangle=%.1f"
                  " pos = (%.3f %.3f) err = (%.3f %.3f)",
                  average_dist, average_dir,
                  rpos->x, rpos->y,
                  rpos_err->x, rpos_err->y );

    ////////////////////////////////////////////////////////////////////
    // get velocity
    if ( ball.has_vel_ )
    {
        double max_dist_dist_chg1
            = ( ball.dist_chng_ / ball.dist_ + 0.02*0.5 ) * max_dist;
        double max_dist_dist_chg2
            = ( ball.dist_chng_ / ball.dist_ - 0.02*0.5 ) * max_dist;

        double min_dist_dist_chg1
            = ( ball.dist_chng_ / ball.dist_ + 0.02*0.5 ) * min_dist;
        double min_dist_dist_chg2
            = ( ball.dist_chng_ / ball.dist_ - 0.02*0.5 ) * min_dist;

        // qstep_dir = 0.1
        double max_dir_chg = ball.dir_chng_ + ( 0.1 * 0.5 );
        double min_dir_chg = ball.dir_chng_ - ( 0.1 * 0.5 );

        double max_dist_dir_chg_r1 = AngleDeg::DEG2RAD * max_dir_chg * max_dist;
        double max_dist_dir_chg_r2 = AngleDeg::DEG2RAD * min_dir_chg * max_dist;

        double min_dist_dir_chg_r1 = AngleDeg::DEG2RAD * max_dir_chg * min_dist;
        double min_dist_dir_chg_r2 = AngleDeg::DEG2RAD * min_dir_chg * min_dist;

        // relative vel pattern : max_dist case
        Vector2D rvel1_1( max_dist_dist_chg1, max_dist_dir_chg_r1 ); rvel1_1.rotate( max_ang );
        Vector2D rvel1_2( max_dist_dist_chg1, max_dist_dir_chg_r1 ); rvel1_2.rotate( min_ang );
        Vector2D rvel2_1( max_dist_dist_chg1, max_dist_dir_chg_r2 ); rvel2_1.rotate( max_ang );
        Vector2D rvel2_2( max_dist_dist_chg1, max_dist_dir_chg_r2 ); rvel2_2.rotate( min_ang );
        Vector2D rvel3_1( max_dist_dist_chg2, max_dist_dir_chg_r1 ); rvel3_1.rotate( max_ang );
        Vector2D rvel3_2( max_dist_dist_chg2, max_dist_dir_chg_r1 ); rvel3_2.rotate( min_ang );
        Vector2D rvel4_1( max_dist_dist_chg2, max_dist_dir_chg_r2 ); rvel4_1.rotate( max_ang );
        Vector2D rvel4_2( max_dist_dist_chg2, max_dist_dir_chg_r2 ); rvel4_2.rotate( min_ang );
        // relative vel pattern : min_dist case
        Vector2D rvel5_1( min_dist_dist_chg1, min_dist_dir_chg_r1 ); rvel5_1.rotate( max_ang );
        Vector2D rvel5_2( min_dist_dist_chg1, min_dist_dir_chg_r1 ); rvel5_2.rotate( min_ang );
        Vector2D rvel6_1( min_dist_dist_chg1, min_dist_dir_chg_r2 ); rvel6_1.rotate( max_ang );
        Vector2D rvel6_2( min_dist_dist_chg1, min_dist_dir_chg_r2 ); rvel6_2.rotate( min_ang );
        Vector2D rvel7_1( min_dist_dist_chg2, min_dist_dir_chg_r1 ); rvel7_1.rotate( max_ang );
        Vector2D rvel7_2( min_dist_dist_chg2, min_dist_dir_chg_r1 ); rvel7_2.rotate( min_ang );
        Vector2D rvel8_1( min_dist_dist_chg2, min_dist_dir_chg_r2 ); rvel8_1.rotate( max_ang );
        Vector2D rvel8_2( min_dist_dist_chg2, min_dist_dir_chg_r2 ); rvel8_2.rotate( min_ang );


        double max_x = max(max(max(max(rvel1_1.x, rvel1_2.x), max(rvel2_1.x, rvel2_2.x)),
                               max(max(rvel3_1.x, rvel3_2.x), max(rvel4_1.x, rvel4_2.x))),
                           max(max(max(rvel5_1.x, rvel5_2.x), max(rvel6_1.x, rvel6_2.x)),
                               max(max(rvel7_1.x, rvel7_2.x), max(rvel8_1.x, rvel8_2.x))));
        double max_y = max(max(max(max(rvel1_1.y, rvel1_2.y), max(rvel2_1.y, rvel2_2.y)),
                               max(max(rvel3_1.y, rvel3_2.y), max(rvel4_1.y, rvel4_2.y))),
                           max(max(max(rvel5_1.y, rvel5_2.y), max(rvel6_1.y, rvel6_2.y)),
                               max(max(rvel7_1.y, rvel7_2.y), max(rvel8_1.y, rvel8_2.y))));

        double min_x = min(min(min(min(rvel1_1.x, rvel1_2.x), min(rvel2_1.x, rvel2_2.x)),
                               min(min(rvel3_1.x, rvel3_2.x), min(rvel4_1.x, rvel4_2.x))),
                           min(min(min(rvel5_1.x, rvel5_2.x), min(rvel6_1.x, rvel6_2.x)),
                               min(min(rvel7_1.x, rvel7_2.x), min(rvel8_1.x, rvel8_2.x))));
        double min_y = min(min(min(min(rvel1_1.y, rvel1_2.y), min(rvel2_1.y, rvel2_2.y)),
                               min(min(rvel3_1.y, rvel3_2.y), min(rvel4_1.y, rvel4_2.y))),
                           min(min(min(rvel5_1.y, rvel5_2.y), min(rvel6_1.y, rvel6_2.y)),
                               min(min(rvel7_1.y, rvel7_2.y), min(rvel8_1.y, rvel8_2.y))));


        Vector2D ave_rvel = rvel1_1; ave_rvel += rvel1_2;
        ave_rvel += rvel2_1; ave_rvel += rvel2_2;
        ave_rvel += rvel3_1; ave_rvel += rvel3_2;
        ave_rvel += rvel4_1; ave_rvel += rvel4_2;
        ave_rvel += rvel5_1; ave_rvel += rvel5_2;
        ave_rvel += rvel6_1; ave_rvel += rvel6_2;
        ave_rvel += rvel7_1; ave_rvel += rvel7_2;
        ave_rvel += rvel8_1; ave_rvel += rvel8_2;

        ave_rvel /= 16.0;

        *rvel = ave_rvel;
        // gvel = rvel + myvel
        rvel_err->assign( (max_x - min_x) * 0.5,
                          (max_y - min_y) * 0.5 );
        // gvel_err = rvel_err + myvel_err
#ifdef DEBUG
        {
            Vector2D raw_rvel( ball.dist_chng_,
                               AngleDeg::DEG2RAD * ball.dir_chng_ );
            raw_rvel.rotate( average_dir );
            dlog.addText( Logger::WORLD,
                          "localize.Ball  Seen raw relative ball vel = (%.3f %.3f)",
                          raw_rvel.x, raw_rvel.y );
        }
#endif
        dlog.addText( Logger::WORLD,
                      "localize.Ball  Seen rel ball vel = (%.3f %.3f) err = (%.3f %.3f)",
                      ave_rvel.x, ave_rvel.y,
                      rvel_err->x, rvel_err->y );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Localization::localizePlayer( const VisualSensor::PlayerT & from,
                              const double & self_face,
                              const double & self_face_err,
                              const Vector2D & self_pos,
                              const Vector2D & self_vel,
                              PlayerT * to ) const
{
    ////////////////////////////////////////////////////////////////////
    // get polar range info
    double average_dist, dist_error;

    if ( ! M_impl->objectTable().getMovableObjInfo( from.dist_,
                                                    &average_dist,
                                                    &dist_error ) )
    {
        std::cerr << "localize.player. Unexpected player distance "
                  << from.dist_ << std::endl;
        dlog.addText( Logger::WORLD,
                      "localize.player. Unexpected player distance %f",
                      from.dist_ );
        return;
    }

    double average_dir, dir_error;

    M_impl->getDirRange( from.dir_,
                         self_face, self_face_err,
                         &average_dir, &dir_error );


    ////////////////////////////////////////////////////////////////////
    // set player info
    to->unum_ = from.unum_;
    to->goalie_ = from.goalie_;


    ////////////////////////////////////////////////////////////////////
    // get coordinate
    to->rpos_.x = average_dist * AngleDeg::cos_deg( average_dir );
    to->rpos_.y = average_dist * AngleDeg::sin_deg( average_dir );

    // ignore error

    // set global coordinate
    to->pos_ = self_pos + to->rpos_;


    ////////////////////////////////////////////////////////////////////
    // get vel
    // use only seen info, not consider noise
    if ( from.has_vel_ )
    {
#if 1
        to->vel_.assign( from.dist_chng_,
                         AngleDeg::DEG2RAD * from.dir_chng_ * average_dist );
        to->vel_.rotate( average_dir );
        to->vel_ += self_vel;
#else
        to->vel_.x = self_vel.x + ( from.dist_chng_ * to->rpos.x / average_dist
                                    - AngleDeg::DEG2RAD * from.dir_chng_ * to->rpos.y );
        to->vel_.y = self_vel.y + ( from.dist_chng_ * to->rpos.y / average_dist
                                    + AngleDeg::DEG2RAD * from.dir_chng_ * to->rpos.x );
#endif
    }
    else
    {
        to->vel_.invalidate();
    }

    ////////////////////////////////////////////////////////////////////
    // get player body & neck global angle
    to->has_face_ = false;
    if ( from.body_ != VisualSensor::DIR_ERR
         && from.face_ != VisualSensor::DIR_ERR )
    {
        to->has_face_ = true;
        to->body_ = AngleDeg::normalize_angle( from.body_ + self_face );
        to->face_ = AngleDeg::normalize_angle( from.face_ + self_face );
    }


    ////////////////////////////////////////////////////////////////////
    // get pointto info
    to->pointto_ = false;
    if ( from.arm_ != VisualSensor::DIR_ERR )
    {
        to->pointto_ = true;
        to->arm_ = AngleDeg::normalize_angle( from.arm_ + self_face );
    }

    ////////////////////////////////////////////////////////////////////
    // get tackle info
    to->tackle_ = from.tackle_;
}

}
