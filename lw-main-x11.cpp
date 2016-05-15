/*************************************************************
** X11 Implementation
** Light Widget Library: Core-X11
**
** Most commonly used controls for GUI's and core classes
**/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include <cstdio>
#include <map>
#include <queue>
#include <string>
#include <thread>

#include "lw-main.h"
#include "lw-resources.h"


using namespace std;


#ifdef UNICODE
#define __TEXT( s ) L##s
#else
#define __TEXT( s ) s
#endif

#define TEXT(s) __TEXT(s)


enum StyleEnum
{
	S_NONE			= 0,
	S_BORDER,
	S_MAX
};

typedef struct _WindowData
{
	StyleEnum	style;
	intptr_t	lparam;
} WindowData;

// -- Light Widget Globals
struct {
	map<Window,WindowData>	wnd_lparam;
	uint32_t				msg_thread_status;
	thread					msg_thread;
	Display*				x_display;
	Atom					wm_delete_msg,
							wm_quit_msg;
} lw_g;


long WindowProcedure( Window wnd, XEvent* ev )
{
	lwBaseControl* widget = (lwBaseControl*)lw_g.wnd_lparam[ ev->xany.window ].lparam;

	switch (ev->type)
	{
	case ClientMessage:
		{
			if ( ev->xclient.data.l[0] == lw_g.wm_delete_msg )
			{
				if ( widget && widget->onClose ) widget->onClose();
			}
		}
		break;

	case Expose: // WM_PAINT equivilent?
		{
			if ( widget == nullptr )
				break;

			XGCValues values;
			values.background = 0xD9D9D9;
			GC gc = XCreateGC( lw_g.x_display, ev->xexpose.window, GCBackground, &values );

			// -- If the count is not zero, break
			if (ev->xexpose.count != 0)
				break;

			// -- Draw text for (static/edit)
			/*if ( ev.xexpose.window == wnd )
			{
				for ( int l = 0; l<lines.size(); l++ )
				{
					XDrawString( dpy, wnd, gc, 10, 10 + (10*l), lines[l].c_str(), lines[l].size() );
				}
			}*/
			// -- Draw button
			if ( widget->m_type == LW_BUTTON )
			{
				Window root;
				int x, y;
				unsigned int width, height;
				unsigned int border_width;
				unsigned int depth;

				if ( XGetGeometry( lw_g.x_display, ev->xexpose.window, &root, &x, &y, &width, &height, &border_width, &depth) == true )
				{
					uint32_t len = widget->getText( nullptr );
					char* text = new char [ len ];
					widget->getText( text );
					XDrawString( lw_g.x_display, ev->xexpose.window, gc, 1, 10, text, len );
					delete [] text;
				}
			}

			XFreeGC( lw_g.x_display, gc );
		}
		break;

	case ButtonPress:
		if ( widget && widget->onClick ) widget->onClick();
		break;
	}

	return 0;
}


int HandleXErrors( Display *dsp, XErrorEvent *ev )
{
	string es = "";

	switch ( ev->error_code )
	{
		case BadRequest:	es += "bad request code"; break;
		case BadValue:	es += "int parameter out of range"; break;
		case BadWindow:	es += "parameter not a Window"; break;
		case BadPixmap:	es += "parameter not a Pixmap"; break;
		case BadAtom:	es += "parameter not an Atom"; break;
		case BadCursor:	es += "parameter not a Cursor"; break;
		case BadFont:	es += "parameter not a Font"; break;
		case BadMatch:	es += "parameter mismatch"; break;
		case BadDrawable:	es += "parameter not a Pixmap or Window"; break;
		case BadAccess:	es += "depending on context:\n" \
				"- key/button already grabbed\n" \
				"- attempt to free an illegal cmap entry\n" \
				"- attempt to store into a read-only color map entry.\n" \
 				"- attempt to modify the access control\n" \
				"list from other than the local host."; break;
		case BadAlloc:	es += "insufficient resources"; break;
		case BadColor:	es += "no such colormap"; break;
		case BadGC:	es += "parameter not a GC"; break;
		case BadIDChoice:	es += "choice not in range or already used"; break;
		case BadName:	es += "font or color name doesn't exist"; break;
		case BadLength:	es += "Request length incorrect"; break;
		case BadImplementation:	es += "server is defective"; break;
	}
	printf( "XLib Error:\n%s\n", es.c_str() );

	return 0;
}


