
#include "lw-gl.h"

#include <GL/gl.h>
#include <GL/glx.h>
#include <iostream>
#include <sstream>

lwOpenGLCanvas::lwOpenGLCanvas() : lwBaseControl()
{
	this->m_type = lwWidgetTypeEnum::LW_VIDEO_CANVAS;
	this->m_device = nullptr;
	this->m_context = nullptr;
}

lwOpenGLCanvas::~lwOpenGLCanvas()
{
	if ( !this->m_mutex.try_lock() )
    {
        std::cerr << "~lwOpenGLCanvas() :: Warning! Context was not unlocked!" << std::endl;
    }

    this->m_mutex.unlock();
	this->destroy();
}

bool lwOpenGLCanvas::create( lwBaseControl* p_parent, std::map<int,int> &p_options )
{
	const int visual_attribs[] =
	{
		GLX_X_RENDERABLE    , True,
		GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
		GLX_RENDER_TYPE     , GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
		GLX_RED_SIZE        , 8,
		GLX_GREEN_SIZE      , 8,
		GLX_BLUE_SIZE       , 8,
		GLX_ALPHA_SIZE      , 8,
		GLX_DEPTH_SIZE      , 24,
		GLX_STENCIL_SIZE    , 8,
		GLX_DOUBLEBUFFER    , True,
		//GLX_SAMPLE_BUFFERS  , 1,
		//GLX_SAMPLES         , 4,
		None
	};

	int glx_major, glx_minor;

	// FBConfigs were added in GLX version 1.3.
	if ( !glXQueryVersion( display, &glx_major, &glx_minor ) ||
       ( ( glx_major == 1 ) && ( glx_minor < 3 ) ) || ( glx_major < 1 ) )
	{
		std::cerr << "Invalid GLX version" << std::endl;
		return false;
	}

	// Getting matching framebuffer configs
	int fbcount;
	GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
	if (!fbc)
	{
		std::cerr << "Failed to retrieve a framebuffer config" << std::endl;
		return false;
	}

#if defined(_DEBUG)
	std::cout << "Found " << fbcount << " matching FB configs." << std::endl;
#endif

	// Pick the FB config/visual with the most samples per pixel
	int best_fbc = -1,
		worst_fbc = -1,
		best_num_samp = -1,
		worst_num_samp = 999;

	int i;
	for (i=0; i<fbcount; ++i)
	{
		XVisualInfo *vi = glXGetVisualFromFBConfig( display, fbc[i] );
		if ( vi )
		{
			int samp_buf, samples;
			glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
			glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES       , &samples  );

			if ( best_fbc < 0 || samp_buf && samples > best_num_samp ) {
				best_fbc = i, best_num_samp = samples;
			}
			if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp ) {
				worst_fbc = i, worst_num_samp = samples;
			}
		}
		XFree( vi );
	}

	GLXFBConfig bestFbc = fbc[ best_fbc ];

	// Be sure to free the FBConfig list allocated by glXChooseFBConfig()
	XFree( fbc );

	// Get a visual
	XVisualInfo *vi = glXGetVisualFromFBConfig( display, bestFbc );

	// Creating colormap\n" );
	XSetWindowAttributes swa;
	Colormap cmap;
	swa.colormap = cmap = XCreateColormap( display,
                                         RootWindow( display, vi->screen ),
                                         vi->visual, AllocNone );
	swa.background_pixmap = None ;
	swa.border_pixel      = 0;
	swa.event_mask        = StructureNotifyMask;

	// Creating window\n" );
	this->m_handle = XCreateWindow( display, RootWindow( display, vi->screen ),
                              0, 0, 1, 1, 0, vi->depth, InputOutput,
                              vi->visual,
                              CWBorderPixel|CWColormap|CWEventMask, &swa );
	if ( !win )
	{
		printf( "Failed to create window.\n" );
		return false;
	}

	// Done with the visual info data
	XFree( vi );

	XMapWindow( display, win );

  // Get the default screen's GLX extension list
  const char *glxExts = glXQueryExtensionsString( display,
                                                  DefaultScreen( display ) );

  // NOTE: It is not necessary to create or make current to a context before
  // calling glXGetProcAddressARB
  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
  glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
           glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );

  GLXContext ctx = 0;

  // Install an X error handler so the application won't exit if GL 3.0
  // context allocation fails.
  //
  // Note this error handler is global.  All display connections in all threads
  // of a process use the same error handler, so be sure to guard against other
  // threads issuing X commands while this code is running.
  ctxErrorOccurred = false;
  int (*oldHandler)(Display*, XErrorEvent*) =
      XSetErrorHandler(&ctxErrorHandler);

  // Check for the GLX_ARB_create_context extension string and the function.
  // If either is not present, use GLX 1.3 context creation method.
  if ( !isExtensionSupported( glxExts, "GLX_ARB_create_context" ) ||
       !glXCreateContextAttribsARB )
  {
    printf( "glXCreateContextAttribsARB() not found"
            " ... using old-style GLX context\n" );
    ctx = glXCreateNewContext( display, bestFbc, GLX_RGBA_TYPE, 0, True );
  }

  // If it does, try to get a GL 3.0 context!
  else
  {
    int context_attribs[] =
      {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
        //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        None
      };

    printf( "Creating context\n" );
    ctx = glXCreateContextAttribsARB( display, bestFbc, 0,
                                      True, context_attribs );

    // Sync to ensure any errors generated are processed.
    XSync( display, False );
    if ( !ctxErrorOccurred && ctx )
      printf( "Created GL 3.0 context\n" );
    else
    {
      // Couldn't create GL 3.0 context.  Fall back to old-style 2.x context.
      // When a context version below 3.0 is requested, implementations will
      // return the newest context version compatible with OpenGL versions less
      // than version 3.0.
      // GLX_CONTEXT_MAJOR_VERSION_ARB = 1
      context_attribs[1] = 1;
      // GLX_CONTEXT_MINOR_VERSION_ARB = 0
      context_attribs[3] = 0;

      ctxErrorOccurred = false;

      printf( "Failed to create GL 3.0 context"
              " ... using old-style GLX context\n" );
      ctx = glXCreateContextAttribsARB( display, bestFbc, 0,
                                        True, context_attribs );
    }
  }

  // Sync to ensure any errors generated are processed.
  XSync( display, False );

  // Restore the original error handler
  XSetErrorHandler( oldHandler );

  if ( ctxErrorOccurred || !ctx )
  {
    printf( "Failed to create an OpenGL context\n" );
    exit(1);
  }

  // Verifying that context is a direct context
  if ( ! glXIsDirect ( display, ctx ) )
  {
    printf( "Indirect GLX rendering context obtained\n" );
  }
  else
  {
    printf( "Direct GLX rendering context obtained\n" );
  }

  printf( "Making context current\n" );
  glXMakeCurrent( display, win, ctx );

  glClearColor( 0, 0.5, 1, 1 );
  glClear( GL_COLOR_BUFFER_BIT );
  glXSwapBuffers ( display, win );

  sleep( 1 );

  glClearColor ( 1, 0.5, 0, 1 );
  glClear ( GL_COLOR_BUFFER_BIT );
  glXSwapBuffers ( display, win );

  sleep( 1 );

  glXMakeCurrent( display, 0, 0 );
  glXDestroyContext( display, ctx );

  XDestroyWindow( display, win );
  XFreeColormap( display, cmap );
    return true;
}
