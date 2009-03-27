
/* Copyright (c) 2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "nodeFactory.h"

#include "canvas.h"
#include "channel.h"
#include "client.h"
#include "config.h"
#include "layout.h"
#include "node.h"
#include "pipe.h"
#include "segment.h"
#include "server.h"
#include "view.h"
#include "window.h"

namespace eq
{

Config* NodeFactory::createConfig( ServerPtr parent )
{
    return new Config( parent );
}
void NodeFactory::releaseConfig( Config* config )
{
    delete config;
}

Node* NodeFactory::createNode( Config* parent )
{
    return new Node( parent );
}
void NodeFactory::releaseNode( Node* node )
{
    delete node;
}

Canvas* NodeFactory::createCanvas()
{
    return new Canvas();
}
void NodeFactory::releaseCanvas( Canvas* canvas )
{
    delete canvas;
}

Segment* NodeFactory::createSegment()
{
    return new Segment();
}
void NodeFactory::releaseSegment( Segment* segment )
{
    delete segment;
}

Layout* NodeFactory::createLayout()
{
    return new Layout();
}
void NodeFactory::releaseLayout( Layout* layout )
{
    delete layout;
}

View* NodeFactory::createView()
{
    return new View();
}
void NodeFactory::releaseView( View* view )
{
    delete view;
}

Pipe* NodeFactory::createPipe( Node* parent )
{
    return new Pipe( parent );
}
void NodeFactory::releasePipe( Pipe* pipe )
{
    delete pipe;
}

Window* NodeFactory::createWindow( Pipe* parent )
{
    return new Window( parent );
}
void NodeFactory::releaseWindow( Window* window )
{
    delete window;
}

Channel* NodeFactory::createChannel( Window* parent )
{
    return new Channel( parent );
}

void NodeFactory::releaseChannel( Channel* channel )
{
    delete channel;
}

}

