/*************************************************************
** Win32 & COM Implementation
** Light Widget Library: Core
**
** Most commonly used controls for GUI's
**/

#undef WINVER
#undef _WIN32_IE
#undef _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_IE       0x0800      // IE 8
#define _WIN32_WINNT    0x0601      // Windows 7
#define WINVER          0x0601      // Windows 7

#define PBS_MARQUEE  0x08
#define PBM_SETMARQUEE WM_USER + 10
#define PBM_SETSTATE WM_USER + 16

#include <Windows.h>
#include <CommCtrl.h>
#include <Windowsx.h>

#include <cstdio>
#include <string>

#include "lw-main.h"
#include "lw-resources.h"


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
			// -- resize the status bar
			if ( widget->m_type == lwControlTypeEnum::LW_FRAME &&
				 widget &&
				 ((lwFrame*)(widget))->m_status_bar != nullptr )
				((lwFrame*)(widget))->m_status_bar->resize( 0, 0 );
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

	case WM_NOTIFY:
		{
			NMHDR* nh = (NMHDR*)lparam;
			switch ( nh->code )
			{
			case TCN_SELCHANGE:
				{
					lwTabGroup *tb = (lwTabGroup*)GetWindowLongPtr( (HWND)nh->hwndFrom, GWLP_USERDATA );
					int32_t selected_tab = TabCtrl_GetCurSel( tb->m_handle );
					for ( int32_t i=0; i<tb->m_tabs.size(); i++)
					{
						for ( int32_t c=0; c<tb->m_tabs[i].children.size(); c++ )
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
lwApplication::lwApplication()
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
        wc.style				= CS_HREDRAW | CS_VREDRAW;//CS_OWNDC | CS_DBLCLKS;

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
        wc.style				= CS_HREDRAW | CS_VREDRAW;//CS_OWNDC | CS_DBLCLKS;

        if ( !RegisterClassEx( &g_window_dialog_class ) )
        {
            printf( "lwError: Failed to register the LW_WINDOWDIALOG class! Error: %u\n", GetLastError() );
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

size_t lwApplication::getWorkingDirectory( char* p_directory, size_t p_max_length )
{
	GetModuleFileName( nullptr, p_directory, p_max_length );
	return true;
}

uint32_t lwApplication::getScreenWidth( void )
{
	return GetSystemMetrics( SM_CXSCREEN );
}

uint32_t lwApplication::getScreenHeight( void )
{
	return GetSystemMetrics( SM_CYSCREEN );
}

bool lwApplication::messageLoop()
{
	MSG msg;

	if ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
	{
		if ( msg.message == WM_QUIT )
			return false;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}


/************************************************************************
** lwBaseControl method & operator implementations
*/

lwBaseControl::lwBaseControl()
{
	this->m_handle = nullptr;

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

lwBaseControl::operator HWND(void)
{
    return this->m_handle;
}

// -- Unneeded?
void lwBaseControl::operator = (const HWND &other)
{
    this->m_handle = other;
}

void lwBaseControl::destroy( void )
{
    if ( this->m_handle != nullptr )
    {
        DestroyWindow( this->m_handle );
        this->m_handle = nullptr;
    }
}

bool lwBaseControl::setText( const char* p_text )
{
    if ( this->m_handle == nullptr ) return (false);

    SetWindowText( this->m_handle, p_text );

    return (true);
}

uint32_t lwBaseControl::getText( char* p_text )
{
    if ( this->m_handle == nullptr ) return (false);

    if ( p_text == nullptr )
    {
        return GetWindowTextLength( this->m_handle );
    }

    GetWindowText( this->m_handle, p_text, GetWindowTextLength( this->m_handle ) );

    return (true);
}

bool lwBaseControl::show( void )
{
    if ( this->m_handle == nullptr ) return (false);

    ShowWindow( this->m_handle, SW_SHOW );

    return (true);
}

bool lwBaseControl::hide( void )
{
    if ( this->m_handle == nullptr ) return (false);

    ShowWindow( this->m_handle, SW_HIDE );

    return (true);
}

bool lwBaseControl::enable()
{
    if ( this->m_handle == nullptr ) return (false);

    EnableWindow( this->m_handle, TRUE );

    return (true);
}

bool lwBaseControl::disable()
{
    if ( this->m_handle == nullptr ) return (false);

    EnableWindow( this->m_handle, FALSE );

    return (true);
}

void lwBaseControl::resize( uint32_t p_width, uint32_t p_height )
{
    if ( this->m_handle == nullptr ) return;
    SetWindowPos( this->m_handle, nullptr, 0,0, p_width, p_height, SWP_NOMOVE | SWP_NOZORDER );
}

void lwBaseControl::move( uint32_t p_x, uint32_t p_y )
{
    if ( this->m_handle == nullptr ) return;
    SetWindowPos( this->m_handle, nullptr, p_x,p_y,0,0, SWP_NOSIZE | SWP_NOZORDER );
}

void lwBaseControl::redraw()
{
    if ( this->m_handle == nullptr ) return;

    RedrawWindow( this->m_handle, nullptr, nullptr, RDW_UPDATENOW );
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
									p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									this);

	if ( this->m_handle == nullptr )
		return false;

	SetWindowPos(this->m_handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    TOOLINFO ti;
    memset(&ti, 0, sizeof(TOOLINFO));
    ti.cbSize = sizeof(TOOLINFO);
    ti.hwnd = p_parent->m_handle;
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS | TTF_PARSELINKS;
    ti.uId = (UINT_PTR) p_target_control->m_handle;
    ti.lpszText = (char*)p_tip_text;
    ti.lParam = (LPARAM)this;
    SendMessage(this->m_handle, TTM_ADDTOOL, 0, (LPARAM) &ti);

    SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

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
									0,0,0,0,
									p_parent->m_handle,
									nullptr,
									GetModuleHandle(nullptr),
									this );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );

	return true;
}

void lwFrame::close()
{
	SendMessage( this->m_handle, WM_CLOSE, 0, 0 );
}

bool lwFrame::run()
{
	MSG msg;

	if ( PeekMessage( &msg, this->m_handle, 0, 0, PM_REMOVE ) )
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
	this->m_type = lwControlTypeEnum::LW_DIALOG;
}

lwDialog::~lwDialog()
{
	this->destroy();
}

bool lwDialog::create( lwBaseControl* p_parent, const char *p_title )
{
	if ( p_parent == nullptr )
		return false;

	uint32_t style = WS_POPUP | WS_VISIBLE | WS_BORDER | WS_SYSMENU | WS_CAPTION;
	if ( this->border      ) style |= WS_BORDER;
	if ( this->titleBar    ) style |= WS_CAPTION;
	if ( this->sizeable    ) style |= WS_SIZEBOX;
	if ( this->closeButton ) style |= WS_SYSMENU;
	if ( this->maximizeButton ) style |= WS_MAXIMIZEBOX;
	if ( this->minimizeButton ) style |= WS_MINIMIZEBOX;

	this->m_handle = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
									"LW_WINDOWDIALOG",
									p_title,
									style,
									0,0,0,0,
									p_parent->m_handle,
									nullptr,
									GetModuleHandle(nullptr),
									this );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );

	return true;
}

bool lwDialog::run()
{
    /*
    ** Run the dialog until WM_QUIT is encountered
    */
	MSG msg;
    int v = 0;

	while ( (v = GetMessage( &msg, this->m_handle, 0, 0 )) != 0 )
	{
	    if ( v == -1 )
        {
            // error!
            DWORD err = GetLastError();
            return false;
        }

		if ( msg.message == WM_QUIT )
			return true;

		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

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
	char			out[MAX_PATH];
	OPENFILENAME	ofn;

	ZeroMemory( out, MAX_PATH );

	ZeroMemory( &ofn, sizeof(OPENFILENAME) );
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = p_window->m_handle;
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
		return tstring( out );
	}

	return tstring( "" );
}

tstring lwFileDialog::save( lwBaseControl* p_window )
{
	char			out[MAX_PATH];
	OPENFILENAME	ofn;

	ZeroMemory( out, MAX_PATH );

	ZeroMemory( &ofn, sizeof(OPENFILENAME) );
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = p_window->m_handle;
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
		return tstring( out );
	}

	return tstring( "" );
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
									p_parent->m_handle,
									nullptr,
									GetModuleHandle(nullptr),
									this);

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwTabGroup::resize( uint32_t p_width, uint32_t p_height )
{
	RECT rc;
	GetWindowRect( this->m_handle, &rc );
	rc.right = p_width;
	rc.bottom = p_height;
	TabCtrl_AdjustRect( this->m_handle, FALSE, &rc );
	lwBaseControl::resize( p_width, p_height );
}

bool lwTabGroup::addTab( tstring p_name )
{
	TCITEM tci;
	TCHAR* psz = new TCHAR [p_name.length()+1];
	memset( psz, 0, sizeof(TCHAR) * (p_name.length()+1) );

	memcpy( psz, p_name.c_str(), sizeof(TCHAR) * p_name.length() );

	tci.mask = TCIF_TEXT;
	if ( this->image ) tci.mask |= TCIF_IMAGE;

	tci.iImage = -1;
	tci.pszText = psz;

	if ( TabCtrl_InsertItem( this->m_handle, m_tabs.size(), &tci ) != -1 )
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
									p_parent->m_handle,
									nullptr,
									GetModuleHandle(nullptr),
									this);

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwComboBox::addItem( const char* p_name )
{
	ComboBox_AddString( this->m_handle, p_name );
}

void lwComboBox::setSelelection( uint16_t p_index )
{
	ComboBox_SetCurSel(this->m_handle, p_index );
}

uint16_t lwComboBox::getSelection( void )
{
	return ComboBox_GetCurSel( this->m_handle );
}

void lwComboBox::clearItems( void )
{
	ComboBox_ResetContent( this->m_handle );
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
									p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									this );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwListBox::addItem( const char* p_name )
{
	ListBox_AddString( this->m_handle, p_name );
}

void lwListBox::setSelection( uint16_t p_index )
{
	ListBox_SetCurSel( this->m_handle, p_index );
}

uint16_t lwListBox::getSelection( void )
{
	return ListBox_GetCurSel( this->m_handle );
}

void lwListBox::clearItems( void )
{
	ListBox_ResetContent( this->m_handle );
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
									p_parent->m_handle,
									nullptr,
									GetModuleHandle(nullptr),
									this);

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

bool lwListView::setImageList( lwImageList* p_image_list )
{
	SendMessage( this->m_handle, LVM_SETIMAGELIST, (WPARAM)LVSIL_NORMAL, (LPARAM)p_image_list->image_list );
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

	return SendMessage( this->m_handle, LVM_INSERTITEM, (WPARAM)0, (LPARAM)&lvi );
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

	return SendMessage( this->m_handle, LVM_INSERTITEM, (WPARAM)0, (LPARAM)&lvi );
}


//

lwTreeView::lwTreeView()
{
}

lwTreeView::~lwTreeView()
{
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
									p_parent->m_handle,
									nullptr,
									GetModuleHandle(nullptr),
									this);

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

bool lwTreeView::setImageList( lwImageList* p_image_list )
{
	SendMessage( this->m_handle, TVM_SETIMAGELIST, (WPARAM)1, (LPARAM)p_image_list->image_list );
	return true;
}

uint32_t lwTreeView::addItemText( string p_text, int p_parent )
{
	TVINSERTSTRUCT	tvis;
	HTREEITEM		hti;
	HTREEITEM		hti_parent = TVI_ROOT;

	if ( p_parent > 0 && p_parent <= this->m_tree_items.size() )
		hti_parent = this->m_tree_items[p_parent-1];

	// -- Zero the insert struct memory
	memset( &tvis, 0, sizeof(TVINSERTSTRUCT));

	// -- Allocate, temporarily, a read/write c-style string
	char* psz = new char [p_text.length()+1];
	strcpy( psz, p_text.c_str() );

	//printf( "lwTreeView::create() - parent:%p/%d size:%u\n", hti_parent, p_parent, this->m_tree_items.size() );

	// -- Fill in the structure
    tvis.hParent				= hti_parent;
    tvis.hInsertAfter			= TVI_LAST;
	tvis.item.mask				= TVIF_TEXT | TVIF_PARAM;
	tvis.item.iImage			= 0;
	tvis.item.iSelectedImage	= 0;
	tvis.item.pszText			= psz;
	tvis.item.cchTextMax		= p_text.length()+1;
	tvis.item.lParam			= this->m_tree_items.size()+1;
	//tvis.item.stateMask		= TVIS_

	hti = (HTREEITEM)SendMessage( this->m_handle, TVM_INSERTITEM, (WPARAM)0, (LPARAM)&tvis );
	delete psz;

	this->m_tree_items.push_back( hti );

	if ( hti != nullptr )
		return 0;

	return this->m_tree_items.size();
}

uint32_t lwTreeView::addItemTextImage( string p_text, int p_image, int p_parent )
{

	return 0;
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
									p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									this );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return  true;
}

void lwButton::setBitmap( lwImage* p_image )
{
	SendMessage( this->m_handle, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)p_image->bitmap );
}


