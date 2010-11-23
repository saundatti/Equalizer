
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2009, Maxim Makhinya
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "channel.h"

#include "frameData.h"
#include "initData.h"
#include "node.h"
#include "pipe.h"
#include "window.h"
#include "hlp.h"
#include "framesOrderer.h"

namespace eVolve
{
Channel::Channel( eq::Window* parent )
        : eq::Channel( parent )
        , _bgColor( eq::Vector3f::ZERO )
        , _drawRange( eq::Range::ALL )
{
    eq::FrameData* frameData = new eq::FrameData;
    frameData->setBuffers( eq::Frame::BUFFER_COLOR );
    _frame.setData( frameData );
}

static void checkError( const std::string& msg ) 
{
    const GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        EQERROR << msg << " GL Error: " << error << std::endl;
}


bool Channel::configInit( const uint32_t initID )
{
    if( !eq::Channel::configInit( initID ))
        return false;

    setNearFar( 0.001f, 10.0f );

    if( getenv( "EQ_TAINT_CHANNELS" ))
    {
        _bgColor = getUniqueColor();
        _bgColor /= 255.f;
    }

    return true;
}

void Channel::frameStart( const eq::uint128_t& frameID, const uint32_t frameNumber )
{
    _drawRange = eq::Range::ALL;
    eq::Channel::frameStart( frameID, frameNumber );
}

void Channel::frameClear( const eq::uint128_t& frameID )
{
    applyBuffer();
    applyViewport();

    if( getRange() == eq::Range::ALL )
        glClearColor( _bgColor.r(), _bgColor.g(), _bgColor.b(), 1.0f );
    else
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    glClear( GL_COLOR_BUFFER_BIT );
}


static void setLights( eq::Matrix4f& invRotationM )
{
    GLfloat lightAmbient[]  = {0.05f, 0.05f, 0.05f, 1.0f};
    GLfloat lightDiffuse[]  = {0.9f , 0.9f , 0.9f , 1.0f};
    GLfloat lightSpecular[] = {0.8f , 0.8f , 0.8f , 1.0f};
    GLfloat lightPosition[] = {1.0f , 1.0f , 1.0f , 0.0f};

    glLightfv( GL_LIGHT0, GL_AMBIENT,  lightAmbient  );
    glLightfv( GL_LIGHT0, GL_DIFFUSE,  lightDiffuse  );
    glLightfv( GL_LIGHT0, GL_SPECULAR, lightSpecular );

    // rotate light in the opposite direction of the model rotation to keep
    // light position constant and avoid recalculating normals in the fragment
    // shader
    glPushMatrix();
    glMultMatrixf( invRotationM.array );
    glLightfv( GL_LIGHT0, GL_POSITION, lightPosition );
    glPopMatrix();
}


void Channel::frameDraw( const eq::uint128_t& frameID )
{
    // Setup frustum
    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    
    EQ_GL_CALL( glMatrixMode( GL_PROJECTION ));
    EQ_GL_CALL( glLoadIdentity( ));
    EQ_GL_CALL( applyFrustum( ));

    EQ_GL_CALL( glMatrixMode( GL_MODELVIEW ));
    EQ_GL_CALL( glLoadIdentity( ));

    // Setup lights before applying head transform, so the light will be
    // consistent in the cave
    const FrameData&    frameData   = _getFrameData();
    const eq::Matrix4f& rotation    = frameData.getRotation();
    const eq::Vector3f& translation = frameData.getTranslation();

    eq::Matrix4f     invRotationM;
    rotation.inverse( invRotationM );
    setLights( invRotationM );

    EQ_GL_CALL( applyHeadTransform( ));

    glTranslatef(  translation.x(), translation.y(), translation.z() );
    glMultMatrixf( rotation.array );

    Pipe*     pipe     = static_cast<Pipe*>( getPipe( ));
    Renderer* renderer = pipe->getRenderer();
    EQASSERT( renderer );

    eq::Matrix4f  modelviewM;     // modelview matrix
    eq::Matrix3f  modelviewITM;   // modelview inversed transposed matrix
    _calcMVandITMV( modelviewM, modelviewITM );

    const eq::Range& range = getRange();
    renderer->render( range, modelviewM, modelviewITM, invRotationM );

    checkError( "error during rendering " );

    _drawRange = range;

#ifndef NDEBUG
    outlineViewport();
#endif
}

const FrameData& Channel::_getFrameData() const
{
    const Pipe* pipe = static_cast< const Pipe* >( getPipe( ));
    return pipe->getFrameData();
}

void Channel::applyFrustum() const
{
    const FrameData& frameData = _getFrameData();

    if( frameData.useOrtho( ))
        eq::Channel::applyOrtho();
    else
        eq::Channel::applyFrustum();
}


void Channel::_calcMVandITMV(
                            eq::Matrix4f& modelviewM,
                            eq::Matrix3f& modelviewITM ) const
{
    const FrameData& frameData = _getFrameData();
    const Pipe*      pipe      = static_cast< const Pipe* >( getPipe( ));
    const Renderer*  renderer  = pipe->getRenderer();

    if( renderer )
    {
        const VolumeScaling& volScaling = renderer->getVolumeScaling();
        
        eq::Matrix4f scale( eq::Matrix4f::ZERO );
        scale.at(0,0) = volScaling.W;
        scale.at(1,1) = volScaling.H;
        scale.at(2,2) = volScaling.D;
        scale.at(3,3) = 1.f;
        
        modelviewM = scale * frameData.getRotation();
    }
    modelviewM.set_translation( frameData.getTranslation( ));

    modelviewM = getHeadTransform() * modelviewM;

    //calculate inverse transposed matrix
    eq::Matrix4f modelviewIM;
    modelviewM.inverse( modelviewIM );
    eq::Matrix3f( modelviewIM ).transpose_to(  modelviewITM  );
}


static void _expandPVP( eq::PixelViewport& pvp, const eq::Images& images,
                        const eq::Vector2i& offset )
{
    for( eq::Images::const_iterator i = images.begin();
         i != images.end(); ++i )
    {
        const eq::PixelViewport imagePVP = (*i)->getPixelViewport() + offset;
        pvp.merge( imagePVP );
    }
}


void Channel::clearViewport( const eq::PixelViewport &pvp )
{
    // clear given area
    glScissor(  pvp.x, pvp.y, pvp.w, pvp.h );
    glClearColor( _bgColor.r(), _bgColor.g(), _bgColor.b(), 1.0f );
    glClear( GL_COLOR_BUFFER_BIT );

    // restore assembly state
    const eq::PixelViewport& windowPVP = getWindow()->getPixelViewport();
    glScissor( 0, 0, windowPVP.w, windowPVP.h );
}

void Channel::_orderFrames( eq::Frames& frames )
{
    eq::Matrix4f        modelviewM;   // modelview matrix
    eq::Matrix3f        modelviewITM; // modelview inversed transposed matrix
    const FrameData&    frameData = _getFrameData();
    const eq::Matrix4f& rotation  = frameData.getRotation();

    if( !frameData.useOrtho( ))
        _calcMVandITMV( modelviewM, modelviewITM );

    orderFrames( frames, modelviewM, modelviewITM, rotation, frameData.useOrtho( ));
}


void Channel::frameAssemble( const eq::uint128_t& frameID )
{
    const bool composeOnly = (_drawRange == eq::Range::ALL);
    eq::FrameData* data = _frame.getData();

    _startAssemble();

    const eq::Frames& frames = getInputFrames();
    eq::PixelViewport  coveredPVP;
    eq::Frames    dbFrames;
    eq::Zoom           zoom( eq::Zoom::NONE );

    // Make sure all frames are ready and gather some information on them
    for( eq::Frames::const_iterator i = frames.begin();
         i != frames.end(); ++i )
    {
        eq::Frame* frame = *i;
        {
            eq::ChannelStatistics stat( eq::Statistic::CHANNEL_FRAME_WAIT_READY,
                                        this );
            frame->waitReady( );
        }
        const eq::Range& range = frame->getRange();
        if( range == eq::Range::ALL ) // 2D frame, assemble directly
            eq::Compositor::assembleFrame( frame, this );
        else
        {
            dbFrames.push_back( frame );
            zoom = frame->getZoom();
            _expandPVP( coveredPVP, frame->getImages(), frame->getOffset() );
        }
    }
    coveredPVP.intersect( getPixelViewport( ));

    if( dbFrames.empty( ))
    {
        resetAssemblyState();
        return;
    }

    // calculate correct frames sequence
    if( !composeOnly && coveredPVP.hasArea( ))
    {
        _frame.clear();
        data->setRange( _drawRange );
        dbFrames.push_back( &_frame );
    }

    _orderFrames( dbFrames );

    // check if current frame is in proper position, read back if not
    if( !composeOnly )
    {
        if( _bgColor == eq::Vector3f::ZERO && dbFrames.front() == &_frame )
            dbFrames.erase( dbFrames.begin( ));
        else if( coveredPVP.hasArea())
        {
            eq::Window::ObjectManager* glObjects = getObjectManager();

            _frame.setOffset( eq::Vector2i( 0, 0 ));
            _frame.setZoom( zoom );
            data->setPixelViewport( coveredPVP );
            _frame.readback( glObjects, getDrawableConfig( ));
            clearViewport( coveredPVP );

            // offset for assembly
            _frame.setOffset( eq::Vector2i( coveredPVP.x, coveredPVP.y ));
        }
    }

    // blend DB frames in order
    eq::Compositor::assembleFramesSorted( dbFrames, this, 0, true /*blendAlpha*/ );
    resetAssemblyState();

    // Update range
    _drawRange = getRange();
}

void Channel::_startAssemble()
{
    applyBuffer();
    applyViewport();
    setupAssemblyState();
}   

void Channel::frameReadback( const eq::uint128_t& frameID )
{
    // Drop depth buffer flag from all output frames
    const eq::Frames& frames = getOutputFrames();
    const FrameData& frameData = _getFrameData();
    for( eq::Frames::const_iterator i = frames.begin(); 
         i != frames.end(); ++i )
    {
        eq::Frame* frame = *i;
        frame->setQuality( eq::Frame::BUFFER_COLOR, frameData.getQuality());
        frame->disableBuffer( eq::Frame::BUFFER_DEPTH );
    }

    eq::Channel::frameReadback( frameID );
}

void Channel::frameViewFinish( const eq::uint128_t& frameID )
{
    _drawHelp();
    _drawLogo();
}

void Channel::_drawLogo( )
{
    // Draw the overlay logo
    const Window* window = static_cast<Window*>( getWindow( ));
    const eq::util::Texture* texture = window->getLogoTexture();
    if( !texture )
        return;

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    applyScreenFrustum();
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_LIGHTING );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glColor3f( 1.0f, 1.0f, 1.0f );
    glEnable( GL_TEXTURE_RECTANGLE_ARB );
    texture->bind();
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER,
                     GL_LINEAR );
    glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, 
                     GL_LINEAR );

    const float tWidth = float( texture->getWidth( ));
   const float tHeight = float( texture->getHeight( ));

    glBegin( GL_TRIANGLE_STRIP ); {
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( 5.0f, 5.0f, 0.0f );

        glTexCoord2f( tWidth, 0.0f );
        glVertex3f( tWidth + 5.0f, 5.0f, 0.0f );

        glTexCoord2f( 0.0f, tHeight );
        glVertex3f( 5.0f, tHeight + 5.0f, 0.0f );

        glTexCoord2f( tWidth, tHeight );
        glVertex3f( tWidth + 5.0f, tHeight + 5.0f, 0.0f );
    } glEnd();

    glDisable( GL_TEXTURE_RECTANGLE_ARB );
    glDisable( GL_BLEND );
}