/************************************************************************
** Light Widget Library Helpers
*/


lwApplication::lwApplication()
{
	XInitThreads();

	XSetErrorHandler( HandleXErrors );

	// -- Create a connection with the XDisplay
	lw_g.x_display = XOpenDisplay( nullptr );

	if ( !lw_g.x_display )
	{
		printf( "lwApplication() :: Failed to get the X Display Connection\n" );
	}

	lw_g.wm_delete_msg = XInternAtom( lw_g.x_display, "WM_DELETE_WINDOW", True );
}

lwApplication::~lwApplication()
{
	// -- The application is finished so we can close our connection to the XDisplay
	if ( !lw_g.x_display )
	{
		XCloseDisplay( lw_g.x_display );
	}
}

size_t lwApplication::getWorkingDirectory( char* p_directory, size_t p_max_length )
{
	return strlen( getcwd( p_directory, p_max_length ) )+1;
}

uint32_t lwApplication::getScreenCount()
{
	return XScreenCount( lw_g.x_display );
}

uint32_t lwApplication::getScreenWidth( void )
{
	return ((Screen*)XScreenOfDisplay( lw_g.x_display, 0 ))->width;
}

uint32_t lwApplication::getScreenHeight( void )
{
	return ((Screen*)XScreenOfDisplay( lw_g.x_display, 0 ))->height;
}

bool lwApplication::messageLoop()
{
	XEvent ev;

	XNextEvent( lw_g.x_display, &ev );

	if ( ev.type == ClientMessage && ev.xclient.data.l[0] == lw_g.wm_delete_msg )
		return false;

	WindowProcedure( 0, &ev );

	return true;
}


/************************************************************************
** lwBaseControl method & operator implementations
*/

lwBaseControl::lwBaseControl()
{
	this->m_handle = 0;
	this->m_type = lwControlTypeEnum::LW_UNKNOWN_CONTROL;

	this->onClick = nullptr;
	this->onClose = nullptr;
	this->onCreate = nullptr;
	this->onDestroy = nullptr;
	this->onDisable = nullptr;
	this->onEnable = nullptr;
	this->onHide = nullptr;
	this->onKeyPress = nullptr;
	this->onKeyRelease = nullptr;
	this->onMove = nullptr;
	this->onRClick = nullptr;
	this->onResize = nullptr;
	this->onShow = nullptr;
}

lwBaseControl::~lwBaseControl()
{
	// -- Catch missed cleanup
    this->destroy();
}

void lwBaseControl::destroy( void )
{
    if ( this->m_handle != 0 )
    {
		XDestroyWindow( lw_g.x_display, this->m_handle );
        this->m_handle = 0;
    }
}

bool lwBaseControl::setFont( lwFont* p_font )
{
	//SendMessage( this->m_handle, WM_SETFONT, (WPARAM)p_font->m_handle, MAKELPARAM(TRUE, 0) );
	//XSetFont();
	return true;
}

bool lwBaseControl::setText( const char* p_text )
{
	if ( this->m_handle == 0 ) return (false);

	XStoreName( lw_g.x_display, this->m_handle, p_text );

	return (true);
}

uint32_t lwBaseControl::getText( char* p_text )
{
	if ( this->m_handle == 0 ) return (false);

	XTextProperty xtp;

	XGetWMName( lw_g.x_display, this->m_handle, &xtp );

	if ( p_text != nullptr )
	{
		memcpy( p_text, xtp.value, xtp.nitems );
		*( p_text + xtp.nitems + 1 ) = '\0';
	}

	return xtp.nitems;
}

bool lwBaseControl::show( void )
{
	if ( this->m_handle == 0 ) return (false);

	XMapWindow( lw_g.x_display, this->m_handle );

	return (true);
}

