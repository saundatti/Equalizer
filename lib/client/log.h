
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

#ifndef EQ_LOG_H
#define EQ_LOG_H

#include <eq/net/log.h>

namespace eq
{
    enum LogTopics
    {
        LOG_ASSEMBLY = net::LOG_CUSTOM << 0,   // 256
        LOG_TASKS    = net::LOG_CUSTOM << 1,   // 512
        LOG_EVENTS   = net::LOG_CUSTOM << 2,   // 1024
        LOG_STATS    = net::LOG_CUSTOM << 3,   // 2048
        LOG_INIT     = net::LOG_CUSTOM << 4,   // 4096
        LOG_SERVER   = net::LOG_CUSTOM << 5,   // 8192
        LOG_CUSTOM   = 0x10000                 // 65536
    };
}
#endif // EQ_LOG_H
