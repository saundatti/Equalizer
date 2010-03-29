
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

#ifndef EQSERVER_CANVAS_H
#define EQSERVER_CANVAS_H

#include "types.h"
#include "visitorResult.h"  // enum

#include <eq/fabric/canvas.h> // base class

#include <eq/base/base.h>
#include <string>

namespace eq
{
namespace fabric
{
    template< class C, class S > class Segment;
}
namespace server
{
    /** The canvas. @sa eq::Canvas */
    class Canvas : public fabric::Canvas< Config, Canvas, Segment, Layout >
    {
    public:
        /** 
         * Constructs a new Canvas.
         */
        EQSERVER_EXPORT Canvas( Config* parent );

        /** Creates a new, deep copy of a canvas. */
        Canvas( const Canvas& from, Config* parent );

        /** Destruct this canvas. */
        virtual ~Canvas();

        /**
         * @name Data Access
         */
        //@{
        /** 
         * Find the first segment of a given name.
         * 
         * @param name the name of the segment to find
         * @return the first segment with the name, or <code>0</code> if no
         *         segment with the name exists.
         */
        Segment* findSegment( const std::string& name );

        /** @return the segment of the given path. */
        Segment* getSegment( const SegmentPath& path );

        /** @return the index path to this canvas. @internal */
        CanvasPath getPath() const;
        //@}

        /**
         * @name Operations
         */
        //@{
        void init();
        void exit();

        /** Unmap this canvas and all its children. */
        void deregister();
        //@}
        
    protected:
        virtual void activateLayout( const uint32_t index );
        virtual Segment* createSegment();
        virtual void releaseSegment( Segment* segment );

    private:
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** Run-time layout switch */
        void _switchLayout( const uint32_t oldIndex, const uint32_t newIndex );
    };

}
}
#endif // EQSERVER_CANVAS_H
