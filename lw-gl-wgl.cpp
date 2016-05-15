
#include <Windows.h>
#include <gl/gl.h>

#include "lw-gl.h"
#include <cstdio>

lwOpenGLCanvas::lwOpenGLCanvas()
{
	this->gl_context = nullptr;
	this->device_context = nullptr;
}

lwOpenGLCanvas::~lwOpenGLCanvas()
{
	this->destroy();
}

bool lwOpenGLCanvas::create( lwBaseControl* p_parent, std::map<std::string,std::string> &p_options )
{
	if ( p_parent == nullptr || p_parent->m_handle == nullptr )
	{
		return false;
	}

	uint32_t style = WS_VISIBLE;
	uint32_t ex_style = 0;
	if ( this->border     ) style = WS_BORDER;
	if ( this->clientEdge ) style = WS_EX_CLIENTEDGE;

	this->m_handle = CreateWindowEx(ex_style,
									"Static",
									"",
									WS_CHILD | WS_CLIPSIBLINGS | style,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									nullptr );

	if ( this->m_handle == nullptr )
	{
		return false;
	}

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );

	this->device_context = (HDC)GetDC( (HWND)this->m_handle );
	if ( this->device_context == nullptr )
	{
	    printf("GLCanvas: Failed to get GDI DC\n");
		return false;
	}

	PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
        32,                        //Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                        //Number of bits for the depthbuffer
        8,                        //Number of bits for the stencilbuffer
        0,                        //Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    if (!SetPixelFormat((HDC)this->device_context, ChoosePixelFormat( (HDC)this->device_context, &pfd ), &pfd))
	{
	    printf("GLCanvas: Failed to set DC Pixel Format\n");
		return false;
	}

	this->gl_context = (lwOpenGLContextType)wglCreateContext( (HDC)this->device_context );
	if ( this->gl_context == nullptr )
	{
	    printf("GLCanvas: Failed to create WGL context\n");
		return false;
	}

	wglMakeCurrent( (HDC)this->device_context, (HGLRC)this->gl_context );

	// -- Size the widget correctly




	wglMakeCurrent( nullptr, nullptr );

	return true;
}

void lwOpenGLCanvas::destroy(void)
{
	if ( this->device_context != nullptr && this->gl_context != nullptr )
	{
		wglMakeCurrent( nullptr, nullptr );
		wglDeleteContext( (HGLRC)this->gl_context );
		ReleaseDC( (HWND)this->m_handle, (HDC)this->device_context );
	}

	lwBaseControl::destroy();
}

void lwOpenGLCanvas::resize( uint32_t p_width, uint32_t p_height )
{
    wglMakeCurrent( (HDC)this->device_context, (HGLRC)this->gl_context );

	lwBaseControl::resize( p_width, p_height );
	glViewport( 0,0, p_width, p_height );

	wglMakeCurrent( nullptr, nullptr );
}

void lwOpenGLCanvas::maximise( )
{
    wglMakeCurrent( (HDC)this->device_context, (HGLRC)this->gl_context );

    int32_t p_width, p_height;
	lwBaseControl::maximise( );
	this->getClientArea( nullptr, nullptr, &p_width, &p_height );
	glViewport( 0,0, p_width, p_height );

	wglMakeCurrent( nullptr, nullptr );
}

bool lwOpenGLCanvas::setCurrent()
{
	if ( this->device_context == nullptr || this->gl_context == nullptr )
	{
		return false;
	}

    m_mutex.lock();
	wglMakeCurrent( (HDC)this->device_context, (HGLRC)this->gl_context );

	return true;
}

bool lwOpenGLCanvas::unsetCurrent()
{
	if ( this->device_context == nullptr || this->gl_context == nullptr )
	{
		return false;
	}

    if (wglGetCurrentContext() == (HGLRC)this->gl_context)
        wglMakeCurrent( nullptr, nullptr );
    m_mutex.unlock();

	return true;
}

bool lwOpenGLCanvas::swapBuffers()
{
	if ( this->device_context == nullptr || this->gl_context == nullptr )
	{
		return false;
	}

	SwapBuffers( (HDC)this->device_context );

	return true;
}