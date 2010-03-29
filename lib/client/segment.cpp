
/* Copyright (c) 2009-2010, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "segment.h"

#include "canvas.h"
#include "channel.h"
#include "config.h"

namespace eq
{

typedef fabric::Segment< Canvas, Segment > Super;

Segment::Segment( Canvas* parent )
        : Super( parent )
{
}

Segment::Segment( const Segment& from, Canvas* parent )
        : fabric::Segment< Canvas, Segment >( from, parent )
{ EQDONTCALL; }

Segment::~Segment()
{
}

Config* Segment::getConfig()
{
    EQASSERT( getCanvas() );
    return getCanvas() ? getCanvas()->getConfig() : 0;
}

const Config* Segment::getConfig() const
{
    EQASSERT( getCanvas() );
    return getCanvas() ? getCanvas()->getConfig() : 0;
}

}

#include "../fabric/segment.cpp"
template class eq::fabric::Segment< eq::Canvas, eq::Segment >;
/** @cond IGNORE */
template std::ostream& eq::fabric::operator << ( std::ostream&,
                        const eq::fabric::Segment< eq::Canvas, eq::Segment >& );
/** @endcond */
