/*************************************************************
** Win32 & COM Implementation
** Light Widget Library: Core
**
** Most commonly used controls for GUI's
**/

/// WinAPI version stuff for IE and OS
#undef WINVER
#undef _WIN32_IE
#undef _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_IE       	0x0800      // IE 8
#define _WIN32_WINNT    	0x0601      // Windows 7
#define WINVER          	0x0601      // Windows 7

/// Define styles and message tokens here that aren't in the GNU Windows headers
#define PBS_MARQUEE			0x08
#define PBM_SETMARQUEE		WM_USER + 10
#define PBM_SETSTATE		WM_USER + 16

#define SS_EDITCONTROL      0x00002000L
#define SS_ENDELLIPSIS      0x00004000L
#define SS_PATHELLIPSIS     0x00008000L
#define SS_WORDELLIPSIS     0x0000C000L
#define SS_ELLIPSISMASK     0x0000C000L

#define IDB_BITMAP1			129
#define IDB_BACKGRND		130
#define IDB_OPEN_FILE		131
#define IDB_CLOSED_FILE		132
#define IDB_DOCUMENT		133

#undef lwNativeWindowType
#define lwNativeWindowType HWND

#include <Windows.h>
#include <CommCtrl.h>
#include <Windowsx.h>

#include <cstdio>
#include <string>
#include <fstream>

#include "lw-main.h"
#include "lw-resources.h"

#include <map>
#include <vector>


using namespace std;


WNDCLASSEX  g_window_frame_class,
			g_window_dialog_class;
lwFrame     g_desktop_window;


LONG_PTR APIENTRY WindowProcedure( HWND handle, UINT message, WPARAM wparam, LPARAM lparam )
{
	lwBaseControl* widget = (lwBaseControl*)GetWindowLongPtr( handle, GWLP_USERDATA );

	switch ( message )
	{
	// -- Special case to help the ::create methods
	case WM_NCCREATE:
		{
			CREATESTRUCT *cs = (CREATESTRUCT*)lparam;
			SetWindowLongPtr(handle, GWLP_USERDATA, (LONG)cs->lpCreateParams );
		}
		return DefWindowProc( handle, message, wparam, lparam );

	case WM_CREATE:
		if ( widget && widget->onCreate != nullptr ) widget->onCreate();
		return 0;

	case WM_DESTROY:
		if ( widget && widget->onDestroy != nullptr ) widget->onDestroy();
		return 0;

	case WM_SIZE:
		{
			if ( widget && widget->onResize != nullptr ) widget->onResize( LOWORD(lparam), HIWORD(lparam) );
			// -- Resize the status bar
			if ( widget->m_type == lwWidgetTypeEnum::LW_FRAME )
			{
				lwFrame* frame = reinterpret_cast<lwFrame*>(widget);
				if ( widget != nullptr && frame->m_status_bar != nullptr )
				{
					frame->m_status_bar->resize( 0, 0 );
				}
			}
		}
		return DefWindowProc( handle, message, wparam, lparam );

	case WM_ENABLE:
		{
			if ( wparam == false )
			{
				if ( widget && widget->onDisable != nullptr ) widget->onDisable();
			}
			else
			{
				if ( widget && widget->onEnable != nullptr ) widget->onEnable();
			}
		}
		return 0;

	case WM_SHOWWINDOW:
		{
			if ( wparam == false )
			{
				if ( widget && widget->onHide != nullptr ) widget->onHide();
			}
			else
			{
				if ( widget && widget->onShow != nullptr ) widget->onShow();
			}
		}
		return 0;

	case WM_CLOSE:
		if ( widget && widget->onClose != nullptr ) widget->onClose();
		PostQuitMessage(0);
		return 0;

	case WM_LBUTTONDOWN:
		if ( widget && widget->onClick != nullptr ) widget->onClick( );
		return 0;

	case WM_KEYDOWN:
		printf( "WindowProcedure :: Key Down\n" );
		if ( widget && widget->onKeyPress != nullptr ) widget->onKeyPress( (uint16_t)wparam );
		return 0;

	case WM_COMMAND:
		{
			// -- Get the window's class pointer through the handle
			widget = (lwBaseControl*)GetWindowLongPtr( (HWND)lparam, GWLP_USERDATA );

			switch ( HIWORD(wparam) )
			{
			case BN_CLICKED:
				if ( widget && widget->onClick != nullptr ) widget->onClick( );
				printf( "WindowProcedure :: WM_COMMAND - BN_CLICKED\n" );
				break;
			}
		}
		return 0;

	case WM_DROPFILES:
		{
			HDROP drop = (HDROP) wparam;
			char file[MAX_PATH];
			lwStringType output = "";

			for ( unsigned int i = 0; i < DragQueryFile(drop, 0xFFFFFFFF, file, MAX_PATH); i++ )
			{
				if (!DragQueryFile(drop, i, file, MAX_PATH)) continue;
				output += lwStringType(file) + "\n";
			}
			MessageBox(handle, output.c_str(), "Dropped", MB_OK);

			DragQueryFile(drop, 0, file, MAX_PATH);

			DragFinish(drop);

			//loadCAR(file);
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR* nh = (NMHDR*)lparam;
			switch ( nh->code )
			{
			case TCN_SELCHANGE:
				{
					lwTabGroup *tb = (lwTabGroup*)GetWindowLongPtr( (HWND)nh->hwndFrom, GWLP_USERDATA );
					int32_t selected_tab = TabCtrl_GetCurSel( (HWND)tb->m_handle );

					for ( int32_t i = 0; i < (int32_t)tb->m_tabs.size(); i++ )
					{
						for ( int32_t c=0; c < (int32_t)tb->m_tabs[i].children.size(); c++ )
						if ( selected_tab == i )
						{
							tb->m_tabs[i].children[c]->show();
						}
						else
						{
							tb->m_tabs[i].children[c]->hide();
						}
					}
					//printf( "WindowProcedure :: WM_NOTIFY - TCN_SELCHANGE, w:%p c:%p t:%u \"%s\"\n", handle, nh->hwndFrom, tb->m_tabs.size(), tb->m_tabs[selected_tab].name.c_str() );
				}
				break;

			case TVN_BEGINLABELEDIT:
				{
					LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO)lparam;
					TVITEM* tvi = &ptvdi->item;
					lwTreeView *tv = (lwTreeView*)GetWindowLongPtr( (HWND)nh->hwndFrom, GWLP_USERDATA );

					if ( tv->onItemEditBegin != nullptr )
					{
						return tv->onItemEditBegin( lwStringType(tvi->pszText) );	// -- onItemEditBegin should return false if it allows the item to be edited
					}
					else
					{
						return false;
					}
				}
				break;

			case TVN_ENDLABELEDIT:
				{
					LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO)lparam;
					TVITEM* tvi = &ptvdi->item;
					lwTreeView *tv = (lwTreeView*)GetWindowLongPtr( (HWND)nh->hwndFrom, GWLP_USERDATA );

					if ( tvi->pszText != nullptr )
					{
						printf( "TVN_ENDLABELEDIT :: Changed string to \"%s\"\n", tvi->pszText );
						if ( tv->onItemEditEnd != nullptr )
						{
							return tv->onItemEditEnd( lwStringType(tvi->pszText) );
						}
						else
						{
							return true;
						}
					}
					else
					{
						// -- Discard the changes (there weren't any)
						return false;
					}

				}
				break;
			}
		}
		return 0;

	case WM_MENUCOMMAND:
		{
			MENUINFO mi;
			ZeroMemory( &mi, sizeof(MENUINFO) );
			mi.cbSize = sizeof(MENUINFO);
			mi.fMask = MIM_MENUDATA;
			GetMenuInfo( (HMENU)lparam, &mi );

			lwMenuBar* mb = (lwMenuBar*)mi.dwMenuData;

			MENUITEMINFO mii;
			ZeroMemory( &mii, sizeof(MENUITEMINFO) );
			mii.cbSize		= sizeof(MENUITEMINFO);
			mii.fMask		= MIIM_ID | MIIM_DATA;
			mii.dwTypeData	= nullptr;
			GetMenuItemInfo( (HMENU)lparam, wparam, true, &mii );

			//printf( "WindowProcedure :: WM_MENUCOMMAND Pos:%u Handle:%p Class:%p ID:%u\n", wparam, (void*)lparam, mb, mii.wID );

			if ( mb != nullptr )
			{
				lwFuncType f = mb->m_events[mii.wID];
				if ( f != nullptr ) f();
			}
		}
		return 0;

	default:
		return DefWindowProc( handle, message, wparam, lparam );
	}

	return 0;
}

