//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2013 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


#ifndef HEADER_INTERPOLATION_ARRAY_HPP
#define HEADER_INTERPOLATION_ARRAY_HPP

#include <assert.h>
#include <vector>

/** This class manages a set of (x_i,y_i) points, x_i must be sorted.
 *  Those values are then used to linearly interpolate the y value for a
 *  given x. If x is less than the minimum x_0, y_0 is returned, if x is
 *  more than the maximum x_n, y_n is returned.
 */
class InterpolationArray
{
private:
    /** The sorted x values. */
    std::vector<float> m_x;

    /** The y values. */
    std::vector<float> m_y;

    /* Pre-computed (x[i+1]-x[i])/(y[i+1]/-y[i]) . */
    std::vector<float> m_delta;

public:
    InterpolationArray() {};

    /** Adds the value pair x/y to the list of all points. It is tested
     *  that the x values are added in order.
     *  \param x, y The pair to add.
     *  \returns 0 If the x values are not sorted, 1 otherwise. */
    int push_back(float x, float y)
    {
        if(m_x.size()>0 && x < m_x[m_x.size()-1])
            return 0;
        m_x.push_back(x);
        m_y.push_back(y);
        if(m_y.size()>1)
        {
            const unsigned int last=(unsigned int) m_x.size()-1;
            // Avoid division by zero, just set m_delta to a large
            // value with the right sign
            if(m_x[last]==m_x[last-1])
                m_delta.push_back( (m_y[last]-m_y[last-1])
                                  / 0.001f                 );
            else
                m_delta.push_back( (m_y[last]-m_y[last-1])
                                 /(m_x[last]-m_x[last-1])  );
        }
        return 1;
    }   // push_back
    // ------------------------------------------------------------------------
    /** Returns the number of X/Y points. */
    unsigned int size() const { return (unsigned int) m_x.size(); }
    // ------------------------------------------------------------------------
    /** Returns the X value for a specified point. */
    float getX(unsigned int i) const { return m_x[i]; }
    // ------------------------------------------------------------------------
    /** Returns the Y value for a specified point. */
    float getY(unsigned int i) const { return m_y[i]; }
    // ------------------------------------------------------------------------
    /** Sets the Y value for a specified point. */
    void setY(unsigned int i, float y)
    {
        m_y[i] = y;
        if(i>0)
            m_delta[i-1] = (m_y[i]-m_y[i-1])
                          /(m_x[i]-m_x[i-1]);
        if(i<m_y.size()-1)
            m_delta[i] = (m_y[i+1]-m_y[i])
                        /(m_x[i+1]-m_x[i]);
    }
    // ------------------------------------------------------------------------
    /** Returns the interpolated Y value for a given x. */
    float get(float x) const
    {
        if(m_x.size()==1 || x<m_x[0])
            return m_y[0];

        if(x>m_x[m_x.size()-1])
            return m_y[m_y.size()-1];

        // Now x must be between two points in m_x
        // The array size in STK are pretty small (typically 3 or 4),
        // so not worth the effort to do a binary search
        for(unsigned int i=1; i<m_x.size(); i++)
        {
            if(x >m_x[i]) continue;
            return m_y[i-1] + m_delta[i-1] * (x - m_x[i-1]);
        }
        assert(false); return 0;  // keep compiler happy
    }   // get

    // ------------------------------------------------------------------------
    /** Returns the X value necessary for a specified Y value. If it's not
     *  possible to find a corresponding X (y is too small or too large),
     *  x_min or x_max is returned. */
    float getReverse(float y) const
    {
        if(m_y.size()==1) return m_x[0];

        if(m_y[1]<m_y[0])   // if decreasing values
        {
            if(y > m_y[0]) return m_x[0];

            const unsigned int last = (unsigned int) m_x.size();

            for(unsigned int i=1; i<last; i++)
            {
                if(y < m_y[i]) continue;
                return m_x[i-1] + (y-m_y[i-1])/ m_delta[i-1];
            }   // for i < last
            return m_x[last-1];
        }
        else   // increasing
        {
            if(y < m_y[0]) return m_x[0];

            const unsigned int last = (unsigned int) m_x.size();

            for(unsigned int i=1; i<last; i++)
            {
                if(y > m_y[i]) continue;
                return m_x[i-1] + (y-m_y[i-1]) / m_delta[i-1];
            }   // for i < last
            return m_x[last-1];
        }   // increasing
    }   // getReverse
};    // InterpolationArray


#endif
