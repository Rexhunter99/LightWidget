
#include "lw-main.h"
#include "lw-resources.h"
#include "lw-gl.h"
#include <cstdio>
#include <cstdlib>
#include <memory.h>
#include <chrono>
#include <thread>

#include <GL/gl.h>

using namespace std;


/* The definition of these instances can go anywhere all except the
 * lwApplication instance, you need to have this created outside the
 * main() function, so leave it in main.cpp */
lwApplication	*g_application = nullptr;
lwFrame			g_window;
lwOpenGLCanvas  g_glcanvas;
lwDialog		g_about_dialog;

std::thread     g_gl_thread;
bool            g_gl_thread_quit = false;
chrono::steady_clock::time_point time_application_start;


/* Follows are implementations of event functions used in the test
 * controls created in main() */

void eventCloseWindow( void )
{
	g_window.quit();
	printf( "Event: Window Close\n" );
}

void eventButtonClick( void )
{
	g_window.close();
	printf( "Event: Button Click\n" );
}

void eventAboutDialogClose( void )
{
	g_about_dialog.destroy();
	g_window.enable();
	g_window.focus();
}

void errorDialog( void )
{
	g_window.disable();

	// -- About Dialog
	g_about_dialog.onClose = eventAboutDialogClose;
	g_about_dialog.create( &g_window, "About" );
	g_about_dialog.resize( 280, 120 );
	g_about_dialog.move( (g_application->getScreenWidth()/2) - 140, (g_application->getScreenHeight()/2) - 60 );
	g_about_dialog.show();

	/*dlg_txt.create( &g_about_dialog, "Test Application\r\nVersion:\t0.0.0\r\nBy James Ogden" );
	dlg_txt.move( 5, 5 );
	dlg_txt.resize( 270-10, 100-10 );
	dlg_txt.show();*/
}

void eventMainWindowResize( int32_t w, int32_t h )
{
	int32_t window_x, window_y, window_w, window_h;

	g_window.getClientArea( &window_x, &window_y, &window_w, &window_h );

	g_glcanvas.setCurrent();

	g_glcanvas.move( window_x, window_y );
	g_glcanvas.resize( window_w, window_h );

	g_glcanvas.unsetCurrent();
}

void threadOpenGLRender(void)
{
    /// -- NOTE: Can do OpenGL/Direct3D rendering if using a GL/D3D control, or just general runtime processing
OPENGLTHREAD_BEGIN:
    g_glcanvas.setCurrent();

    glClearColor( 0.0f, 0.0f, 0.5f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );
	glLoadIdentity();

	chrono::steady_clock::time_point time_now = chrono::steady_clock::now();
	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(time_now - time_application_start);
	glRotatef( 360.0f * time_span.count(), 0.0f, 0.0f, 1.0f );

	glBegin( GL_TRIANGLES );
		glColor3ub( 255, 0, 0 );	glVertex2f( 0.0f, 1.0f );
		glColor3ub( 0, 255, 0 );	glVertex2f( 1.0f,-1.0f );
		glColor3ub( 0, 0, 255 );	glVertex2f(-1.0f,-1.0f );
	glEnd();

	g_glcanvas.swapBuffers();

	g_glcanvas.unsetCurrent();
	std::this_thread::sleep_for( chrono::milliseconds( 1 ) );

	if ( g_gl_thread_quit == false )
        goto OPENGLTHREAD_BEGIN;
    else
        return;
}


int main( int argc, char **argv )
{
	g_application = new lwApplication( argc, argv );
	g_application->setAuthor( "Rexhunter99" );
	g_application->setTitle( "Light Widget: OpenGL Canvas Demo" );
	g_application->setVersion( "0.0.1" );

	// -- Used in the OpenGL rendering
	time_application_start = chrono::steady_clock::now();

	// -- Just some output stuff
	printf( "ScreenCount->%u\nScreenResolution->%u x %u\nApplicationName->\"%s\"\n",
			g_application->getScreenCount(),
			g_application->getScreenWidth(),
			g_application->getScreenHeight(),
			g_application->getTitle().c_str() );

	// -- Create the main window
	g_window.onResize = eventMainWindowResize;
	g_window.onClose = eventCloseWindow;
	g_window.border = 1;
	g_window.sizeable = 1;
	g_window.titleBar = 1;
	g_window.closeButton = 1;
	g_window.minimizeButton = 1;
	g_window.maximizeButton = 1;
	g_window.acceptFiles = 0;
	if ( !g_window.create( nullptr, "LightWidget: OpenGL Canvas Demo" ) )
	{
		return 1000;
	}
	g_window.resize( 960, 640 );
	g_window.center();

	// -- Create the OpenGLCanvas
	std::map<std::string,std::string> gl_options;
	// TODO: Fill the options up for custom context creation.
	g_glcanvas.create((lwBaseControl*)&g_window, gl_options);
	g_glcanvas.maximise();

    // -- Redraw the window
	g_window.redraw();

	// -- Spawn a rendering thread for the Canvas (Multi threaded APP)
	g_gl_thread = std::thread(threadOpenGLRender);

	// -- Run the message/main loop
	printf( "Main loop begins\n" );
	g_application->messageLoop();
	printf( "Main loop ended\n" );

	// -- Clean up the windows, dialogs, widgets and threads
	g_gl_thread_quit = true;
	g_gl_thread.join();
	g_glcanvas.destroy();
	g_window.destroy();
	delete g_application;

	return 0;
}