bool lwBaseControl::hide( void )
{
	if ( this->m_handle == 0 ) return (false);

	XLockDisplay( lw_g.x_display );
	XUnmapWindow( lw_g.x_display, this->m_handle );
	XUnlockDisplay( lw_g.x_display );

	return (true);
}

bool lwBaseControl::enable()
{
    if ( this->m_handle == 0 ) return (false);

    return (true);
}

bool lwBaseControl::disable()
{
    if ( this->m_handle == 0 ) return (false);

    return (true);
}

void lwBaseControl::resize( uint32_t p_width, uint32_t p_height )
{
	if ( this->m_handle == 0 ) return;

	XResizeWindow( lw_g.x_display, this->m_handle, p_width, p_height );
}

void lwBaseControl::move( uint32_t p_x, uint32_t p_y )
{
	if ( this->m_handle == 0 ) return;

	XMoveWindow( lw_g.x_display, this->m_handle, p_x, p_y );
}

void lwBaseControl::redraw()
{
	if ( this->m_handle == 0 ) return;
}

bool lwBaseControl::focus()
{
	if ( this->m_handle == 0 ) return false;

	XSetInputFocus( lw_g.x_display, this->m_handle, RevertToParent, CurrentTime );

	return true;
}


/************************************************************************
** lwToolTip method implementations
*/
// TODO: clean up and optimise create() method

lwToolTip::lwToolTip()
{
	this->m_type = lwControlTypeEnum::LW_TOOLTIP;
    m_handle = 0;
    control = 0;
}
lwToolTip::~lwToolTip()
{
    this->destroy();
}

bool lwToolTip::create( lwBaseControl* p_parent, lwBaseControl* p_target_control, const char* p_tip_text)
{
	if ( p_parent == nullptr || p_target_control == nullptr || p_tip_text == nullptr )
		return false;

    return true;
}


/************************************************************************
** lwDialog method implementations
*/

lwFrame::lwFrame() : lwBaseControl()
{
	this->m_type = lwControlTypeEnum::LW_FRAME;
}

lwFrame::~lwFrame()
{
}

bool lwFrame::create( lwBaseControl* p_parent, const char* p_title )
{
	if ( this->m_handle != 0 )
		return false;

	XLockDisplay( lw_g.x_display );

	this->m_handle = XCreateSimpleWindow(	lw_g.x_display,
											RootWindow( lw_g.x_display, 0 ),
											0,0, 1,1,
											0,
											0x00,
											0xD9D9D9 );

	if ( this->m_handle == 0 )
		return false;

	XStoreName( lw_g.x_display, this->m_handle, p_title );

	XSetWMProtocols( lw_g.x_display, this->m_handle, &lw_g.wm_delete_msg, 1 );

	XSelectInput( lw_g.x_display, this->m_handle, ExposureMask | ButtonPressMask | ButtonReleaseMask );

	XMapWindow( lw_g.x_display, this->m_handle );

	XFlush( lw_g.x_display );

	XUnlockDisplay( lw_g.x_display );

	WindowData wd;
	wd.lparam = (intptr_t)this;
	wd.style = StyleEnum::S_BORDER;
	lw_g.wnd_lparam.insert( pair<Window,WindowData>(this->m_handle,wd) );

	return true;
}

void lwFrame::close()
{
	if ( this->m_handle == 0 ) return;

	XEvent ev;
	memset(&ev, 0, sizeof (XEvent));

	ev.xclient.type = ClientMessage;
	ev.xclient.window = this->m_handle;
	ev.xclient.message_type = XInternAtom( lw_g.x_display, "WM_PROTOCOLS", true );
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = XInternAtom( lw_g.x_display, "WM_DELETE_WINDOW", false );
	ev.xclient.data.l[1] = CurrentTime;

	XSendEvent( lw_g.x_display, this->m_handle, False, NoEventMask, &ev );
}