LONG_PTR CALLBACK DialogProcedure( HWND handle, UINT message, WPARAM wparam, LPARAM lparam )
{
	lwBaseControl* widget = (lwBaseControl*)GetWindowLongPtr( handle, GWLP_USERDATA );

	switch ( message )
	{
	// -- Special case to help the ::create methods
	case WM_NCCREATE:
		{
			CREATESTRUCT *cs = (CREATESTRUCT*)lparam;
			SetWindowLongPtr(handle, GWLP_USERDATA, (LONG)cs->lpCreateParams );
		}
		return DefWindowProc( handle, message, wparam, lparam );

    case WM_CLOSE:
        if ( widget && widget->onClose != nullptr ) widget->onClose();
        return 0;

	case WM_CREATE:
		if ( widget && widget->onCreate != nullptr ) widget->onCreate();
		return 0;

	case WM_DESTROY:
		if ( widget && widget->onDestroy != nullptr ) widget->onDestroy();
		return 0;

	case WM_SHOWWINDOW:
		{
			if ( wparam == false )
			{
				if ( widget && widget->onHide != nullptr ) widget->onHide();
			}
			else
			{
				if ( widget && widget->onShow != nullptr ) widget->onShow();
			}
		}
		return 0;
	}

	return DefWindowProc( handle, message, wparam, lparam );
}

/************************************************************************
** Light Widget Library Helpers
*/

ULONG_PTR EnableVisualStyles(VOID)
{
	// THIS IS A HACK!  NOT RECCOMMENDED!
	// I FOUND THIS HERE: http://stackoverflow.com/questions/4308503/how-to-enable-visual-styles-without-a-manifest
    TCHAR dir[MAX_PATH];
    ULONG_PTR ulpActivationCookie = FALSE;
    ACTCTX actCtx =
    {
        sizeof(actCtx),
        ACTCTX_FLAG_RESOURCE_NAME_VALID
            | ACTCTX_FLAG_SET_PROCESS_DEFAULT
            | ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID,
        TEXT("shell32.dll"), 0, 0, dir, (LPCTSTR)124
    };
    UINT cch = GetSystemDirectory(dir, sizeof(dir) / sizeof(*dir));
    if (cch >= sizeof(dir) / sizeof(*dir)) { return FALSE; /*shouldn't happen*/ }
    dir[cch] = TEXT('\0');
    ActivateActCtx(CreateActCtx(&actCtx), &ulpActivationCookie);
    return ulpActivationCookie;
}

/*
* lwInitComCtl() Function
* Initializes the Common Control Classes
*/
lwApplication::lwApplication( int p_arg_count, char* p_arguments[] )
{
	// -- Initialise the common controls library to v6.1 capabilities
	INITCOMMONCONTROLSEX icex;
	icex.dwSize =		sizeof(INITCOMMONCONTROLSEX);
	/*icex.dwICC =		ICC_TAB_CLASSES | ICC_LINK_CLASS | ICC_BAR_CLASSES | ICC_TREEVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES;*/
	icex.dwICC =        0xBFFF; // -- All Version 6.1 valid controls.
	InitCommonControlsEx( &icex );

	// -- HACK: Enable Visual Styles for Windows (Avoids using a manifest file which is clunky)
	EnableVisualStyles();

	// -- Register a custom Window Class specifically for our Window Frame type
	{
		WNDCLASSEX& wc = g_window_frame_class;
		wc.cbSize				= sizeof(WNDCLASSEX);
		wc.cbClsExtra			= 0;
		wc.cbWndExtra			= 0;
		wc.hbrBackground		= (HBRUSH) COLOR_WINDOW;
		wc.hCursor				= LoadCursor( nullptr, IDC_ARROW );
		wc.hIcon				= LoadIcon( nullptr, IDI_APPLICATION );
		wc.hIconSm				= nullptr;
		wc.hInstance			= GetModuleHandle( nullptr );
		wc.lpfnWndProc			= WindowProcedure;
		wc.lpszClassName		= "LW_WINDOWFRAME";
		wc.lpszMenuName			= nullptr;
		wc.style				= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;//CS_OWNDC | CS_DBLCLKS;

		if ( !RegisterClassEx( &g_window_frame_class ) )
		{
			printf( "lwError: Failed to register the LW_WINDOWFRAME class!\n" );
		}
	}

	// -- Register a custom Window Class specifically for our Window Dialog type
	{
	    WNDCLASSEX& wc = g_window_dialog_class;
        wc.cbSize				= sizeof(WNDCLASSEX);
        wc.cbClsExtra			= 0;
        wc.cbWndExtra			= 0;
        wc.hbrBackground		= (HBRUSH) COLOR_WINDOW;
        wc.hCursor				= LoadCursor( nullptr, IDC_ARROW );
        wc.hIcon				= LoadIcon( nullptr, IDI_APPLICATION );
        wc.hIconSm				= nullptr;
        wc.hInstance			= GetModuleHandle( nullptr );
        wc.lpfnWndProc			= DialogProcedure;
        wc.lpszClassName		= "LW_WINDOWDIALOG";
        wc.lpszMenuName			= nullptr;
        wc.style				= CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW;//CS_OWNDC | CS_DBLCLKS;

        if ( !RegisterClassEx( &g_window_dialog_class ) )
        {
            printf( "lwError: Failed to register the LW_WINDOWDIALOG class! Error: %u\n", (unsigned int)GetLastError() );
        }
	}

    // -- Initialise the desktop frame
	g_desktop_window.m_handle = HWND_DESKTOP;
}

lwApplication::~lwApplication()
{
	UnregisterClass( "LW_WINDOWFRAME", GetModuleHandle( nullptr ) );
	UnregisterClass( "LW_WINDOWDIALOG", GetModuleHandle( nullptr ) );
}

void lwApplication::setAuthor( lwStringType p_author )
{
	this->m_author = p_author;
}

void lwApplication::setTitle( lwStringType p_title )
{
	this->m_title = p_title;
}

void lwApplication::setVersion( lwStringType p_version )
{
	this->m_version = p_version;
}

lwStringType lwApplication::getAuthor( ) const
{
	return this->m_author;
}

lwStringType lwApplication::getTitle( ) const
{
	return this->m_title;
}

