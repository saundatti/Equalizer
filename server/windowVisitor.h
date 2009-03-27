
/* Copyright (c) 2008-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifndef EQSERVER_WINDOWVISITOR_H
#define EQSERVER_WINDOWVISITOR_H

#include "channelVisitor.h"

namespace eq
{
namespace server
{
    class Window;

    /**
     * A visitor to traverse non-const windows and children.
     */
    class WindowVisitor : public ChannelVisitor
    {
    public:
        /** Constructs a new WindowVisitor. */
        WindowVisitor(){}
        
        /** Destruct the WindowVisitor */
        virtual ~WindowVisitor(){}

        /** Visit a window on the down traversal. */
        virtual VisitorResult visitPre( Window* window )
            { return TRAVERSE_CONTINUE; }

        /** Visit a window on the up traversal. */
        virtual VisitorResult visitPost( Window* window )
            { return TRAVERSE_CONTINUE; }
    };

    /**
     * A visitor to traverse const windows and children.
     */
    class ConstWindowVisitor : public ConstChannelVisitor
    {
    public:
        /** Constructs a new WindowVisitor. */
        ConstWindowVisitor(){}
        
        /** Destruct the WindowVisitor */
        virtual ~ConstWindowVisitor(){}

        /** Visit a window on the down traversal. */
        virtual VisitorResult visitPre( const Window* window )
            { return TRAVERSE_CONTINUE; }

        /** Visit a window on the up traversal. */
        virtual VisitorResult visitPost( const Window* window )
            { return TRAVERSE_CONTINUE; }
    };
}
}
#endif // EQSERVER_WINDOWVISITOR_H