void lwFrame::quit()
{
	if ( this->m_handle == 0 ) return;

	XEvent ev;
	memset(&ev, 0, sizeof (XEvent));

	ev.xclient.type = ClientMessage;
	ev.xclient.window = this->m_handle;
	ev.xclient.message_type = XInternAtom( lw_g.x_display, "WM_PROTOCOLS", true );
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = XInternAtom( lw_g.x_display, "WM_DELETE_WINDOW", false );
	ev.xclient.data.l[1] = CurrentTime;

	XSendEvent( lw_g.x_display, this->m_handle, False, NoEventMask, &ev );
}


bool lwFrame::run()
{
	if ( this->m_handle == 0 ) return true;

	XEvent ev;

	if ( XCheckWindowEvent( lw_g.x_display, this->m_handle, 0xFFFFFF, &ev ) )
	{
		if ( ev.type == ClientMessage && ev.xclient.data.l[0] == lw_g.wm_delete_msg )
			return false;

		WindowProcedure( 0, &ev );
	}

	/*if ( XPending( lw_g.x_display ) > 0 )
	{
		XNextEvent( lw_g.x_display, &ev );

		if ( ev.type == ClientMessage && ev.xclient.data.l[0] == lw_g.wm_delete_msg )
			return false;

		WindowProcedure( 0, &ev );
	}*/

	return true;
}


/************************************************************************
** lwDialog method implementations
*/

lwDialog::lwDialog()
{
	this->m_type = lwControlTypeEnum::LW_DIALOG;
}

lwDialog::~lwDialog()
{
	this->destroy();
}

bool lwDialog::create( lwBaseControl* p_parent, tstring p_title )
{
	if ( p_parent == nullptr )
		return false;

	//this->m_handle = XCreateSimpleWindow();

	if ( this->m_handle == 0 )
		return false;

	return true;
}


/************************************************************************
** lwFileDialog method implementations
*/

lwFileDialog::lwFileDialog( )
{
	this->m_default_extension = TEXT("*.*");
	this->m_extension_filter = TEXT("All Files (*.*)\0*.*\0");
	this->m_initial_directory = TEXT("");
	this->m_path_filename = TEXT("");
	this->m_title = TEXT("");
}

void lwFileDialog::setInitialDirectory( tstring p_initial_directory )
{
	this->m_initial_directory = p_initial_directory;
}

void lwFileDialog::setDefaultExt( tstring p_default_extension )
{
	this->m_default_extension = p_default_extension;
}

void lwFileDialog::setFilters( tstring p_filters )
{
	this->m_extension_filter = p_filters;
}

void lwFileDialog::setTitle( tstring p_title )
{
	this->m_title = p_title;
}

tstring lwFileDialog::open( lwBaseControl* p_window )
{
	Window wnd;

	wnd = XCreateSimpleWindow(	lw_g.x_display,
								RootWindow( lw_g.x_display, 0 ),
								0, 0, 0, 0,
								4, 0,
								0xFF00FFFF);

	XMapWindow( lw_g.x_display, wnd );
	XFlush( lw_g.x_display );

	XDestroyWindow( lw_g.x_display, wnd );

	return tstring( TEXT("") );
}

tstring lwFileDialog::save( lwBaseControl* p_window )
{
	return tstring( TEXT("") );
}


/************************************************************************
** lwTabGroup method implementations
*/

lwTabGroup::lwTabGroup()
{
	this->m_type = lwControlTypeEnum::LW_TABS;
}

lwTabGroup::~lwTabGroup()
{
	this->m_tabs.clear();
	this->destroy();
}

bool lwTabGroup::create( lwBaseControl* p_parent )
{
	if ( p_parent == nullptr )
		return false;

	XLockDisplay( lw_g.x_display );

	this->m_handle = XCreateSimpleWindow(	lw_g.x_display,
											p_parent->m_handle,
											0,0,1,1,
											1,
											0x00,
											0xD9D9D9 );

	if ( this->m_handle == 0 )
	{
		printf( "lwButton::create() - Failed to create the X Window\n" );
		return false;
	}

	XMapWindow( lw_g.x_display, this->m_handle );

	XFlush( lw_g.x_display );

	XUnlockDisplay( lw_g.x_display );

	lw_g.wnd_lparam[this->m_handle].lparam = (intptr_t)this;

	if ( this->onCreate ) this->onCreate();

	return  true;
}

