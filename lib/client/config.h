
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_CONFIG_H
#define EQ_CONFIG_H

#include <eq/client/commandQueue.h>  // member
#include <eq/client/matrix4.h>       // member
#include <eq/client/types.h>         // typedefs
#include <eq/client/visitorResult.h> // enum

#include <eq/net/session.h>          // base class
#include <eq/base/monitor.h>         // member

namespace eq
{
    class ConfigDeserializer;
    class ConfigVisitor;
    class Node;
    class SceneObject;
    struct ConfigEvent;

    /**
     * The configuration groups all processes of the application in a single
     * net::Session.
     */
    class EQ_EXPORT Config : public net::Session
    {
    public:
        /** Constructs a new config. */
        Config( ServerPtr parent );

        /** Destructs a config. */
        virtual ~Config();

        /** @name Data Access */
        //*{
        ClientPtr getClient();
        ServerPtr getServer();

        CommandQueue* getNodeThreadQueue();

        uint32_t getCurrentFrame()  const { return _currentFrame; }
        uint32_t getFinishedFrame() const { return _finishedFrame.get(); }

        /** @return the vector of nodes instantiated on this process. */
        const NodeVector& getNodes() const { return _nodes; }

        /** @return the vector of layouts, app-node only. */
        const LayoutVector& getLayouts() const { return _layouts; }

        /** @return the layout of the given identifier, or 0. */
        Layout* findLayout( const uint32_t id );

        /** @return the view of the given identifier, or 0. */
        View* findView( const uint32_t id );

        /** @return the vector of canvases, app-node only. */
        const CanvasVector& getCanvases() const { return _canvases; }

        /** @return the name of this config. */
        const std::string& getName() const { return _name; }

        /** 
         * Traverse this config and all children using a config visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( ConfigVisitor& visitor );

        /** Get all received statistics. */
        void getStatistics( std::vector< FrameStatistics >& statistics );

        /**
         * @return true while the config is initialized and no exit event
         *         happened.
         */
        bool isRunning() const { return _running; }

		/** Stop running the config. */
		void stopRunning() { _running = false; }

        /** @return the global time in ms. */
        int64_t getTime() const { return _clock.getTime64(); }
        //*}

        /** @name Operations. */
        //*{
        /** 
         * Initializes this configuration.
         * 
         * @param initID an identifier to be passed to all init methods.
         * @return true if the initialization was successful, false if not.
         */
        virtual bool init( const uint32_t initID );

        /** 
         * Exits this configuration.
         * 
         * A config which could not be exited properly may not be
         * re-initialized.
         *
         * @return <code>true</code> if the exit was successful,
         *         <code>false</code> if not.
         */
        virtual bool exit();
        //*}

        /**
         * @name Frame Control
         */
        //*{
        /** 
         * Requests a new frame of rendering.
         * 
         * @param frameID a per-frame identifier passed to all rendering
         *                methods.
         * @return the frame number of the new frame.
         */
        virtual uint32_t startFrame( const uint32_t frameID );

        /** 
         * Finish the rendering of a frame.
         * 
         * @return the frame number of the finished frame, or <code>0</code> if
         *         no frame has been finished.
         */
        virtual uint32_t finishFrame();

        /**
         * Finish rendering all pending frames.
         *
         * @return the frame number of the last finished frame.
         */
        virtual uint32_t finishAllFrames();

        /** 
         * Release the local synchronization of the config for a frame.
         * 
         * This is used by the local node to release the local frame
         * synchronization.
         *
         * @param frameNumber the frame to release.
         */
        void releaseFrameLocal( const uint32_t frameNumber )
            { _unlockedFrame = frameNumber; }
        //*}

        /** @name Event handling. */
        //*{
        /** 
         * Send an event to the application node.
         * 
         * @param event the event.
         */
        void sendEvent( ConfigEvent& event );

        /** 
         * Get the next received event on the application node.
         * 
         * The returned event is valid until the next call to this method.
         * 
         * @return a config event.
         */
        const ConfigEvent* nextEvent();

        /** 
         * Try to get an event on the application node.
         * 
         * The returned event is valid until the next call to this method.
         * 
         * @return a config event, or 0 if no events are pending.
         */
        const ConfigEvent* tryNextEvent();

        /** @return true if events are pending. */
        bool checkEvent() const { return !_eventQueue.empty(); }

        /**
         * Handle all config events.
         *
         * Called at the end of each frame to handle pending config events. The
         * default implementation calls handleEvent() on all pending events,
         * without blocking.
         */
        virtual void handleEvents();

        /** 
         * Handle one config event.
         * 
         * @param event the event.
         * @return <code>true</code> if the event requires a redraw,
         *         <code>false</code> if not.
         */
        virtual bool handleEvent( const ConfigEvent* event );
        //*}
        
