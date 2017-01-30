
#include <Windows.h>
#include <gl/gl.h>

#include "lw-gl.h"
#include <cstdio>
#include <iostream>

using namespace std;

lwOpenGLCanvas::lwOpenGLCanvas()
{
	this->gl_context = nullptr;
	this->device_context = nullptr;
	//this->m_type = lwWidgetTypeEnum::LW_VIDEO_CANVAS + 1;
}

lwOpenGLCanvas::~lwOpenGLCanvas()
{
    if ( !this->m_mutex.try_lock() )
    {
        fprintf(stderr, "~lwOpenGLCanvas() :: Warning! Context was not unlocked!");
    }

    this->m_mutex.unlock();
	this->destroy();
}

bool lwOpenGLCanvas::create( lwBaseControl* p_parent, std::map<int,int> &p_options )
{

	if ( p_parent == nullptr || p_parent->m_handle == nullptr )
	{
		return false;
	}

	int r_redbits = 8,
		r_greenbits = 8,
		r_bluebits = 8,
		r_alphabits = 8,
		r_depthbits = 16,
		r_stencilbits = 0,
		r_profile = 0,
		r_gl_major_version = 1,
		r_gl_minor_version = 0;

	try { r_profile	= p_options.at(lwOpenGLCanvasAttributeEnum::LWGL_PROFILE); }
	catch ( const out_of_range &oor ) {}
	try { r_gl_major_version = p_options.at(lwOpenGLCanvasAttributeEnum::LWGL_API_MAJOR_VERSION); }
	catch ( const out_of_range &oor ) {}
	try { r_gl_minor_version = p_options.at(lwOpenGLCanvasAttributeEnum::LWGL_API_MINOR_VERSION); }
	catch ( const out_of_range &oor ) {}
	try { r_redbits	= p_options.at(lwOpenGLCanvasAttributeEnum::LWGL_RED_BITS); }
	catch ( const out_of_range &oor ) {}
	try { r_greenbits	= p_options.at(lwOpenGLCanvasAttributeEnum::LWGL_GREEN_BITS); }
	catch ( const out_of_range &oor ) {}
	try { r_bluebits	= p_options.at(lwOpenGLCanvasAttributeEnum::LWGL_BLUE_BITS); }
	catch ( const out_of_range &oor ) {}
	try { r_alphabits	= p_options.at(lwOpenGLCanvasAttributeEnum::LWGL_ALPHA_BITS); }
	catch ( const out_of_range &oor ) {}
	try { r_depthbits	= p_options.at(lwOpenGLCanvasAttributeEnum::LWGL_DEPTH_BITS); }
	catch ( const out_of_range &oor ) {}
	try { r_stencilbits	= p_options.at(lwOpenGLCanvasAttributeEnum::LWGL_STENCIL_BITS); }
	catch ( const out_of_range &oor ) {}

	// -- Legacy compatibility
    int r_colorbits = r_redbits + r_greenbits + r_bluebits + r_alphabits;

	uint32_t style = WS_VISIBLE;
	uint32_t ex_style = 0;

	if ( this->border     ) style       |= WS_BORDER;
	if ( this->clientEdge ) ex_style    |= WS_EX_CLIENTEDGE;

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
	    fprintf(stderr, "GLCanvas: Failed to get GDI DC\n");
		return false;
	}

	PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
        r_colorbits,                        //Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        r_depthbits,                        //Number of bits for the depthbuffer
        r_stencilbits,                        //Number of bits for the stencilbuffer
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

	// TODO: Create a GL3+ Context here if requested

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