void lwTabGroup::resize( uint32_t p_width, uint32_t p_height )
{
	lwBaseControl::resize( p_width, p_height );
}

bool lwTabGroup::addTab( tstring p_name )
{
	return false;
}

bool lwTabGroup::addControlToTab( tstring p_name, lwBaseControl* p_control )
{
	for ( uint32_t i=0; i<this->m_tabs.size(); i++ )
	{
		if ( this->m_tabs[i].name == p_name )
		{
			this->m_tabs[i].children.push_back( p_control );
			return true;
		}
	}

	return false;
}

void lwTabGroup::getClientArea( int32_t& p_left, int32_t& p_top, int32_t& p_width, int32_t& p_height )
{
}


/************************************************************************
** lwComboBox method implementations
*/

lwComboBox::lwComboBox()
{
	this->m_type = lwControlTypeEnum::LW_COMBO_BOX;
}

lwComboBox::~lwComboBox()
{
	this->destroy();
}

bool lwComboBox::create( lwBaseControl* p_parent, const char *p_title )
{
	return true;
}

void lwComboBox::addItem( const char* p_name )
{
}

void lwComboBox::setSelelection( uint16_t p_index )
{
}

uint16_t lwComboBox::getSelection( void )
{
	return 0;
}

void lwComboBox::clearItems( void )
{
}


/************************************************************************
** lwListBox method implementations
*/

lwListBox::lwListBox()
{
	this->m_type = lwControlTypeEnum::LW_LIST_BOX;
}

lwListBox::~lwListBox()
{
	this->destroy();
}

bool lwListBox::create( lwBaseControl* p_parent, char *p_title )
{
	return true;
}

void lwListBox::addItem( const char* p_name )
{
}

void lwListBox::setSelection( uint16_t p_index )
{
}

uint16_t lwListBox::getSelection( void )
{
	return 0;
}

void lwListBox::clearItems( void )
{
}


//

lwListView::lwListView()
{
	this->m_type = lwControlTypeEnum::LW_LIST_VIEW;
}

lwListView::~lwListView()
{
	this->destroy();
}

bool lwListView::create( lwBaseControl* p_parent, const char* p_title )
{
	return true;
}

bool lwListView::setImageList( lwImageList* p_image_list )
{
	return true;
}

uint32_t lwListView::addItemText( const char* p_text, int p_item )
{
	return 0;
}

uint32_t lwListView::addItemTextImage( const char* p_text, int p_image, int p_item )
{
	return 0;
}


//

lwTreeView::lwTreeView()
{
	this->m_type = lwControlTypeEnum::LW_TREEVIEW;
}

lwTreeView::~lwTreeView()
{
	this->destroy();
}

bool lwTreeView::create( lwBaseControl* p_parent, const char* p_title )
{
	if ( p_parent == nullptr )
		return false;

	XLockDisplay( lw_g.x_display );

	printf( "lwTreeView::create() - create button\n" );

	this->m_handle = XCreateSimpleWindow(	lw_g.x_display,
											p_parent->m_handle,
											0,0,1,1,
											1,
											0x00,
											0xFFFFFF );

	if ( this->m_handle == 0 )
	{
		printf( "lwTreeView::create() - Failed to create the X Window\n" );
		return false;
	}

	printf( "lwTreeView::create() - Map window\n" );

	XMapWindow( lw_g.x_display, this->m_handle );

	XFlush( lw_g.x_display );

	XUnlockDisplay( lw_g.x_display );

	lw_g.wnd_lparam[this->m_handle].lparam = (intptr_t)this;

	return  true;
}

bool lwTreeView::setImageList( lwImageList* p_image_list )
{
	return true;
}

uint32_t lwTreeView::addItemText( string p_text, int p_parent )
{
	return 0;
}

