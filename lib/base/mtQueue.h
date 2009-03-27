
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_MTQUEUE_H
#define EQBASE_MTQUEUE_H

#include <eq/base/base.h>
#include <eq/base/debug.h>
#include <queue>

#include <string.h>

namespace eq
{
namespace base
{
class MTQueuePrivate;

    /**
     * A queue with a blocking pop() implementation, typically used between two
     * execution threads.
     */
    template< typename T > class MTQueue
    {
    public:
        /** 
         * Constructs a new queue.
         */
        MTQueue();

        /** Destructs the Queue. */
        ~MTQueue();

        /** @return true if the queue is empty, false otherwise. */
        bool empty() const { return _queue.empty(); }

        /** @return the number of items currently in the queue. */
        size_t size() const { return _queue.size(); }

        /** Retrieve and pop the front element from the queue, may block. */
        T pop();

        /** Try to get the front element from the queue, returns 0 if empty. */
        T tryPop();

        T back() const;
        void push( const T& element );
        void pushFront( const T& element );

        static const T NONE;

    private:
        std::deque< T > _queue;

        MTQueuePrivate* _data;
    };

//----------------------------------------------------------------------
// implementation
//----------------------------------------------------------------------

// Crude test if pthread.h was included
#ifdef PTHREAD_MUTEX_INITIALIZER
#  ifndef HAVE_PTHREAD_H
#    define HAVE_PTHREAD_H
#  endif
#endif

// The application has to include pthread.h if it wants to instantiate new queue
// types, since on Windows the use of pthreads-Win32 includes might create 
// hard-to-resolve type conflicts with other header files.

#ifdef HAVE_PTHREAD_H

class MTQueuePrivate
{
public:
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
};

template< typename T > const T MTQueue<T>::NONE = 0;

template< typename T >
MTQueue<T>::MTQueue()
{
    _data = new MTQueuePrivate;

    // mutex init
    int error = pthread_mutex_init( &_data->mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " 
                << strerror( error ) << std::endl;
        return;
    }
    // condvar init
    error = pthread_cond_init( &_data->cond, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread condition: " 
                << strerror( error ) << std::endl;
        return;
    }
}

template< typename T >
MTQueue<T>::~MTQueue()
{
    pthread_mutex_destroy( &_data->mutex );
    pthread_cond_destroy( &_data->cond );
    delete _data;
    _data = 0;
}

template< typename T >
T MTQueue<T>::pop()
{
    pthread_mutex_lock( &_data->mutex );
    while( _queue.empty( ))
        pthread_cond_wait( &_data->cond, &_data->mutex );
                
    EQASSERT( !_queue.empty( ));
    T element = _queue.front();
    _queue.pop_front();
    pthread_mutex_unlock( &_data->mutex );
    return element;
}

template< typename T >
T MTQueue<T>::tryPop()
{
    if( _queue.empty( ))
        return NONE;
    
    pthread_mutex_lock( &_data->mutex );
    if( _queue.empty( ))
    {
        pthread_mutex_unlock( &_data->mutex );
        return NONE;
    }
    
    T element = _queue.front();
    _queue.pop_front();
    pthread_mutex_unlock( &_data->mutex );
    return element;
}   

template< typename T >
T MTQueue<T>::back() const
{
    pthread_mutex_lock( &_data->mutex );
    if( _queue.empty( ))
    {
        pthread_mutex_unlock( &_data->mutex );
        return NONE;
    }
    // else
    T element = _queue.back();
    pthread_mutex_unlock( &_data->mutex );
    return element;
}

template< typename T >
void MTQueue<T>::push( const T& element )
{
    pthread_mutex_lock( &_data->mutex );
    _queue.push_back( element );
    pthread_cond_signal( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
}

template< typename T >
void MTQueue<T>::pushFront( const T& element )
{
    pthread_mutex_lock( &_data->mutex );
    _queue.push_front( element );
    pthread_cond_signal( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
}
#endif //HAVE_PTHREAD_H
}

}
#endif //EQBASE_MTQUEUE_H
