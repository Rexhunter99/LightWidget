
#include <Windows.h>

#include "lw-gl.h"
#include <iostream>
#include <sstream>

#include <GL/gl.h>
#include <GL/wglext.h>

using namespace std;


PFNWGLGETEXTENSIONSSTRINGARBPROC	wglGetExtensionsStringARB = nullptr;
PFNWGLCREATECONTEXTATTRIBSARBPROC	wglCreateContextAttribsARB = nullptr;
PFNWGLCHOOSEPIXELFORMATARBPROC		wglChoosePixelFormatARB = nullptr;


lwOpenGLCanvas::lwOpenGLCanvas()
{
	this->m_context = nullptr;
	this->m_device = nullptr;
	this->m_type = lwWidgetTypeEnum::LW_VIDEO_CANVAS;
}

lwOpenGLCanvas::~lwOpenGLCanvas()
{
    if ( !this->m_mutex.try_lock() )
    {
        cerr << "~lwOpenGLCanvas() :: Warning! Context was not unlocked!" << endl;
    }

    this->m_mutex.unlock();
	this->destroy();
}


void lwOpenGLCanvas::parseExtensions()
{
	const char *cstr_extensions = reinterpret_cast<const char*>(glGetString( GL_EXTENSIONS ));
	stringstream ss;
	ss << cstr_extensions;

	while (!ss.eof())
	{
		string extension = "";
		getline(ss, extension, ' ');
		this->m_extensions.insert( pair<string, bool>(extension, true) );
	}

	if (wglGetExtensionsStringARB)
	{
		ss.clear();
		cstr_extensions = reinterpret_cast<const char*>(wglGetExtensionsStringARB( (HDC)this->m_device ));
		ss << cstr_extensions;

		while (!ss.eof())
		{
			string extension = "";
			getline(ss, extension, ' ');
			this->m_extensions.insert( pair<string, bool>(extension, true) );
		}
	}
}

bool lwOpenGLCanvas::extensionIsSupported( const std::string &extension )
{
	return (this->m_extensions.find(extension) != this->m_extensions.end());
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
		r_aa_samples = 0,
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
	try { r_aa_samples	= p_options.at(lwOpenGLCanvasAttributeEnum::LWGL_MULTISAMPLEAA_SAMPLES); }
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

	this->m_device = (HDC)GetDC( (HWND)this->m_handle );
	HDC dc = (HDC)this->m_device;
	if ( this->m_device == nullptr )
	{
	    cerr << "GLCanvas: Failed to get GDI DC" << endl;
		return false;
	}

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
		PFD_TYPE_RGBA,				//The kind of framebuffer. RGBA or palette.
		(uint8_t)r_colorbits & 0xff,			//Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		(uint8_t)r_depthbits & 0xff,			//Number of bits for the depthbuffer
		(uint8_t)r_stencilbits & 0xff,		//Number of bits for the stencilbuffer
        0,							//Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
	int pixel_format = 0;
	unsigned int num_pixel_formats = 0;

	pixel_format = ChoosePixelFormat( (HDC)this->m_device, &pfd );

	if ( !pixel_format )
	{
		cerr << "GLCanvas: Failed to choose an appropriate pixel format!" << endl;
		return false;
	}

    if (!SetPixelFormat((HDC)this->m_device, pixel_format, &pfd))
	{
	    cerr << "GLCanvas: Failed to set DC pixel format" << endl;
		return false;
	}

	this->m_context = (lwOpenGLContextType)wglCreateContext( (HDC)this->m_device );
	if ( this->m_context == nullptr )
	{
	    cerr << "GLCanvas: Failed to create WGL context" << endl;
		return false;
	}

	wglMakeCurrent( dc, (HGLRC)this->m_context );

	// -- Check for ARB extension support and parse all extensions
	wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
	this->parseExtensions();

	/**************************************
	** Create a modern OpenGL context   **/

	const int pixelformat_attribs[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
		WGL_COLOR_BITS_ARB, r_colorbits,
		WGL_RED_BITS_ARB, r_redbits,
		WGL_GREEN_BITS_ARB, r_greenbits,
		WGL_BLUE_BITS_ARB, r_bluebits,
		WGL_ALPHA_BITS_ARB, r_alphabits,
		WGL_DEPTH_BITS_ARB, r_depthbits,
		WGL_STENCIL_BITS_ARB, r_stencilbits,
		0,        //End
	};

	float pixelformat_attribs_float[] = {0.0f,0.0f};

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, r_gl_major_version,
		WGL_CONTEXT_MINOR_VERSION_ARB, r_gl_minor_version,
		WGL_CONTEXT_FLAGS_ARB, 0
#if defined(_DEBUG)
			| WGL_CONTEXT_DEBUG_BIT_ARB
#endif
		,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	if ( this->extensionIsSupported("WGL_ARB_pixel_format") )
	{
		wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

		bool valid = wglChoosePixelFormatARB(dc, pixelformat_attribs, pixelformat_attribs_float, 1, &pixel_format, &num_pixel_formats );

		if ( !valid )
		{
			cerr << "GLCanvas: Failed to choose extensible pixel format" << endl;
		}

		if (!SetPixelFormat( dc, pixel_format, &pfd ))
		{
			cerr << "GLCanvas: Failed to set extensible DC pixel format" << endl;
		}
	}

	if ( this->extensionIsSupported("WGL_ARB_create_context") )
	{
		wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
		if ( !wglCreateContextAttribsARB )
		{
			cerr << "Error! Unable to retrieve address of \"wglCreateContextAttribsARB\"" << endl;
		}
		else
		{
			HGLRC new_context = wglCreateContextAttribsARB(dc, 0, attribs);
			wglMakeCurrent( nullptr, nullptr );
			wglDeleteContext( (HGLRC)this->m_context );
			wglMakeCurrent( dc, new_context );
			this->m_context = (lwOpenGLContextType)new_context;
		}
	}
	else
	{	//It's not possible to make a GL 3.x context. Use the old style context (GL 2.1 and before)
		//m_hrc = tempContext;
		cerr << "Warning! WGL_ARB_create_context is not supported." << endl;
	}

	wglMakeCurrent( nullptr, nullptr );

	return true;
}

