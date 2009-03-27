
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

#ifndef EQ_NODEVISITOR_H
#define EQ_NODEVISITOR_H

#include <eq/client/pipeVisitor.h> // base class

namespace eq
{
    class Node;

    /**
     * A visitor to traverse non-const nodes and children.
     */
    class NodeVisitor : public PipeVisitor
    {
    public:
        /** Constructs a new NodeVisitor. */
        NodeVisitor(){}
        
        /** Destruct the NodeVisitor */
        virtual ~NodeVisitor(){}

        /** Visit a node on the down traversal. */
        virtual VisitorResult visitPre( Node* node )
            { return TRAVERSE_CONTINUE; }

        /** Visit a node on the up traversal. */
        virtual VisitorResult visitPost( Node* node )
            { return TRAVERSE_CONTINUE; }
    };
}
#endif // EQ_NODEVISITOR_H
