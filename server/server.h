
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQSERVER_SERVER_H
#define EQSERVER_SERVER_H

#include "base.h"
#include "types.h"
#include "visitorResult.h" // enum

#include <eq/client/nodeType.h>  // for TYPE_EQ_SERVER enum

#include <eq/net/command.h>      // used in inline method
#include <eq/net/commandQueue.h> // member
#include <eq/net/node.h>         // base class & eqsStartLocalServer declaration

namespace eq
{
namespace fabric
{
    template< class S, class C, class O, class L, class CV > class Config;
}
namespace server
{
    class ServerVisitor;

    /**
     * The Equalizer server.
     */
    class Server : public net::Node
    {
    public:
        /** 
         * Constructs a new Server.
         */
        EQSERVER_EXPORT Server();

        /** 
         * Runs the server.
         * 
         * @return <code>true</code> if the server did run successfully,
         *         <code>false</code> if not.
         */
        EQSERVER_EXPORT bool run();

        void registerConfig( Config* config );
        
        /** @return the vector of configurations. */
        const ConfigVector& getConfigs() const { return _configs; }

        /** Delete all configs of this server (exit). */
        EQSERVER_EXPORT void deleteConfigs();

        /** @return the command queue to the server thread */
        net::CommandQueue* getServerThreadQueue() 
            { return &_serverThreadQueue; }

        /** 
         * Traverse this server and all children using a server visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        EQSERVER_EXPORT VisitorResult accept( ServerVisitor& visitor );
        EQSERVER_EXPORT VisitorResult accept( ServerVisitor& visitor ) const;

        /** @return the global time in milliseconds. */
        int64_t getTime() const { return _clock.getTime64(); }

    protected:
        virtual ~Server();

        /** @sa net::Node::dispatchCommand */
        virtual bool dispatchCommand( net::Command& command );

        /** @sa net::Node::invokeCommand */
        virtual net::CommandResult invokeCommand( net::Command& command );
        
    private:
        /** The unique config identifier. */
        uint32_t _configID;

        /** The list of configurations. */
        ConfigVector _configs;

        typedef base::UUIDHash< Config* > ConfigHash;
        /** The application-allocated configurations, mapped by identifier. */
        ConfigHash _appConfigs;

        /** The receiver->main command queue. */
        net::CommandQueue    _serverThreadQueue;

        /** The global clock. */
        base::Clock _clock;

        /** The current state. */
        bool _running;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        /** @sa net::Node::getType */
        virtual uint32_t getType() const { return eq::TYPE_EQ_SERVER; }

        friend class fabric::Config< Server, Config, Observer, Layout, Canvas >;

        /**  Add a new config to this server. */
        void _addConfig( Config* config );

        /** Remove a config from this server. */
        bool _removeConfig( Config* config );

        void        _handleCommands(); 

        /** The command functions. */
        net::CommandResult _cmdChooseConfig( net::Command& command );
        net::CommandResult _cmdReleaseConfig( net::Command& command );
        net::CommandResult _cmdShutdown( net::Command& command );
    };

    EQSERVER_EXPORT std::ostream& operator << ( std::ostream&, const Server* );
}
}
#endif // EQSERVER_SERVER_H