void Channel::_drawHelp()
{
    const FrameData& frameData = _getFrameData();
    std::string message = frameData.getMessage();

    if( !frameData.showHelp() && message.empty( ))
        return;

    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );

    glColor3f( 1.f, 1.f, 1.f );

    if( frameData.showHelp( ))
    {
        const eq::Window::Font* font = getWindow()->getSmallFont();
        std::string help = EVolve::getHelp();
        float y = 340.f;

        for( size_t pos = help.find( '\n' ); pos != std::string::npos;
             pos = help.find( '\n' ))
        {
            glRasterPos3f( 10.f, y, 0.99f );

            font->draw( help.substr( 0, pos ));
            help = help.substr( pos + 1 );
            y -= 16.f;
        }
        // last line
        glRasterPos3f( 10.f, y, 0.99f );
        font->draw( help );
    }

    if( !message.empty( ))
    {
        const eq::Window::Font* font = getWindow()->getMediumFont();

        const eq::Viewport& vp = getViewport();
        const eq::PixelViewport& pvp = getPixelViewport();

        const float width = pvp.w / vp.w;
        const float xOffset = vp.x * width;

        const float height = pvp.h / vp.h;
        const float yOffset = vp.y * height;
        const float yMiddle = 0.5f * height;
        float y = yMiddle - yOffset;

        for( size_t pos = message.find( '\n' ); pos != std::string::npos;
             pos = message.find( '\n' ))
        {
            glRasterPos3f( 10.f - xOffset, y, 0.99f );
            
            font->draw( message.substr( 0, pos ));
            message = message.substr( pos + 1 );
            y -= 22.f;
        }
        // last line
        glRasterPos3f( 10.f - xOffset, y, 0.99f );
        font->draw( message );
    }

    EQ_GL_CALL( resetAssemblyState( ));
}


}