lwStringType lwApplication::getVersion( ) const
{
	return this->m_version;
}

size_t lwApplication::getWorkingDirectory( char* p_directory, size_t p_max_length )
{
	return GetModuleFileName( nullptr, p_directory, p_max_length );
}

uint32_t lwApplication::getScreenCount( void ) const
{
	return GetSystemMetrics( SM_CMONITORS ); // Only retrieves visible monitors/screens, not virtual ones or mirrors for drivers (eg; Optimus)
}

uint32_t lwApplication::getScreenWidth( void ) const
{
	return GetSystemMetrics( SM_CXSCREEN );
}

uint32_t lwApplication::getScreenHeight( void ) const
{
	return GetSystemMetrics( SM_CYSCREEN );
}

bool lwApplication::messageLoop()
{
	MSG msg;

	do
	{
		if ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
		{
			if ( msg.message == WM_QUIT )
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	} while ( msg.message != WM_QUIT );

	return true; //return msg.wParam;
}


/************************************************************************
** lwBaseControl method & operator implementations
*/

lwBaseControl::lwBaseControl()
{
	this->m_handle = nullptr;
	this->m_type = lwWidgetTypeEnum::LW_UNKNOWN_WIDGET;

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
    if ( this->m_handle != nullptr )
    {
		DestroyWindow( (HWND)this->m_handle );
		this->m_handle = nullptr;
    }
}

bool lwBaseControl::setText( lwStringType p_text )
{
	if ( this->m_handle == nullptr ) return (false);

	SetWindowText( (HWND)this->m_handle, p_text.c_str() );

	return (true);
}

bool lwBaseControl::setFont( lwFont* p_font )
{
	SendMessage( (HWND)this->m_handle, WM_SETFONT, (WPARAM)p_font->m_handle, MAKELPARAM(TRUE, 0) );
	return true;
}

size_t lwBaseControl::getText( lwStringType& p_text )
{
    if ( this->m_handle == nullptr ) return (false);

    LPTSTR buffer = new TCHAR [GetWindowTextLength( (HWND)this->m_handle )];

    GetWindowText((HWND) this->m_handle, buffer, GetWindowTextLength( (HWND)this->m_handle ) );

    p_text = buffer;

    delete [] buffer;

    return GetWindowTextLength( (HWND)this->m_handle );
}

void lwBaseControl::getArea( int32_t* x, int32_t* y, int32_t* w, int32_t* h )
{
	RECT rc;
	GetWindowRect( (HWND)this->m_handle, &rc );

	if ( x != nullptr ) *x = rc.left;
	if ( y != nullptr ) *y = rc.top;
	if ( w != nullptr ) *w = rc.right - rc.left;
	if ( h != nullptr ) *h = rc.bottom - rc.top;
}

void lwBaseControl::getClientArea( int32_t* x, int32_t* y, int32_t* w, int32_t* h )
{
	RECT rc;
	GetClientRect( (HWND)this->m_handle, &rc );

	if ( x != nullptr ) *x = rc.left;
	if ( y != nullptr ) *y = rc.top;
	if ( w != nullptr ) *w = rc.right - rc.left;
	if ( h != nullptr ) *h = rc.bottom - rc.top;
}

bool lwBaseControl::show( void )
{
    if ( this->m_handle == nullptr ) return (false);

    ShowWindow((HWND) this->m_handle, SW_SHOW );

    return (true);
}

bool lwBaseControl::hide( void )
{
    if ( this->m_handle == nullptr ) return (false);

    ShowWindow((HWND) this->m_handle, SW_HIDE );

    return (true);
}

bool lwBaseControl::enable()
{
    if ( this->m_handle == nullptr ) return (false);

    EnableWindow((HWND) this->m_handle, TRUE );

    return (true);
}

bool lwBaseControl::disable()
{
    if ( this->m_handle == nullptr ) return (false);

    EnableWindow((HWND) this->m_handle, FALSE );

    return (true);
}

void lwBaseControl::resize( uint32_t p_width, uint32_t p_height )
{
    if ( this->m_handle == nullptr ) return;
    SetWindowPos((HWND) this->m_handle, nullptr, 0,0, p_width, p_height, SWP_NOMOVE | SWP_NOZORDER );
}

void lwBaseControl::move( uint32_t p_x, uint32_t p_y )
{
    if ( this->m_handle == nullptr ) return;
    SetWindowPos((HWND) this->m_handle, nullptr, p_x,p_y,0,0, SWP_NOSIZE | SWP_NOZORDER );
}

void lwBaseControl::redraw()
{
    if ( this->m_handle == nullptr ) return;

    RECT rc;
    GetWindowRect( (HWND)this->m_handle, &rc );
    SetWindowPos( (HWND)this->m_handle, nullptr, 0,0, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOZORDER );

    RedrawWindow( (HWND)this->m_handle, nullptr, nullptr, RDW_UPDATENOW );
}

bool lwBaseControl::focus()
{
	SetFocus((HWND) this->m_handle );
	return (GetLastError()==0) ? true : false;
}

void lwBaseControl::center()
{
	RECT parent_rc, rc;
	GetClientRect( GetParent( (HWND)this->m_handle ), &parent_rc );
	GetWindowRect( (HWND)this->m_handle, &rc );

	rc.left = ( parent_rc.right / 2 ) - ( rc.right/2 );
	rc.top = ( parent_rc.bottom / 2 ) - ( rc.bottom/2 );
	printf( "center() :: %d, %d %d %d\n", parent_rc.right, parent_rc.bottom, rc.left, rc.top );

	SetWindowPos( (HWND)this->m_handle, 0, rc.left, rc.top, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );
}

void lwBaseControl::maximise()
{
	if ( this->m_type == LW_FRAME )
	{
		ShowWindow( (HWND)this->m_handle, SW_MAXIMIZE );
	}
	else
	{
		// -- manual resize here, also needs to account for vertical/horizontal/grid containers
		RECT prc;
		GetClientRect( GetParent( (HWND)this->m_handle ), &prc );
		SetWindowPos( (HWND)this->m_handle, nullptr, prc.left, prc.top, prc.right, prc.bottom, SWP_NOZORDER );
	}
}

/************************************************************************
** lwToolTip method implementations
*/
// TODO: clean up and optimise create() method

lwToolTip::lwToolTip()
{
    m_handle = nullptr;
    control = nullptr;
}
lwToolTip::~lwToolTip()
{
    this->destroy();
}

bool lwToolTip::create( lwBaseControl* p_parent, lwBaseControl* p_target_control, const char* p_tip_text)
{
	if ( p_parent == nullptr || p_target_control == nullptr || p_tip_text == nullptr )
		return false;

	this->m_handle = CreateWindowEx(0,
									TOOLTIPS_CLASS,
									nullptr,
									WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | WS_VISIBLE,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									this);

	if ( this->m_handle == nullptr )
		return false;

	SetWindowPos( (HWND)this->m_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    TOOLINFO ti;
    memset(&ti, 0, sizeof(TOOLINFO));
    ti.cbSize		= sizeof(TOOLINFO);
    ti.hwnd			= (HWND)p_parent->m_handle;
    ti.uFlags		= TTF_IDISHWND | TTF_SUBCLASS | TTF_PARSELINKS;
    ti.uId			= (UINT_PTR) p_target_control->m_handle;
    ti.lpszText		= (char*)p_tip_text;
    ti.lParam		= (LPARAM)this;
    SendMessage( (HWND)this->m_handle, TTM_ADDTOOL, 0, (LPARAM) &ti);

    SetWindowLongPtr( (HWND)this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( (HWND)this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

    return true;
}


/************************************************************************
** lwDialog method implementations
*/

lwFrame::lwFrame() : lwBaseControl()
{
	this->m_type = lwWidgetTypeEnum::LW_FRAME;
}

lwFrame::~lwFrame()
{
}

bool lwFrame::create( lwBaseControl* p_parent, const char* p_title )
{
	if ( p_parent == nullptr ) p_parent = &g_desktop_window;

	uint32_t ex_style = 0;
	if ( this->acceptFiles ) ex_style |= WS_EX_ACCEPTFILES;
	if ( this->toolWindow  ) ex_style |= WS_EX_TOOLWINDOW;
	if ( this->dialogFrame ) ex_style |= WS_EX_DLGMODALFRAME;

	uint32_t style = WS_POPUP | WS_VISIBLE;
	if ( this->border      ) style |= WS_BORDER;
	if ( this->titleBar    ) style |= WS_CAPTION;
	if ( this->sizeable    ) style |= WS_SIZEBOX;
	if ( this->closeButton ) style |= WS_SYSMENU;
	if ( this->maximizeButton ) style |= WS_MAXIMIZEBOX;
	if ( this->minimizeButton ) style |= WS_MINIMIZEBOX;

	this->m_handle = CreateWindowEx(ex_style,
									"LW_WINDOWFRAME",
									p_title,
									style,
									CW_USEDEFAULT, CW_USEDEFAULT,
									0, 0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle(nullptr),
									this );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );

	return true;
}

void lwFrame::getClientArea( int32_t* x, int32_t* y, int32_t* w, int32_t* h )
{
	RECT rc;
	GetClientRect( (HWND)this->m_handle, &rc );

	RECT sb_rc, tb_rc;
	memset( &sb_rc, 0, sizeof(RECT) );
	memset( &tb_rc, 0, sizeof(RECT) );

	if ( this->m_status_bar != nullptr )
	{
		GetWindowRect( (HWND)this->m_status_bar->m_handle, &sb_rc );
	}
	if ( this->m_tool_bar != nullptr )
	{
		SendMessage( (HWND)this->m_tool_bar->m_handle, TB_AUTOSIZE, 0, 0);
		GetWindowRect( (HWND)this->m_tool_bar->m_handle, &tb_rc );
	}

	if ( x != nullptr ) *x = rc.left;
	if ( y != nullptr ) *y = rc.top + ( tb_rc.bottom - tb_rc.top );
	if ( w != nullptr ) *w = rc.right - rc.left;
	if ( h != nullptr ) *h = (rc.bottom - rc.top) - (sb_rc.bottom - sb_rc.top) - ( tb_rc.bottom - tb_rc.top );
}

void lwFrame::center()
{
	lwBaseControl::center();
	/*RECT parent_rc, rc;

	parent_rc.left = 0;
	parent_rc.top = 0;
	parent_rc.right = GetSystemMetrics( SM_CXSCREEN );
	parent_rc.bottom = GetSystemMetrics( SM_CYSCREEN );

	GetWindowRect( (HWND)this->m_handle, &rc );

	rc.left = ( parent_rc.right / 2 ) - ( rc.right/2 );
	rc.top = ( parent_rc.bottom / 2 ) - ( rc.bottom/2 );

	SetWindowPos( (HWND)this->m_handle, 0, rc.left, rc.top, rc.right, rc.bottom, SWP_NOSIZE | SWP_NOZORDER );*/
}

void lwFrame::close()
{
	SendMessage((HWND) this->m_handle, WM_CLOSE, 0, 0 );
}

void lwFrame::quit()
{
	PostMessage((HWND) this->m_handle, WM_QUIT, 0, 0 );
}


bool lwFrame::run()
{
	MSG msg;

	if ( PeekMessage( &msg,(HWND) this->m_handle, 0, 0, PM_REMOVE ) )
	{
		if ( msg.message == WM_QUIT )
			return false;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}


/************************************************************************
** lwDialog method implementations
*/

lwDialog::lwDialog()
{
	this->m_type = lwWidgetTypeEnum::LW_DIALOG;
}

lwDialog::~lwDialog()
{
	this->destroy();
}

bool lwDialog::create( lwBaseControl* p_parent, lwStringType p_title )
{
	if ( p_parent == nullptr )
		return false;

	uint32_t style = WS_CHILDWINDOW | WS_POPUP | WS_VISIBLE | WS_BORDER | WS_SYSMENU | WS_CAPTION;
	if ( this->border      ) style |= WS_BORDER;
	if ( this->titleBar    ) style |= WS_CAPTION;
	if ( this->sizeable    ) style |= WS_SIZEBOX;
	if ( this->closeButton ) style |= WS_SYSMENU;
	if ( this->maximizeButton ) style |= WS_MAXIMIZEBOX;
	if ( this->minimizeButton ) style |= WS_MINIMIZEBOX;

	this->m_handle = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
									"LW_WINDOWDIALOG",
									p_title.c_str(),
									style,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle(nullptr),
									this );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );

	return true;
}


/************************************************************************
** lwFileDialog method implementations
*/

lwFileDialog::lwFileDialog( )
{
	this->m_default_extension = "*.*";
	this->m_extension_filter = "All Files (*.*)\0*.*\0";
	this->m_initial_directory = "";
	this->m_path_filename = "";
	this->m_title = "";
}

void lwFileDialog::setInitialDirectory( lwStringType p_initial_directory )
{
	this->m_initial_directory = p_initial_directory;
}

void lwFileDialog::setDefaultExt( lwStringType p_default_extension )
{
	this->m_default_extension = p_default_extension;
}

void lwFileDialog::setFilters( lwStringType p_filters )
{
	this->m_extension_filter = p_filters;
}

void lwFileDialog::setTitle( lwStringType p_title )
{
	this->m_title = p_title;
}

lwStringType lwFileDialog::open( lwBaseControl* p_window )
{
	char			out[MAX_PATH];
	OPENFILENAME	ofn;

	ZeroMemory( out, MAX_PATH );

	ZeroMemory( &ofn, sizeof(OPENFILENAME) );
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)p_window->m_handle;
	ofn.lpstrFile = out;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = this->m_extension_filter.c_str();
	ofn.nFilterIndex = 1;
	ofn.lpstrTitle = this->m_title.c_str();
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;

	if ( this->explorer ) ofn.Flags |= OFN_EXPLORER;
	if ( this->allowSizing ) ofn.Flags |= OFN_ENABLESIZING;
	if ( this->pathMustExist ) ofn.Flags |= OFN_PATHMUSTEXIST;
	if ( this->fileMustExist ) ofn.Flags |= OFN_FILEMUSTEXIST;
	if ( this->hideReadOnly ) ofn.Flags |= OFN_HIDEREADONLY;
	if ( this->readOnly ) ofn.Flags |= OFN_READONLY;
	if ( this->allowMultiSelect ) ofn.Flags |= OFN_ALLOWMULTISELECT;
	if ( this->createFilePrompt ) ofn.Flags |= OFN_CREATEPROMPT;
	if ( this->overwriteFilePrompt ) ofn.Flags |= OFN_OVERWRITEPROMPT;

	if ( GetOpenFileName( &ofn ) )
	{
		return lwStringType( out );
	}

	return lwStringType( "" );
}

lwStringType lwFileDialog::save( lwBaseControl* p_window )
{
	char			out[MAX_PATH];
	OPENFILENAME	ofn;

	ZeroMemory( out, MAX_PATH );

	ZeroMemory( &ofn, sizeof(OPENFILENAME) );
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)p_window->m_handle;
	ofn.lpstrFile = out;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = "All Files (*.*)\0*.*\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

	if ( GetSaveFileName( &ofn ) )
	{
		return lwStringType( out );
	}

	return lwStringType( "" );
}


