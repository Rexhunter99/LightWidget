#pragma once
#ifndef __LIGHTWIDGET_OPENGLCANVAS_H__
#define __LIGHTWIDGET_OPENGLCANVAS_H__

#include <lw-main.h>
#include <map>
#include <string>
#include <mutex>



#define LW_OPENGLCANVAS_VERSION "0.5"


typedef struct _OPENGLCONTEXT*	lwOpenGLContextType;
typedef void*					lwDeviceContextType;

/** @class lwOpenGLCanvas
 * @version 1.0
 * @author Rexhunter99
 * @brief OpenGL compatible 'canvas' widget
 * This is a basic implementation of an OpenGL canvas widget that allows
 * developers to add a region in their windows for rendering OpenGL contexts
 * with.  It is implemented via WGL for Windows and GLX for X11 display
 * managers that are used in systems such as Ubuntu and Debian.
 */
class lwOpenGLCanvas : public lwBaseControl
{
private:

	lwOpenGLContextType gl_context;
	lwDeviceContextType device_context;
	std::mutex          m_mutex;

public:

    enum lwOpenGLCanvasAttributeEnum {
        LWGL_PROFILE            = 0,
        LWGL_COLOR_BITS,
        LWGL_DEPTH_BITS,
        LWGL_STENCIL_BITS,
        LWGL_MULTISAMPLEAA_SAMPLES,
        LWGL_API_MAJOR_VERSION,
        LWGL_API_MINOR_VERSION,

        // END OF ATTRIBUTES
        // BEGIN ATTRIBUTE VALUES

        LWGL_PROFILE_LEGACY     = 0,    // Do not load extensions for extensible pixel formats or context creation (GL 2.1 and below)
        LWGL_PROFILE_CORE       = 1,    // A core profile is desired, use extensions to retrieve a suitable context and pixel format (GL 3.0 and above)
        LWGL_PROFILE_COMPAT     = 2,    // A compatibility profile is desired that allows deprecated functionality to be used in newer profiles.

    };

    lwOpenGLCanvas();
    ~lwOpenGLCanvas();

	bool create( lwBaseControl* p_parent, std::map<lwOpenGLCanvasAttributeEnum,int> &p_options );

	/** @fn destroy()
	 * @overload
	 */
    void destroy( void );

    /** @fn resize()
	 * @overload
	 * @param p_width The width in pixels of the widget
	 * @param p_height The height in pixels of the widget
	 */
    void resize( uint32_t p_width, uint32_t p_height );

    /** @fn maxmise()
	 * @overload
	 */
    void maximise( );

	/** @fn setCurrent()
	 * @brief Set this canvas' context as the current render target for OpenGL contexts
	 * Set this canvas's context as the current render target for OpenGL contexts
     */
	bool setCurrent();

	/** @fn unsetCurrent()
	 * @brief Unset this canvas' context as the current render target for OpenGL contexts
	 * Set this canvas's context as the current render target for OpenGL contexts
     */
	bool unsetCurrent();

	/** @fn swapBuffers()
	 * @brief Swap this canvas' back and front buffers
	 * Swap this canvas' back and front buffers, the drivers determine if the buffer is flipped or copied
     */
    bool swapBuffers();
};

#endif // __LIGHTWIDGET_OPENGLCANVAS_H__