//

lwGroup::lwGroup()
{
}

lwGroup::~lwGroup()
{
	this->destroy();
}

void lwGroup::create( lwBaseControl* p_parent, char *p_title )
{
	this->m_handle = CreateWindowEx(0,
									WC_BUTTON,
									p_title,
									WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
									0,0,0,0,
									p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									nullptr );

	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
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
									p_parent->m_handle,
									NULL,
									GetModuleHandle(NULL),
									NULL );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwCheckBox::setCheck( bool p_check )
{
	if (p_check)
		SendMessage( this->m_handle, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0 );
	else
		SendMessage( this->m_handle, BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0 );
}

bool lwCheckBox::getCheck( void )
{
	if ( SendMessage( this->m_handle, BM_GETCHECK, (WPARAM)0, (LPARAM)0 ) == BST_CHECKED )
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
	uint32_t style = WS_VISIBLE;
	if ( this->border )			style |= WS_BORDER;
	if ( this->image )			style |= SS_BITMAP;

	this->m_handle = CreateWindowEx(ex_style,
									WC_STATIC,
									p_title,
									WS_CHILD | SS_SIMPLE | style,
									0,0,0,0,
									p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									nullptr );
	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwStatic::setImage( lwImage* p_image )
{
	SendMessage(this->m_handle, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)p_image->bitmap );
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
	DestroyMenu( m_handle );
}

