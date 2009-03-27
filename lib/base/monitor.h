
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQBASE_MONITOR_H
#define EQBASE_MONITOR_H

#include <eq/base/nonCopyable.h> // base class

#include <errno.h>
#include <string.h>
#include <iostream>


namespace eq
{
namespace base
{
    class MonitorPrivate;

    /**
     * A monitor primitive.
     *
     * A monitor has a value, which can be monitored to reach a certain
     * state. The caller is blocked until the condition is fulfilled.
     *
     * Template instantiations are at the end of monitor.cpp.
     */
    template< typename T > class Monitor : public NonCopyable
    {
    public:
        /** 
         * Constructs a new monitor for the given thread type.
         */
        Monitor() : _value( static_cast<T>( 0 ))    { _construct(); }
        Monitor( const T& value ) : _value( value ) { _construct(); }

        void _construct();
        
        /** Destructs the monitor. */
        ~Monitor();

        /** @name Changing the monitored value. */
        //*{
        Monitor& operator++ ();    // prefix only
        Monitor& operator-- ();    // prefix only
        Monitor& operator = ( const T& value )
            {
                set( value );
                return *this;
            }

        void set( const T& value );
        //*}

        /** @name Monitor the value. */
        //*{
        const T& waitEQ( const T& value ) const;
        const T& waitNE( const T& value ) const;
        const T& waitGE( const T& value ) const;
        const T& waitLE( const T& value ) const;
        //*}

        /** @name Comparison Operators. */
        //*{
        bool operator == ( const T& value ) const { return _value == value; }
        bool operator != ( const T& value ) const { return _value != value; }
        bool operator < ( const T& value ) const { return _value < value; }
        bool operator > ( const T& value ) const { return _value > value; }
        bool operator <= ( const T& value ) const { return _value <= value; }
        bool operator >= ( const T& value ) const { return _value >= value; }

        bool operator == ( const Monitor<T>& rhs ) const
            { return _value == rhs._value; }
        bool operator != ( const Monitor<T>& rhs ) const
            { return _value != rhs._value; }
        bool operator < ( const Monitor<T>& rhs ) const
            { return _value < rhs._value; }
        bool operator > ( const Monitor<T>& rhs ) const
            { return _value > rhs._value; }
        bool operator <= ( const Monitor<T>& rhs ) const
            { return _value <= rhs._value; }
        bool operator >= ( const Monitor<T>& rhs ) const
            { return _value >= rhs._value; }
        //*}

        const T& get() const { return _value; }
        T operator + ( const T& value ) const { return _value + value; }

    private:
        T _value;
        MonitorPrivate* _data;
    };

typedef Monitor< bool >     Monitorb;
typedef Monitor< uint32_t > Monitoru;

template< typename T >
std::ostream& operator << ( std::ostream& os, const Monitor<T>& monitor );
}
}

//----------------------------------------------------------------------
// implementation
//----------------------------------------------------------------------

// Crude test if pthread.h was included
#ifdef PTHREAD_MUTEX_INITIALIZER
#  ifndef HAVE_PTHREAD_H
#    define HAVE_PTHREAD_H
#  endif
#endif

// Monitor for uint32_t and bool are explicitly instantiated in monitor.cpp
// Monitors for other types can be created by including pthread.h before this
// file.  
// The application has to include pthread.h since on Windows the use of
// pthreads-Win32 library includes might create hard to resolve type conflicts
// with other header files.

#ifdef HAVE_PTHREAD_H

#include <eq/base/debug.h>
#include <eq/base/log.h>

namespace eq
{
namespace base
{
class MonitorPrivate
{
public:
    pthread_cond_t  cond;
    pthread_mutex_t mutex;
};

template< typename T > 
inline void Monitor<T>::_construct()
{
    _data = new MonitorPrivate;

    int error = pthread_cond_init( &_data->cond, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread condition: " 
                << strerror( error ) << std::endl;
        return;
    } 
    
    error = pthread_mutex_init( &_data->mutex, 0 );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: "
                << strerror( error ) << std::endl;
        return;
    }
}
        
template< typename T > 
inline Monitor<T>::~Monitor()
{
    pthread_cond_destroy( &_data->cond );
    pthread_mutex_destroy( &_data->mutex );
    delete _data;
    _data = 0;
}

template< typename T > 
inline Monitor<T>& Monitor<T>::operator++ ()
{
    pthread_mutex_lock( &_data->mutex );
    ++_value;
    pthread_cond_broadcast( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
    return *this;
}

template< typename T > 
inline Monitor<T>& Monitor<T>::operator-- ()
{
    pthread_mutex_lock( &_data->mutex );
    --_value;
    pthread_cond_broadcast( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
    return *this;
}

template<> 
inline Monitor< bool >& Monitor< bool >::operator-- ()
{
    EQUNIMPLEMENTED;
    return *this;
}

template< typename T > 
inline void Monitor<T>::set( const T& value )
{
    pthread_mutex_lock( &_data->mutex );
    _value = value;
    pthread_cond_broadcast( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
}

template< typename T > 
inline const T& Monitor<T>::waitEQ( const T& value ) const
{
    pthread_mutex_lock( &_data->mutex );
    while( _value != value )
        pthread_cond_wait( &_data->cond, &_data->mutex);
    pthread_mutex_unlock( &_data->mutex );
    return value;
}

template< typename T > 
inline const T& Monitor<T>::waitNE( const T& value ) const
{
    pthread_mutex_lock( &_data->mutex );
    while( _value == value )
        pthread_cond_wait( &_data->cond, &_data->mutex);
    const T& newValue = _value;
    pthread_mutex_unlock( &_data->mutex );
    return newValue;
}

template< typename T > 
inline const T& Monitor<T>::waitGE( const T& value ) const
{
    pthread_mutex_lock( &_data->mutex );
    while( _value < value )
        pthread_cond_wait( &_data->cond, &_data->mutex);
    const T& newValue = _value;
    pthread_mutex_unlock( &_data->mutex );
    return newValue;
}

template< typename T > 
inline const T& Monitor<T>::waitLE( const T& value ) const
{
    pthread_mutex_lock( &_data->mutex );
    while( _value > value )
        pthread_cond_wait( &_data->cond, &_data->mutex);
    const T& newValue = _value;
    pthread_mutex_unlock( &_data->mutex );
    return newValue;
}

template< typename T >
std::ostream& operator << ( std::ostream& os, const Monitor<T>& monitor )
{
    os << "Monitor< " << monitor.get() << " >";
    return os;
}
}
}
#endif // HAVE_PTHREAD_H
#endif //EQBASE_MONITOR_H