/************************************************************************
** lwTabGroup method implementations
*/

lwTabGroup::lwTabGroup()
{
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

	this->m_handle = CreateWindowEx(0,
									WC_TABCONTROL,
									"",
									WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle(nullptr),
									this);

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	if ( this->onCreate != nullptr ) this->onCreate();

	return true;
}

void lwTabGroup::resize( uint32_t p_width, uint32_t p_height )
{
	lwBaseControl::resize( p_width, p_height );

	RECT rc;
	GetWindowRect((HWND) this->m_handle, &rc );
	TabCtrl_AdjustRect((HWND) this->m_handle, FALSE, &rc );	// client
}

bool lwTabGroup::addTab( lwStringType p_name )
{
	TCITEM tci;
	TCHAR* psz = new TCHAR [p_name.length()+1];
	memset( psz, 0, sizeof(TCHAR) * (p_name.length()+1) );

	memcpy( psz, p_name.c_str(), sizeof(TCHAR) * p_name.length() );

	tci.mask = TCIF_TEXT;
	if ( this->image ) tci.mask |= TCIF_IMAGE;

	tci.iImage = -1;
	tci.pszText = psz;

	if ( TabCtrl_InsertItem((HWND) this->m_handle, m_tabs.size(), &tci ) != -1 )
	{
		lwTab tab;
		tab.name = p_name;
		tab.children.clear();
		this->m_tabs.push_back( tab );
		delete [] psz;
		return true;
	}

	delete [] psz;
	return false;
}