uint32_t lwTreeView::addItemTextImage( string p_text, int p_image, int p_parent )
{
	return 0;
}


//

lwButton::lwButton()
{
	this->m_type = lwControlTypeEnum::LW_BUTTON;
}

lwButton::~lwButton()
{
	this->destroy();
}

bool lwButton::create( lwBaseControl* p_parent, const char *p_title )
{
	if ( p_parent == nullptr )
		return false;

	XLockDisplay( lw_g.x_display );

	this->m_handle = XCreateSimpleWindow(	lw_g.x_display,
											p_parent->m_handle,
											0,0,1,1,
											1,
											0x00,
											0xD9D9D9 );

	if ( this->m_handle == 0 )
	{
		printf( "lwButton::create() - Failed to create the X Window\n" );
		return false;
	}

	XStoreName( lw_g.x_display, this->m_handle, p_title );

	XSelectInput( lw_g.x_display, this->m_handle, ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask );

	XMapWindow( lw_g.x_display, this->m_handle );

	XFlush( lw_g.x_display );

	XUnlockDisplay( lw_g.x_display );


	WindowData wd;
	wd.lparam = (intptr_t)this;
	wd.style = S_BORDER;
	lw_g.wnd_lparam.insert( pair<Window,WindowData>(this->m_handle,wd) );

	return  true;
}

void lwButton::setBitmap( lwImage* p_image )
{
}


//

lwTextArea::lwTextArea()
{
	this->m_type = lwControlTypeEnum::LW_TEXTAREA;
}

lwTextArea::~lwTextArea()
{
	this->destroy();
}

bool lwTextArea::create( lwBaseControl* p_parent )
{
	if ( p_parent == nullptr )
		return false;

	return  true;
}


//

lwGroup::lwGroup()
{
	this->m_type = lwControlTypeEnum::LW_GROUP;
}

lwGroup::~lwGroup()
{
	this->destroy();
}

void lwGroup::create( lwBaseControl* p_parent, char *p_title )
{
}


//

lwCheckBox::lwCheckBox()
{
	this->m_type = lwControlTypeEnum::LW_CHECK_BOX;
}

lwCheckBox::~lwCheckBox()
{
	this->destroy();
}

bool lwCheckBox::create( lwBaseControl* p_parent, const char* p_title )
{
	return true;
}

void lwCheckBox::setCheck( bool p_check )
{
}

bool lwCheckBox::getCheck( void )
{
	return true;
}


//

lwStatic::lwStatic()
{
	this->m_type = lwControlTypeEnum::LW_STATIC;
}

lwStatic::~lwStatic()
{
	this->destroy();
}

bool lwStatic::create( lwBaseControl* p_parent, const char* p_title )
{
	if ( p_parent == nullptr )
		return false;

	return true;
}

void lwStatic::setImage( lwImage* p_image )
{
}


//

lwMenuBar::lwMenuBar()
{
	m_handle = 0;
}

lwMenuBar::~lwMenuBar()
{
	m_submenus.clear();
	m_events.clear();
}

bool lwMenuBar::create( lwBaseControl* p_parent )
{
	return true;
}

bool lwMenuBar::addPopupMenu( tstring p_menu_name, tstring p_name )
{
	return true;
}

bool lwMenuBar::addItem( tstring p_menu_name, tstring p_name, lwFuncType p_event )
{
	return 0;
}

bool lwMenuBar::insertItem( tstring p_menu_name, uint32_t p_pos, tstring p_name, lwFuncType p_event )
{
	return 0;
}

void lwMenuBar::removeItem( tstring p_menu_name, uint32_t p_pos )
{
}

bool lwMenuBar::addSeparator( tstring p_menu_name )
{
	return false;
}

bool lwMenuBar::addBreak( tstring p_menu_name )
{
	return false;
}

void lwMenuBar::update()
{
}

/// Private Method
WINDOW_T lwMenuBar::i_findMenu( tstring p_name )
{
	return 0;
}


//

lwStatusBar::lwStatusBar()
{
	this->m_type = lwControlTypeEnum::LW_STATUS_BAR;
}