void lwOpenGLCanvas::destroy(void)
{
	if ( this->m_device != nullptr && this->m_context != nullptr )
	{
		wglMakeCurrent( nullptr, nullptr );
		wglDeleteContext( (HGLRC)this->m_context );
		ReleaseDC( (HWND)this->m_handle, (HDC)this->m_device );
	}

	lwBaseControl::destroy();
}

void lwOpenGLCanvas::resize( uint32_t p_width, uint32_t p_height )
{
    wglMakeCurrent( (HDC)this->m_device, (HGLRC)this->m_context );

	lwBaseControl::resize( p_width, p_height );
	glViewport( 0,0, p_width, p_height );

	if ( this->onResize )
	{
		this->onResize( p_width, p_height );
	}

	wglMakeCurrent( nullptr, nullptr );
}

void lwOpenGLCanvas::maximise( )
{
    wglMakeCurrent( (HDC)this->m_device, (HGLRC)this->m_context );

    int32_t p_width, p_height;
	lwBaseControl::maximise( );
	this->getClientArea( nullptr, nullptr, &p_width, &p_height );
	glViewport( 0,0, p_width, p_height );

	wglMakeCurrent( nullptr, nullptr );
}

bool lwOpenGLCanvas::setCurrent()
{
	if ( this->m_device == nullptr || this->m_context == nullptr )
	{
		return false;
	}

    m_mutex.lock();
	wglMakeCurrent( (HDC)this->m_device, (HGLRC)this->m_context );

	return true;
}

bool lwOpenGLCanvas::unsetCurrent()
{
	if ( this->m_device == nullptr || this->m_context == nullptr )
	{
		return false;
	}

    if (wglGetCurrentContext() == (HGLRC)this->m_context)
        wglMakeCurrent( nullptr, nullptr );
    m_mutex.unlock();

	return true;
}

void lwOpenGLCanvas::lock()
{
	m_mutex.lock();
}

void lwOpenGLCanvas::unlock()
{
	m_mutex.unlock();
}

bool lwOpenGLCanvas::swapBuffers()
{
	if ( this->m_device == nullptr || this->m_context == nullptr )
	{
		return false;
	}

	SwapBuffers( (HDC)this->m_device );

	return true;
}