bool lwTabGroup::addControlToTab( lwStringType p_name, lwBaseControl* p_control )
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
	RECT rc,rc2;
	GetWindowRect((HWND) this->m_handle, &rc );
	GetWindowRect((HWND) this->m_handle, &rc2 );
	TabCtrl_AdjustRect((HWND) this->m_handle, FALSE, &rc );	// client

	p_left = rc.left - rc2.left;
	p_top = rc.top - rc2.top;
	p_width = rc.right - rc.left;
	p_height = rc.bottom - rc.top;
}


/************************************************************************
** lwComboBox method implementations
*/

lwComboBox::lwComboBox()
{
}

lwComboBox::~lwComboBox()
{
	this->destroy();
}

bool lwComboBox::create( lwBaseControl* p_parent, const char *p_title )
{
	this->m_handle = CreateWindowEx(0,
									WC_COMBOBOX,
									p_title,
									WS_BORDER | WS_CHILD | WS_VISIBLE,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle(nullptr),
									this);

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwComboBox::addItem( const char* p_name )
{
	ComboBox_AddString((HWND) this->m_handle, p_name );
}

void lwComboBox::setSelection( uint16_t p_index )
{
	ComboBox_SetCurSel( (HWND)this->m_handle, p_index );
}

uint16_t lwComboBox::getSelection( void )
{
	return ComboBox_GetCurSel((HWND) this->m_handle );
}

void lwComboBox::clearItems( void )
{
	ComboBox_ResetContent((HWND) this->m_handle );
}


/************************************************************************
** lwListBox method implementations
*/

lwListBox::lwListBox()
{
}

lwListBox::~lwListBox()
{
	this->destroy();
}

bool lwListBox::create( lwBaseControl* p_parent, char *p_title )
{
	this->m_handle = CreateWindowEx(0,
									WC_LISTBOX,
									p_title,
									WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									this );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwListBox::addItem( const char* p_name )
{
	ListBox_AddString((HWND) this->m_handle, p_name );
}

void lwListBox::setSelection( uint16_t p_index )
{
	ListBox_SetCurSel((HWND) this->m_handle, p_index );
}

uint16_t lwListBox::getSelection( void )
{
	return ListBox_GetCurSel((HWND) this->m_handle );
}

void lwListBox::clearItems( void )
{
	ListBox_ResetContent((HWND) this->m_handle );
}


//

lwListView::lwListView()
{
}

lwListView::~lwListView()
{
	this->destroy();
}

bool lwListView::create( lwBaseControl* p_parent, const char* p_title )
{
	this->m_handle = CreateWindowEx(0,
									WC_LISTVIEW,
									p_title,
									WS_BORDER | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle(nullptr),
									this);

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

bool lwListView::setImageList( lwImageList* p_image_list )
{
	SendMessage((HWND) this->m_handle, LVM_SETIMAGELIST, (WPARAM)LVSIL_NORMAL, (LPARAM)p_image_list->image_list );
	return true;
}

uint32_t lwListView::addItemText( const char* p_text, int p_item )
{
	LVITEM lvi;
	memset( &lvi, 0, sizeof(LVITEM));
	lvi.mask = LVIF_TEXT;
	lvi.iItem = p_item;
	lvi.iSubItem = 0;
	lvi.pszText = (char*)p_text;
	lvi.cchTextMax = strlen( p_text ) + 1;

	return SendMessage((HWND) this->m_handle, LVM_INSERTITEM, (WPARAM)0, (LPARAM)&lvi );
}

uint32_t lwListView::addItemTextImage( const char* p_text, int p_image, int p_item )
{
	LVITEM lvi;
	memset( &lvi, 0, sizeof(LVITEM));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE;
	lvi.iItem = p_item;
	lvi.iSubItem = 0;
	lvi.pszText = (char*)p_text;
	lvi.cchTextMax = strlen( p_text ) + 1;
	lvi.iImage = p_image;

	return SendMessage((HWND) this->m_handle, LVM_INSERTITEM, (WPARAM)0, (LPARAM)&lvi );
}


/*********************************************************************
** Light Widget Tree View
*/

typedef struct __treeview_item_data
{
	HTREEITEM                 handle;
	__treeview_item_data      *parent;
	lwStringType              name;
} __treeview_item_data;

typedef struct __treeview_data
{
	//
	lwImageList*						imagelist;
	std::map<lwStringType, __treeview_item_data> items;
} __treeview_data;

typedef map<lwStringType, __treeview_item_data> __treeview_item_map;


lwTreeView::lwTreeView()
{
	this->m_type = lwWidgetTypeEnum::LW_TREEVIEW;
	this->m_data = new __treeview_data;
}

lwTreeView::~lwTreeView()
{
	delete reinterpret_cast<__treeview_data*>(this->m_data);
	this->destroy();
}

bool lwTreeView::create( lwBaseControl* p_parent, const char* p_title )
{
	if ( p_parent == nullptr )
		return false;

	uint32_t ex_style = 0;//WS_EX_CLIENTEDGE;
	uint32_t style = WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT;
	if ( this->border      ) style |= WS_BORDER;
	if ( this->itemButtons ) style |= TVS_HASBUTTONS;
	if ( this->itemEditable) style |= TVS_EDITLABELS;

	this->m_handle = CreateWindowEx(ex_style,
									WC_TREEVIEW,
									p_title,
									WS_CLIPSIBLINGS | WS_CHILD | style,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle(nullptr),
									this);

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

bool lwTreeView::setImageList( lwImageList* p_image_list )
{
	SendMessage( (HWND)this->m_handle, TVM_SETIMAGELIST, (WPARAM)TVSIL_NORMAL, (LPARAM)p_image_list->image_list );
	return true;
}

uint32_t lwTreeView::addItemText( lwStringType p_text, lwStringType p_parent )
{
	return this->addItemTextImage( p_text, -1, p_parent );
}

uint32_t lwTreeView::addItemTextImage( lwStringType p_text, int p_image, lwStringType p_parent )
{
	TVINSERTSTRUCT	tvis;
	HTREEITEM				item;
	HTREEITEM				parent = TVI_ROOT;
	__treeview_data	*data = reinterpret_cast<__treeview_data*>(this->m_data);
	__treeview_item_data	*parent_item_data = nullptr;
	__treeview_item_map::iterator parent_it = data->items.end();
	__treeview_item_data	new_item_data;

	// -- If a parent was specified then find its data
	if ( p_parent != TEXT("") )
	{
		parent_it = data->items.find( p_parent );

		if ( parent_it != data->items.end() )
		{
			parent = parent_it->second.handle;
		}
		else return false;
	}

	// -- Zero the insert struct memory
	memset( &tvis, 0, sizeof(TVINSERTSTRUCT));

	// -- Fill in the structure
  tvis.hParent             = parent;
  tvis.hInsertAfter        = TVI_LAST;
	tvis.item.mask           = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
	tvis.item.pszText        = const_cast<char*>(p_text.c_str());
	tvis.item.cchTextMax     = p_text.length()+1;
	tvis.item.lParam         = this->m_tree_items.size()+1;
	tvis.item.stateMask      = TVIS_STATEIMAGEMASK;
	tvis.item.state          = INDEXTOSTATEIMAGEMASK(0);

	// -- Image values
	if ( p_image == -1 )
	{
		tvis.item.mask           |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvis.item.iImage         = p_image;
		tvis.item.iSelectedImage = tvis.item.iImage;
	}

	item = (HTREEITEM)SendMessage((HWND) this->m_handle, TVM_INSERTITEM, (WPARAM)0, (LPARAM)&tvis );

	new_item_data.handle = item;
	new_item_data.parent = &parent_it->second;
	data->items.insert( pair<lwStringType, __treeview_item_data>(p_text, new_item_data) );
	//this->m_tree_items.push_back( item );

	if ( item != nullptr )
	{
		return 0;
	}

	return this->m_tree_items.size();
}

uint32_t lwTreeView::getBKColor()
{
	uint32_t c = TreeView_GetBkColor( (HWND)this->m_handle );

	if ( c == -1 )
	{
		c = GetSysColor( COLOR_WINDOW );
	}

	return c;
}


//

lwButton::lwButton()
{
}

lwButton::~lwButton()
{
	this->destroy();
}

bool lwButton::create( lwBaseControl* p_parent, const char *p_title )
{
	if ( p_parent == nullptr )
		return false;

	uint32_t ex_style = 0;
	uint32_t style = WS_VISIBLE | BS_TEXT;
	if ( this->border      ) style |= WS_BORDER;

	this->m_handle = CreateWindowEx(ex_style,
									WC_BUTTON,
									p_title,
									WS_CHILD | style,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									this );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return  true;
}

void lwButton::setBitmap( lwImage* p_image )
{
	SendMessage((HWND) this->m_handle, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)p_image->bitmap );
}


//

lwTextInput::lwTextInput()
{

}

lwTextInput::~lwTextInput()
{
	this->destroy();
}

bool lwTextInput::create( lwBaseControl *p_parent )
{
	if ( p_parent == nullptr || p_parent->m_handle == nullptr )
	{
		return false;
	}

	uint32_t ex_style = 0;
	uint32_t style = WS_VISIBLE;
	if ( this->border )			style |= WS_BORDER;
	if ( this->readOnly )		style |= ES_READONLY;
	if ( this->passwordInput )	style |= ES_PASSWORD;
	if ( this->numbersInput )	style |= ES_NUMBER;
	if ( this->alignLeft )		style |= ES_LEFT;
	if ( this->alignCenter )	style |= ES_CENTER;
	if ( this->alignRight )		style |= ES_RIGHT;

	this->m_handle = CreateWindowEx(ex_style,
									"Edit",
									"",
									WS_CHILD | style,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle(nullptr),
									this );

	if ( this->m_handle == nullptr )
	{
		return false;
	}

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}


//

lwTextArea::lwTextArea()
{
}

lwTextArea::~lwTextArea()
{
	this->destroy();
}

bool lwTextArea::create( lwBaseControl* p_parent )
{
	if ( p_parent == nullptr || p_parent->m_handle == nullptr )
	{
		return false;
	}

	uint32_t ex_style = 0;
	uint32_t style = WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
	if ( this->border )			style |= WS_BORDER;
	if ( this->readOnly )		style |= ES_READONLY;
	if ( this->passwordInput )	style |= ES_PASSWORD;
	if ( this->numbersInput )	style |= ES_NUMBER;
	if ( this->alignLeft )		style |= ES_LEFT;
	if ( this->alignCenter )	style |= ES_CENTER;
	if ( this->alignRight )		style |= ES_RIGHT;

	this->m_handle = CreateWindowEx(ex_style,
									WC_EDIT,
									nullptr,
									WS_CHILD | style,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									this );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return  true;
}


//

lwGroup::lwGroup()
{
}

lwGroup::~lwGroup()
{
	this->destroy();
}

bool lwGroup::create( lwBaseControl* p_parent, char *p_title )
{
	if ( p_parent == nullptr || p_parent->m_handle == nullptr )
	{
		return false;
	}

	uint32_t ex_style = 0;
	uint32_t style = WS_VISIBLE | BS_GROUPBOX;
	if ( this->border )			style |= WS_BORDER;

	this->m_handle = CreateWindowEx(ex_style,
									WC_BUTTON,
									p_title,
									WS_CHILD | style,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									nullptr );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}


//

lwCheckBox::lwCheckBox()
{
}

lwCheckBox::~lwCheckBox()
{
	this->destroy();
}

bool lwCheckBox::create( lwBaseControl* p_parent, const char* p_title )
{
	this->m_handle = CreateWindowEx(	0,
									WC_BUTTON,
									p_title,
									WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									NULL,
									GetModuleHandle(NULL),
									NULL );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwCheckBox::setCheck( bool p_check )
{
	if (p_check)
		SendMessage((HWND) this->m_handle, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0 );
	else
		SendMessage((HWND) this->m_handle, BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0 );
}

bool lwCheckBox::getCheck( void )
{
	if ( SendMessage((HWND) this->m_handle, BM_GETCHECK, (WPARAM)0, (LPARAM)0 ) == BST_CHECKED )
		return true;
	else
		return false;
}


//

lwStatic::lwStatic()
{
}

lwStatic::~lwStatic()
{
	this->destroy();
}

bool lwStatic::create( lwBaseControl* p_parent, const char* p_title )
{
	if ( p_parent == nullptr )
		return false;

	uint32_t ex_style = 0;
	uint32_t style = WS_VISIBLE | SS_LEFT | SS_NOPREFIX | SS_EDITCONTROL;
	if ( this->border )			style |= WS_BORDER;
	if ( this->image )			style |= SS_BITMAP;

	this->m_handle = CreateWindowEx(ex_style,
									WC_STATIC,
									p_title,
									WS_CHILD | style,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									nullptr );
	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwStatic::setImage( lwImage* p_image )
{
	SendMessage( (HWND)this->m_handle, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)p_image->bitmap );
}


//

lwSysLink::lwSysLink()
{

}

lwSysLink::~lwSysLink()
{
	this->destroy();
}

bool lwSysLink::create( lwBaseControl* p_parent, const char* p_text )
{
	if ( p_parent == nullptr )
		return false;

	this->m_handle = CreateWindowEx(0,
									"SysLink",
									p_text,
									WS_VISIBLE | WS_CHILD | WS_TABSTOP,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									nullptr );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}


//

lwMenuBar::lwMenuBar()
{
	m_handle = nullptr;
}

lwMenuBar::~lwMenuBar()
{
	m_submenus.clear();
	m_events.clear();
	DestroyMenu( (HMENU)m_handle );
}

bool lwMenuBar::create( lwBaseControl* p_parent )
{
	if ( m_handle != nullptr ) return true;

	m_handle = (HWND)CreateMenu();

	if ( m_handle == nullptr )
		return false;

	if ( p_parent != nullptr )
    {
        SetMenu( (HWND)p_parent->m_handle, (HMENU)m_handle );
    }
    m_parent = p_parent;

    MENUINFO mi;
    ZeroMemory( &mi, sizeof(MENUINFO) );
    mi.cbSize = sizeof(MENUINFO);
    mi.fMask = MIM_STYLE | MIM_MENUDATA | MIM_APPLYTOSUBMENUS;
    mi.dwStyle = MNS_NOTIFYBYPOS;
    mi.dwMenuData = (ULONG_PTR)this;
    SetMenuInfo( (HMENU)m_handle, &mi );

	return true;
}

bool lwMenuBar::addPopupMenu( lwStringType p_menu_name, lwStringType p_name )
{
	HMENU submenu = CreatePopupMenu();
	HMENU parent = (HMENU)this->i_findMenu( p_menu_name );

	if ( !AppendMenu( (HMENU)parent, MF_POPUP | MF_STRING, (UINT_PTR)submenu, p_name.c_str() ) )
		return false;

	m_submenus.push_back( pair<lwStringType,lwNativeWindowType>(p_name,(HWND)submenu) );

	return true;
}

bool lwMenuBar::addItem( lwStringType p_menu_name, lwStringType p_name, lwFuncType p_event )
{
	uint32_t p_id = m_events.size();
	bool b = AppendMenu( (HMENU)i_findMenu( p_menu_name ), MF_STRING, p_id, p_name.c_str() );
	m_events.push_back( p_event );
	return b;
}

bool lwMenuBar::insertItem( lwStringType p_menu_name, uint32_t p_pos, lwStringType p_name, lwFuncType p_event )
{
	uint32_t p_id = m_events.size();
	bool b = InsertMenu( (HMENU)i_findMenu( p_menu_name ), p_pos, MF_STRING, p_id, p_name.c_str() );
	m_events.push_back( p_event );
	return b;
}

void lwMenuBar::removeItem( lwStringType p_menu_name, uint32_t p_pos )
{
	// TODO: Remove event
	RemoveMenu( (HMENU)i_findMenu( p_menu_name ), p_pos, 0 );
}

bool lwMenuBar::addSeparator( lwStringType p_menu_name )
{
	return AppendMenu( (HMENU)i_findMenu( p_menu_name ), MF_SEPARATOR, 0, NULL );
}

bool lwMenuBar::addBreak( lwStringType p_menu_name )
{
	return AppendMenu( (HMENU)i_findMenu( p_menu_name ), MF_MENUBREAK, 0, NULL );
}

void lwMenuBar::update()
{
	if ( m_handle == nullptr ) return;

	MENUINFO mi;
    ZeroMemory( &mi, sizeof(MENUINFO) );
    mi.cbSize = sizeof(MENUINFO);
    mi.fMask = MIM_STYLE | MIM_MENUDATA | MIM_APPLYTOSUBMENUS;
    mi.dwStyle = MNS_NOTIFYBYPOS;
    mi.dwMenuData = (ULONG_PTR)this;
    SetMenuInfo( (HMENU)m_handle, &mi );

	if ( m_parent != nullptr );
    DrawMenuBar( (HWND)m_parent->m_handle );
}

/// Private Method
lwNativeWindowType lwMenuBar::i_findMenu( lwStringType p_name )
{
	if ( p_name == "" )
		return m_handle;

	// Find the sub menu handle
	for ( size_t i=0; i<m_submenus.size(); i++ )
	{
		if ( m_submenus[i].first == p_name )
		{
			return m_submenus[i].second;
			break;
		}
	}

	return m_handle;
}


//

lwStatusBar::lwStatusBar()
{
}

lwStatusBar::~lwStatusBar()
{
	this->destroy();
}

bool lwStatusBar::create( lwFrame* p_parent, uint16_t p_parts, int32_t* p_part_width )
{
	if ( p_parent == nullptr ) return false;

	this->m_handle = CreateWindowEx(0,
									STATUSCLASSNAME,
									nullptr,
									WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									(HMENU)nullptr,
									GetModuleHandle(nullptr),
									this);

	if ( this->m_handle == nullptr )
		return false;

	// -- Give the frame a copy of this as the status bar object it needs to resize
	p_parent->m_status_bar = this;

	SendMessage((HWND) this->m_handle, SB_SETPARTS, (WPARAM)p_parts, (LPARAM)p_part_width );

	// -- Set the userdata to a pointer of this and set the stock font object
	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	// -- This updates the Status Bar so that it resizes to the Window dimensions correctly.
	SendMessage((HWND) this->m_handle, WM_SIZE, 0, 0 );

	return true;
}

void lwStatusBar::setSectionText( uint16_t p_index, lwStringType p_text )
{
	SendMessage( (HWND)this->m_handle, SB_SETTEXT, p_index, (LPARAM)p_text.c_str() );
}

void lwStatusBar::resize( uint32_t p_width, uint32_t p_height )
{
	// -- This updates the Status Bar so that it resizes to the Window dimensions correctly.
	SendMessage((HWND) this->m_handle, WM_SIZE, 0, 0 );
}

uint32_t lwStatusBar::getHeight()
{
	RECT rc;
	GetWindowRect( (HWND)this->m_handle, &rc);
	return rc.bottom - rc.top;
}


//

lwProgressBar::lwProgressBar()
{
}

lwProgressBar::~lwProgressBar()
{
	this->destroy();
}

bool lwProgressBar::create( lwBaseControl* p_parent )
{
	if ( p_parent == nullptr )
		return false;

	uint32_t ex_style = 0;
	uint32_t style = WS_VISIBLE;
	if ( this->border      ) style |= WS_BORDER;

	this->m_handle = CreateWindowEx(ex_style,
									PROGRESS_CLASS,
									nullptr,
									WS_CHILD | style | PBS_SMOOTH,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									this );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwProgressBar::setMarquee( bool p_marquee, uint32_t p_update_ms )
{
	uint32_t styles = GetWindowLong((HWND) this->m_handle, GWL_STYLE );

	if ( p_marquee )
		styles |= PBS_MARQUEE;
	else
		styles &= ~PBS_MARQUEE;

	SetWindowLong( (HWND) this->m_handle, GWL_STYLE, styles );

	SendMessage( (HWND) this->m_handle, PBM_SETMARQUEE, (WPARAM)p_marquee, (LPARAM)p_update_ms );
}

void lwProgressBar::setRange( uint16_t p_min, uint16_t p_max )
{
	SendMessage( (HWND) this->m_handle, PBM_SETRANGE, (WPARAM)0, MAKELPARAM( p_min, p_max ) );
}

void lwProgressBar::setPosition( uint16_t p_index )
{
	SendMessage( (HWND) this->m_handle, PBM_SETPOS, (WPARAM)p_index, (LPARAM)0);
}

void lwProgressBar::setState( uint16_t p_state )
{
	SendMessage( (HWND) this->m_handle, PBM_SETSTATE, (WPARAM)p_state, (LPARAM)0);
}

void lwProgressBar::setStep( uint16_t p_step_size )
{
	SendMessage( (HWND)this->m_handle, PBM_SETSTEP, (WPARAM)p_step_size, (LPARAM)0);
}

void lwProgressBar::step( void )
{
	SendMessage( (HWND)this->m_handle, PBM_STEPIT, (WPARAM)0, (LPARAM)0);
}

void lwProgressBar::setBackgroundColor( uint32_t p_rgba )
{
	SendMessage( (HWND)this->m_handle, PBM_SETBKCOLOR, (WPARAM)0, (LPARAM)p_rgba);
}

void lwProgressBar::setBarColor( uint32_t p_rgba )
{
	SendMessage( (HWND)this->m_handle, PBM_SETBARCOLOR, (WPARAM)0, (LPARAM)p_rgba);
}


//

lwTrackBar::lwTrackBar()
{
}

lwTrackBar::~lwTrackBar()
{
	this->destroy();
}

bool lwTrackBar::create( lwBaseControl* p_parent )
{
	if ( p_parent == nullptr )
		return false;

	uint32_t ex_style = 0;
	uint32_t style = WS_VISIBLE;
	if ( this->border        ) style |= WS_BORDER;
	if ( this->auto_ticks    ) style |= TBS_AUTOTICKS;
	if ( this->vertical      ) style |= TBS_VERT;
	if ( this->ticks_bottom  ) style |= TBS_BOTTOM;
	if ( this->ticks_top     ) style |= TBS_TOP;
	if ( this->ticks_left    ) style |= TBS_LEFT;
	if ( this->ticks_right   ) style |= TBS_RIGHT;
	// TODO: Fill in the lwTrackBar style properties

	this->m_handle = CreateWindowEx(ex_style,
									TRACKBAR_CLASS,
									nullptr,
									WS_CHILD | TBS_HORZ | style,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									this );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( (HWND)this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwTrackBar::setRange( uint16_t p_min, uint16_t p_max )
{
	SendMessage( (HWND)this->m_handle, TBM_SETRANGE, (WPARAM)0, MAKELPARAM( p_min, p_max ) );
}

void lwTrackBar::setPosition( uint16_t p_index )
{
	SendMessage( (HWND)this->m_handle, TBM_SETPOS, (WPARAM)p_index, (LPARAM)0);
}

uint16_t lwTrackBar::getPosition() const
{
	return SendMessage( (HWND)this->m_handle, TBM_GETPOS, 0, 0 );
}


//

lwToolBar::lwToolBar()
{
	this->m_type = LW_TOOL_BAR;
}

lwToolBar::~lwToolBar()
{
	for ( size_t i=0; i<this->m_buttons.size(); i++ )
	{
		delete (TBBUTTON*)this->m_buttons[i];
	}
	this->destroy();
}

bool lwToolBar::create( lwFrame* p_parent )
{
	if ( p_parent == nullptr )
		return false;

	uint32_t ex_style = TBSTYLE_EX_MIXEDBUTTONS;
	uint32_t style = WS_VISIBLE | TBSTYLE_AUTOSIZE | TBSTYLE_LIST | TBSTYLE_TOOLTIPS;
	if ( this->border      ) style |= WS_BORDER;

	this->m_handle = CreateWindowEx(ex_style,
									TOOLBARCLASSNAME,
									nullptr,
									WS_CHILD | style,
									0,0,0,0,
									(HWND)p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									this );
	if ( this->m_handle == nullptr )
		return false;

	// -- Give the frame a copy of this as the tool bar object it needs to resize
	p_parent->m_tool_bar = this;

	// Compatability fix
	SendMessage( (HWND)this->m_handle,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);

	SetWindowLongPtr((HWND) this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage((HWND) this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwToolBar::setImageList( lwImageList* p_image_list )
{
	if ( p_image_list != nullptr )
	{
		SendMessage( (HWND)this->m_handle, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)p_image_list->image_list );
	}
	else
	{
		SendMessage( (HWND)this->m_handle, TB_LOADIMAGES, (WPARAM)IDB_STD_SMALL_COLOR, (LPARAM)HINST_COMMCTRL );
	}
}

uint32_t lwToolBar::addButton( std::string p_text )
{
	TBBUTTON* tbb = new TBBUTTON;
	ZeroMemory( tbb, sizeof(TBBUTTON) );
	tbb->iBitmap = I_IMAGENONE;
	tbb->iString = (int)p_text.c_str();
	tbb->fsState = TBSTATE_ENABLED;
	tbb->fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON | BTNS_SHOWTEXT;
	tbb->idCommand = this->m_buttons.size();

	this->m_buttons.push_back( tbb );

	SendMessage( (HWND)this->m_handle, TB_ADDBUTTONS, 1, (LPARAM)tbb );

	return this->m_buttons.size()-1;
}

uint32_t lwToolBar::addButtonImage( uint32_t p_bitmap, string p_text )
{
	TBBUTTON* tbb = new TBBUTTON;
	ZeroMemory( tbb, sizeof(TBBUTTON) );
	tbb->iBitmap = p_bitmap;
	tbb->iString = (int)p_text.c_str();
	tbb->fsState = TBSTATE_ENABLED;
	tbb->fsStyle = BTNS_AUTOSIZE | BTNS_BUTTON;
	tbb->idCommand = this->m_buttons.size();

	if ( p_text == "" )
	{
		tbb->iString = 0;
		tbb->fsStyle |= BTNS_SHOWTEXT;
	}

	this->m_buttons.push_back( tbb );

	SendMessage( (HWND)this->m_handle, TB_ADDBUTTONS, 1, (LPARAM)tbb );

	return this->m_buttons.size()-1;
}

uint32_t lwToolBar::addSeparator( )
{
	TBBUTTON *tbb = new TBBUTTON;
	ZeroMemory( tbb, sizeof(TBBUTTON) );
	tbb->iBitmap = 0;
	tbb->fsState = TBSTATE_ENABLED;
	tbb->fsStyle = BTNS_SEP;
	tbb->idCommand = 0;

	this->m_buttons.push_back( tbb );

	SendMessage( (HWND)this->m_handle, TB_ADDBUTTONS, 1, (LPARAM)tbb );

	return this->m_buttons.size()-1;
}

void lwToolBar::removeButton( uint32_t p_index )
{
	SendMessage( (HWND)this->m_handle, TB_ADDBUTTONS, (WPARAM)p_index, (LPARAM)0 );
	delete this->m_buttons[p_index];
	this->m_buttons.erase( this->m_buttons.begin() + p_index );
}

void lwToolBar::resize( uint32_t p_width, uint32_t p_height )
{
	SendMessage((HWND)this->m_handle, TB_AUTOSIZE, 0, 0 );
}
