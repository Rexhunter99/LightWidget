
#include "lw-main.h"
#include "lw-resources.h"
#include <cstdio>
#include <cstdlib>
#include <memory.h>
#include <chrono>
#include <thread>

#include <gl/gl.h>

using namespace std;


/* The definition of these instances can go anywhere all except the
 * lwApplication instance, you need to have this created outside the
 * main() function, so leave it in main.cpp */
lwApplication	*g_application = nullptr;
lwFrame			g_window;
lwToolTip		tip;
lwMenuBar		menubar;
lwToolBar		g_toolbar;
lwStatusBar		statusbar;
lwTreeView		treeview;
lwTabGroup		tabs;
lwDialog		g_about_dialog;
lwStatic		dlg_txt;


/* Follows are implementations of event functions used in the test
 * controls created in main() */
void eventFileMenuOpen()
{
	lwFileDialog fd;

	printf( "Event: File Menu Open:\"%s\"\n", fd.open( &g_window ).c_str() );
}

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

void eventHelpMenuAbout( void )
{
	g_window.disable();

	// -- About Dialog
	g_about_dialog.onClose = eventAboutDialogClose;
	g_about_dialog.create( &g_window, "About" );
	g_about_dialog.resize( 280, 120 );
	g_about_dialog.move( (g_application->getScreenWidth()/2) - 140, (g_application->getScreenHeight()/2) - 60 );
	g_about_dialog.show();

	dlg_txt.create( &g_about_dialog, "Test Application\r\nVersion:\t0.0.0\r\nBy James Ogden" );
	dlg_txt.move( 5, 5 );
	dlg_txt.resize( 270-10, 100-10 );
	dlg_txt.show();
}

void eventMainWindowResize( int32_t w, int32_t h )
{
	int32_t window_x, window_y, window_w, window_h;

	g_window.getClientArea( &window_x, &window_y, &window_w, &window_h );

	treeview.move( window_x, window_y );
	treeview.resize( 200, window_h );
}


void threadOpenGLRender()
{
	/// -- NOTE: Can do OpenGL/Direct3D rendering if using a GL/D3D control, or just general runtime processing.
	/*glcanvas.setCurrent();

	glClearColor( 0.0f, 0.0f, 0.2f, 1.0f );
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

	glcanvas.swapBuffers();*/
	this_thread::sleep_for( chrono::milliseconds( 1 ) );
}


int main( int argc, char **argv )
{
	g_application = new lwApplication( argc, argv );
	g_application->setAuthor( "Rexhunter99" );
	g_application->setTitle( "Light Widget: Library Demo" );
	g_application->setVersion( "0.0.0" );

	// -- Used in the OpenGL rendering
	chrono::steady_clock::time_point time_application_start = chrono::steady_clock::now();

	lwImageList project_imagelist;

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
	g_window.acceptFiles = 1;
	if ( !g_window.create( nullptr, "Breeze Games: Game Studio" ) )
	{
		return 1000;
	}
	g_window.resize( 1024, 768 );
	g_window.center();

	// -- Create the menu bar for the main window
	menubar.create( &g_window );
	menubar.addPopupMenu( "", "File" );
		menubar.addItem( "File", "New", nullptr );
		menubar.addItem( "File", "Open", eventFileMenuOpen );
		menubar.addSeparator( "File" );
		menubar.addItem( "File", "Settings", nullptr );
		menubar.addSeparator( "File" );
		menubar.addItem( "File", "Exit", eventButtonClick );
	menubar.addPopupMenu( "", "Help" );
		menubar.addItem( "Help", "Manual", nullptr );
		menubar.addItem( "Help", "About", eventHelpMenuAbout );

	g_toolbar.border = 1;
	g_toolbar.create( &g_window );
	g_toolbar.setImageList( nullptr );
	g_toolbar.addButtonImage( 6, "New" );
	g_toolbar.addButtonImage( 7, "Open" );
	g_toolbar.addButtonImage( 8, "Save" );
	g_toolbar.addSeparator();
	g_toolbar.addButtonImage( 3, "Undo" );
	g_toolbar.addButtonImage( 4, "Redo" );

	if ( project_imagelist.create( 16, 16, 32, 32 ) )
	{
		lwImage icon[3];

		icon[0].createFromFile( "icons/folder-16.bmp", 0xFFFF00FF, 0x00 | treeview.getBKColor() );
		icon[1].createFromFile( "icons/sound-16.bmp", 0xFFFF00FF, 0x00 | treeview.getBKColor() );
		icon[2].createFromFile( "icons/config-16.bmp", 0xFFFF00FF, 0x00 | treeview.getBKColor() );

		project_imagelist.addImage( &icon[0] );
		project_imagelist.addImage( &icon[1] );
		project_imagelist.addImage( &icon[2] );
		//project_imagelist.addSystemImage( 0 );

		if ( project_imagelist.getImageCount() < 3 )
		{
			fprintf( stderr, "wgm | ERROR | Failed to add the images to the imagelist resource!\n" );
		}
	}

	// -- Create a tree view
	treeview.border = 1;
	treeview.itemButtons = 1;
	treeview.itemEditable = 1;
	treeview.create( &g_window, "Project" );
	treeview.setImageList( &project_imagelist );
	treeview.addItemText( "Graphics" );		// ID == 1
		treeview.addItemTextImage( "g_test", 3, "Graphics" );
	treeview.addItemTextImage( "Audio", 1 );
		treeview.addItemText( "s_chime", "Audio" );
		treeview.addItemTextImage( "SubFolder01", 0, "Audio" );
			treeview.addItemText( "s_dummy", "SubFolder01" );
		treeview.addItemText( "s_bells", "Audio" );
	treeview.addItemText( "Shaders" );
	treeview.addItemText( "Scripts" );
	treeview.addItemText( "Fonts" );
	treeview.addItemText( "Objects" );
	treeview.addItemText( "Rooms" );
	treeview.addItemText( "Extensions" );
	treeview.addItemTextImage( "Config", 2 );
	treeview.addItemTextImage( "Info", 3 );
	treeview.move( 256, 16 );
	treeview.resize( 200, 400 );

	// -- Create a statusbar
	int32_t sb_widths[] = { 100, 200, -1 };
	statusbar.create( &g_window, 3, sb_widths );
	statusbar.setSectionText( 0, "Section 1" );
	statusbar.setSectionText( 1, "Section 2" );
	statusbar.setSectionText( 2, "Section 3" );

	// -- Resize and reposition elements properly
	g_toolbar.resize(0,0);
	g_window.redraw();
	menubar.update();

	// -- Run the message/main loop
	printf( "Main loop begins\n" );
	g_application->messageLoop();
	printf( "Main loop ended\n" );

	// -- Clean up the windows and controls
	g_window.destroy();

	project_imagelist.destroy();

	delete g_application;

	return 0;
}