lwStatusBar::~lwStatusBar()
{
	this->destroy();
}

bool lwStatusBar::create( lwFrame* p_parent, uint16_t p_parts, int32_t* p_part_width )
{
	if ( p_parent == nullptr ) return false;

	return true;
}

void lwStatusBar::setSectionText( uint16_t p_index, char *p_text )
{
}

void lwStatusBar::resize( uint32_t p_width, uint32_t p_height )
{
}

uint32_t lwStatusBar::getHeight()
{
	return 0;
}


//

lwProgressBar::lwProgressBar()
{
	this->m_type = lwControlTypeEnum::LW_PROGRESS_BAR;
}

lwProgressBar::~lwProgressBar()
{
	this->destroy();
}

bool lwProgressBar::create( lwBaseControl* p_parent )
{
	if ( p_parent == 0 )
		return false;


	return true;
}

void lwProgressBar::setMarquee( bool p_marquee, uint32_t p_update_ms )
{
}

void lwProgressBar::setRange( uint16_t p_min, uint16_t p_max )
{
}

void lwProgressBar::setPosition( uint16_t p_index )
{
}

void lwProgressBar::setState( uint16_t p_state )
{
}

void lwProgressBar::setStep( uint16_t p_step_size )
{
}

void lwProgressBar::step( void )
{
}

void lwProgressBar::setBackgroundColor( uint32_t p_rgba )
{
}

void lwProgressBar::setBarColor( uint32_t p_rgba )
{
}


//

lwToolBar::lwToolBar()
{
}

lwToolBar::~lwToolBar()
{
	this->destroy();
}

bool lwToolBar::create( lwBaseControl* p_parent )
{
	if ( p_parent == nullptr )
		return false;

	return true;
}

void lwToolBar::setImageList( lwImageList* p_image_list )
{
}

uint32_t lwToolBar::addButton(unsigned int idBitmap)
{
	return 0;
}

uint32_t lwToolBar::addSeparator(unsigned int idBitmap)
{
	return 0;
}

void lwToolBar::removeButton( uint32_t p_index )
{
}

void lwToolBar::resize( uint32_t p_width, uint32_t p_height )
{
}



/************************************************************************************************************
** lw-resources.h
************************************************************************************************************/

/************************************************************************
** lwImage : Image(BMP/TGA/PNG) resource implementation
*/

lwImage::lwImage()
{
	this->bitmap = 0;
}

lwImage::~lwImage()
{
	this->destroy();
}

bool lwImage::create( uint32_t p_width, uint32_t p_height, uint8_t p_bitdepth, void* p_data )
{
	return true;
}

void lwImage::destroy()
{
}


/************************************************************************
** lwImageList : List of images implementation
*/

lwImageList::lwImageList()
{
	this->image_list.clear();
}

lwImageList::~lwImageList()
{
	this->destroy();
}

bool lwImageList::create( uint32_t p_width, uint32_t p_height, uint8_t p_bitdepth, uint16_t p_maximum_images )
{
	return true;
}

void lwImageList::destroy()
{
}

bool lwImageList::addImage( lwImage* p_image )
{
	return true;
}

uint32_t lwImageList::getImageCount( void )
{
	return this->image_list.size();
}


/************************************************************************
** lwBaseControl method & operator implementations
*/

lwFont::lwFont()
{
	this->m_handle = 0;
	this->m_resource = TEXT("");
}

lwFont::~lwFont()
{
	this->destroy();
}

bool lwFont::create( tstring p_fontname, uint32_t p_size, bool p_bold, bool p_italic )
{
	if ( this->m_handle == 0 )
		return false;

	return true;
}

bool lwFont::load( tstring p_filename )
{
	return true;
}

void lwFont::destroy()
{
	//if ( this->m_handle != nullptr )
		//XFree( this->m_handle )

	//if ( this->m_resource == "" )
		//RemoveFontResource( this->m_resource.c_str() );

	this->m_handle = 0;
	this->m_resource = TEXT("");
}