bool lwMenuBar::create( lwBaseControl* p_parent )
{
	if ( m_handle != nullptr ) return true;

	m_handle = CreateMenu();

	if ( m_handle == nullptr )
		return false;

	if ( p_parent != nullptr )
    {
        SetMenu( p_parent->m_handle, m_handle );
    }
    m_parent = p_parent;

    MENUINFO mi;
    ZeroMemory( &mi, sizeof(MENUINFO) );
    mi.cbSize = sizeof(MENUINFO);
    mi.fMask = MIM_STYLE | MIM_MENUDATA | MIM_APPLYTOSUBMENUS;
    mi.dwStyle = MNS_NOTIFYBYPOS;
    mi.dwMenuData = (ULONG_PTR)this;
    SetMenuInfo( m_handle, &mi );

	return true;
}

bool lwMenuBar::addPopupMenu( const char* p_menu_name, const char* p_name )
{
	HMENU submenu = CreatePopupMenu();
	HMENU parent = this->i_findMenu( p_menu_name );

	if ( !AppendMenu( parent, MF_POPUP | MF_STRING, (UINT_PTR)submenu, p_name ) )
		return false;

	m_submenus.push_back( pair<string,HMENU>(p_name,submenu) );

	return true;
}

bool lwMenuBar::addItem( const char* p_menu_name, char *p_name, lwFuncType p_event )
{
	uint32_t p_id = m_events.size();
	bool b = AppendMenu( i_findMenu( p_menu_name ), MF_STRING, p_id, p_name );
	m_events.push_back( p_event );
	return b;
}