        /** @name Observer Interface. */
        //*{
        /** 
         * Set the head matrix.
         *
         * The head matrix specifies the transformation origin->observer.
         * Together with the eye separation, this determines the eye positions.
         * The eye position and wall or projection description define the shape
         * of the frustum and the channel's head matrix during rendering.
         *
         * @param matrix the matrix
         */
        void setHeadMatrix( const vmml::Matrix4f& matrix );

        /** @return the current head matrix. */
        const vmml::Matrix4f& getHeadMatrix() const { return _headMatrix; }

        /** Set the eye separation, i.e., the distance between the eyes. */
        void setEyeBase( const float eyeBase );

        /** @return the current eye separation. */
        float getEyeBase() const { return _eyeBase; }
        //*}

        /** @name Error Information. */
        //*{
        /** @return the error message from the last operation. */
        const std::string& getErrorMessage() const { return _error; }
        //*}

#ifdef EQ_TRANSMISSION_API
        /** @name Data Transmission. */
        //*{
        /** 
         * Send data to all active render client nodes.
         * 
         * @param data the pointer to the data.
         * @param size the data size.
         */
        void broadcastData( const void* data, uint64_t size );
        //*}
#endif

        /** Undocumented */
        void freezeLoadBalancing( const bool onOff );

        /** 
         * Set the window system for the config's message pump, used by
         * non-threaded pipes.
         * @internal
         */
        void setWindowSystem( const WindowSystem windowSystem );

    protected:

        /** @sa eq::net::Session::setLocalNode() */
        virtual void setLocalNode( net::NodePtr node );

    private:
        /** The node identifier of the node running the application thread. */
        net::NodeID _appNodeID;
        friend class Server;

        /** The name. */
        std::string _name;

        /** The node running the application thread. */
        net::NodePtr _appNode;

#ifdef EQ_TRANSMISSION_API
        /** The list of the running client node identifiers. */
        std::vector<net::NodeID> _clientNodeIDs;

        /** The running client nodes, is cleared when _clientNodeIDs changes. */
        net::NodeVector _clientNodes;
#endif

        /** Locally-instantiated nodes of this config. */
        NodeVector _nodes;

        /** The list of layouts, app-node only. */
        LayoutVector _layouts;

        /** The list of canvases, app-node only. */
        CanvasVector _canvases;

        /** The matrix describing the head position and orientation. */
        Matrix4f _headMatrix;
        friend class ConfigCommitVisitor;

        /** The distance between the left and the right eye. */
        float _eyeBase;

        /** The reason for the last error. */
        std::string _error;

        /** The receiver->app thread event queue. */
        CommandQueue _eventQueue;
        
        /** The last received event to be released. */
        net::Command* _lastEvent;

        /** Global statistics events, index per frame and channel. */
        std::deque< FrameStatistics > _statistics;
        base::SpinLock                _statisticsMutex;

        /** The maximum number of outstanding frames. */
        uint32_t _latency;

        /** The last started frame. */
        uint32_t _currentFrame;
        /** The last locally released frame. */
        uint32_t _unlockedFrame;
        /** The last completed frame. */
        base::Monitor< uint32_t > _finishedFrame;

        /** The global clock. */
        base::Clock _clock;

        /** true while the config is initialized and no window has exited. */
        bool _running;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        friend class Node;
        void _addNode( Node* node );
        void _removeNode( Node* node );
        Node* _findNode( const uint32_t id );

        friend class ConfigDeserializer;
        void _addCanvas( Canvas* canvas );
        void _removeCanvas( Canvas* canvas );
        void _addLayout( Layout* layout );
        void _removeLayout( Layout* layout );

        bool _needsLocalSync() const;

        /** 
         * Update statistics for the finished frame, push it to the local node
         * for visualization.
         */
        void _updateStatistics( const uint32_t finishedFrame );

#ifdef EQ_TRANSMISSION_API
        /** Connect client nodes of this config. */
        bool _connectClientNodes();
#endif

        /** Init the application node instance */
        void _initAppNode( const uint32_t distributorID );

        /** The command functions. */
        net::CommandResult _cmdSyncClock( net::Command& command );
        net::CommandResult _cmdCreateNode( net::Command& command );
        net::CommandResult _cmdDestroyNode( net::Command& command );
        net::CommandResult _cmdInitReply( net::Command& command );
        net::CommandResult _cmdExitReply( net::Command& command );
        net::CommandResult _cmdStartFrameReply( net::Command& command );
        net::CommandResult _cmdReleaseFrameLocal( net::Command& command );
        net::CommandResult _cmdFrameFinish( net::Command& command );
#ifdef EQ_TRANSMISSION_API
        net::CommandResult _cmdData( net::Command& command );
#endif
        net::CommandResult _cmdUnmap( net::Command& command );
    };
}

#endif // EQ_CONFIG_H