bool lwMenuBar::insertItem( const char* p_menu_name, uint32_t p_pos, char *p_name, lwFuncType p_event )
{
	uint32_t p_id = m_events.size();
	bool b = InsertMenu( i_findMenu( p_menu_name ), p_pos, MF_STRING, p_id, p_name );
	m_events.push_back( p_event );
	return b;
}

void lwMenuBar::removeItem( const char* p_menu_name, uint32_t p_pos )
{
	RemoveMenu( i_findMenu( p_menu_name ), p_pos, 0 );
}

bool lwMenuBar::addSeparator( const char* p_menu_name )
{
	return AppendMenu( i_findMenu( p_menu_name ), MF_SEPARATOR, 0, NULL );
}

bool lwMenuBar::addBreak( const char* p_menu_name )
{
	return AppendMenu( i_findMenu( p_menu_name ), MF_MENUBREAK, 0, NULL );
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
    SetMenuInfo( m_handle, &mi );

	if ( m_parent != nullptr );
    DrawMenuBar( m_parent->m_handle );
}

/// Private Method
HMENU lwMenuBar::i_findMenu( const char* p_name )
{
	if ( p_name == nullptr )
		return m_handle;

	// Find the sub menu handle
	for ( size_t i=0; i<m_submenus.size(); i++ )
	{
		if ( strcmp( (char*)m_submenus[i].first.c_str(), p_name ) == 0 )
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

bool lwStatusBar::create( lwFrame* p_parent, uint16_t p_parts, uint32_t* p_part_width )
{
	if ( p_parent == nullptr ) return false;

	this->m_handle = CreateWindowEx(0,
									STATUSCLASSNAME,
									nullptr,
									WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
									0,0,0,0,
									p_parent->m_handle,
									(HMENU)nullptr,
									GetModuleHandle(nullptr),
									this);

	if ( this->m_handle == nullptr )
		return false;

	// -- Give the frame a copy of this as the status bar object it needs to resize
	p_parent->m_status_bar = this;

	SendMessage( this->m_handle, SB_SETPARTS, (WPARAM)p_parts, (LPARAM)p_part_width );

	// -- Set the userdata to a pointer of this and set the stock font object
	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	// -- This updates the Status Bar so that it resizes to the Window dimensions correctly.
	SendMessage( this->m_handle, WM_SIZE, 0, 0 );

	return true;
}

void lwStatusBar::setSectionText( uint16_t p_index, char *p_text )
{
	SendMessage(this->m_handle, SB_SETTEXT, p_index, (LPARAM)p_text);
}

void lwStatusBar::resize( uint32_t p_width, uint32_t p_height )
{
	// -- This updates the Status Bar so that it resizes to the Window dimensions correctly.
	SendMessage( this->m_handle, WM_SIZE, 0, 0 );
}

uint32_t lwStatusBar::getHeight()
{
	RECT rc;
	GetWindowRect(this->m_handle, &rc);
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

	this->m_handle = CreateWindowEx(0,
									PROGRESS_CLASS,
									nullptr,
									WS_CHILD | style | PBS_SMOOTH,
									0,0,0,0,
									p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									this );

	if ( this->m_handle == nullptr )
		return false;

	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwProgressBar::setMarquee( bool p_marquee, uint32_t p_update_ms )
{
	uint32_t styles = GetWindowLong( this->m_handle, GWL_STYLE );

	if ( p_marquee )
		styles |= PBS_MARQUEE;
	else
		styles &= ~PBS_MARQUEE;

	SetWindowLong( this->m_handle, GWL_STYLE, styles );

	SendMessage( this->m_handle, PBM_SETMARQUEE, (WPARAM)p_marquee, (LPARAM)p_update_ms );
}

void lwProgressBar::setRange( uint16_t p_min, uint16_t p_max )
{
	SendMessage( this->m_handle, PBM_SETRANGE, (WPARAM)0, MAKELPARAM( p_min, p_max ) );
}

void lwProgressBar::setPosition( uint16_t p_index )
{
	SendMessage( this->m_handle, PBM_SETPOS, (WPARAM)p_index, (LPARAM)0);
}

void lwProgressBar::setState( uint16_t p_state )
{
	SendMessage( this->m_handle, PBM_SETSTATE, (WPARAM)p_state, (LPARAM)0);
}

void lwProgressBar::setStep( uint16_t p_step_size )
{
	SendMessage(this->m_handle, PBM_SETSTEP, (WPARAM)p_step_size, (LPARAM)0);
}

void lwProgressBar::step( void )
{
	SendMessage(this->m_handle, PBM_STEPIT, (WPARAM)0, (LPARAM)0);
}

void lwProgressBar::setBackgroundColor( uint32_t p_rgba )
{
	SendMessage(this->m_handle, PBM_SETBKCOLOR, (WPARAM)0, (LPARAM)p_rgba);
}

void lwProgressBar::setBarColor( uint32_t p_rgba )
{
	SendMessage(this->m_handle, PBM_SETBARCOLOR, (WPARAM)0, (LPARAM)p_rgba);
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

	uint32_t ex_style = 0;
	uint32_t style = WS_VISIBLE | TBSTYLE_AUTOSIZE;
	if ( this->border      ) style |= WS_BORDER;

	this->m_handle = CreateWindowEx(0,
									TOOLBARCLASSNAME,
									nullptr,
									WS_CHILD | style,
									0,0,0,0,
									p_parent->m_handle,
									nullptr,
									GetModuleHandle( nullptr ),
									this );
	if ( this->m_handle == nullptr )
		return false;

	// Compatability fix
	SendMessage(this->m_handle,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);

	SetWindowLongPtr( this->m_handle, GWLP_USERDATA, (LONG)this );
	SendMessage( this->m_handle, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	return true;
}

void lwToolBar::setImageList( lwImageList* p_image_list )
{
	SendMessage( this->m_handle, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)p_image_list->image_list );
}

uint32_t lwToolBar::addButton(unsigned int idBitmap)
{
	TBBUTTON tbb;
	ZeroMemory( &tbb, sizeof(tbb) );
	tbb.iBitmap = 0;
	tbb.fsState = TBSTATE_ENABLED;
	tbb.fsStyle = TBSTYLE_BUTTON;
	tbb.idCommand = this->m_buttons.size();

	this->m_buttons.push_back( tbb );

	SendMessage(this->m_handle, TB_ADDBUTTONS, sizeof(TBBUTTON), (LPARAM)&tbb );

	return this->m_buttons.size()-1;
}

uint32_t lwToolBar::addSeparator(unsigned int idBitmap)
{
	TBBUTTON tbb;
	ZeroMemory( &tbb, sizeof(tbb) );
	tbb.iBitmap = 0;
	tbb.fsState = TBSTATE_ENABLED;
	tbb.fsStyle = TBSTYLE_SEP;
	tbb.idCommand = 0;

	this->m_buttons.push_back( tbb );

	SendMessage(this->m_handle, TB_ADDBUTTONS, sizeof(TBBUTTON), (LPARAM)&tbb );

	return this->m_buttons.size()-1;
}

void lwToolBar::removeButton( uint32_t p_index )
{
	SendMessage(this->m_handle, TB_ADDBUTTONS, (WPARAM)p_index, (LPARAM)0 );
}

void lwToolBar::resize( uint32_t p_width, uint32_t p_height )
{
	SendMessage( this->m_handle, TB_AUTOSIZE, 0, 0 );
}



/**********************************************************************************
** lw-resources.h
**********************************************************************************/

lwImage::lwImage()
{
	this->bitmap = nullptr;
}

lwImage::~lwImage()
{
	this->destroy();
}

bool lwImage::create( uint32_t p_width, uint32_t p_height, uint8_t p_bitdepth, void* p_data )
{
	this->bitmap = CreateBitmap( p_width, p_height, 1, p_bitdepth, p_data );

	if ( this->bitmap == nullptr )
		return false;

	return true;
}

void lwImage::destroy()
{
	DeleteObject( this->bitmap );
}


lwImageList::lwImageList()
{
	this->image_list = nullptr;
}

lwImageList::~lwImageList()
{
	this->destroy();
}

bool lwImageList::create( uint32_t p_width, uint32_t p_height, uint8_t p_bitdepth, uint16_t p_maximum_images )
{
	uint32_t flags = 0;

	switch (p_bitdepth)
	{
	case 8:
		flags |= ILC_COLOR8;
		break;
	case 16:
		flags |= ILC_COLOR16;
		break;
	case 24:
		flags |= ILC_COLOR24;
		break;
	case 32:
		flags |= ILC_COLOR32;
		break;
	}

	this->image_list = ImageList_Create( p_width, p_height, flags, 1, p_maximum_images );

	if ( this->image_list == nullptr )
		return false;

	return true;
}

void lwImageList::destroy()
{
	ImageList_Destroy( this->image_list );
}

bool lwImageList::addImage( lwImage* p_image )
{
	ImageList_Add( this->image_list, p_image->bitmap, nullptr );
	return true;
}

uint32_t lwImageList::getImageCount( void )
{
	return ImageList_GetImageCount( this->image_list );
}
