
/*************************************************************
** GTK Implementation
** Light Widget Library: Core-GTK
**
** Most commonly used controls for GUI's and core classes
**/

#undef lwNativeWindowType
#define lwNativeWindowType GtkWidget*

#include <gtk/gtk.h>

#include <cstdio>
#include <map>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>

#include "lw-main.h"
#include "lw-resources.h"


using namespace std;


/************************************************************************
** Light Widget Library Helpers
*/
typedef struct __eventheader
{
	lwStringType		type;
	lwBaseControl*		widget;
} lwEventHeaderType;

static void GlobalEventDispatcher( GtkWidget* p_widget, lwEventHeaderType *p_event )
{
	if ( p_event == nullptr ) return;

	if ( p_event->widget != nullptr )
	{
		if ( p_event->type == "destroy" )
		{
			if ( p_event->widget->onClose != nullptr ) p_event->widget->onClose();
		}
	}

	delete p_event;
}

lwApplication::lwApplication( int p_arg_count, char* p_arguments[] )
{
	gtk_init( &p_arg_count, &p_arguments );
}

lwApplication::~lwApplication()
{
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
	/// Linux/Unix:
	//return lwStringType( getcwd( p_directory, p_max_length ) ).length();
	/// Windows:
	//return GetModuleFileName( nullptr, p_directory, p_max_length );
	/// Bugger it:
	return 0;
}

uint32_t lwApplication::getScreenCount() const
{
	uint32_t screen_n = gdk_display_get_n_screens( gdk_screen_get_display( gdk_screen_get_default() ) );
	return screen_n;
}

uint32_t lwApplication::getScreenWidth( void ) const
{
	uint32_t screen_w = gdk_screen_get_width( gdk_screen_get_default() );
	return screen_w;
}

uint32_t lwApplication::getScreenHeight( void ) const
{
	uint32_t screen_h = gdk_screen_get_height( gdk_screen_get_default() );
	return screen_h;
}

bool lwApplication::messageLoop()
{
	// NOTE: gtk_main() is not acceptable here
	/*if ( !lw_main_thread.joinable() )
	{
		lw_main_thread = thread( lw_main_thread_fn );

		while ( !lw_main_thread_b );;
	}
    else if ( !lw_main_thread_b )
	{
		return false;
	}*/

	gtk_main();

	return true;
}


/************************************************************************
** lwBaseControl method & operator implementations
*/

lwBaseControl::lwBaseControl()
{
	this->m_handle = 0;
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
    if ( this->m_handle != nullptr && GTK_IS_WIDGET( this->m_handle ) )
    {
			gtk_widget_destroy( this->m_handle );
			this->m_handle = nullptr;
    }
}

bool lwBaseControl::setFont( lwFont* p_font )
{
	/// TODO (Rexhunter99#1#): Implement lwBaseControl::setFont()
	//PangoFontDescription pfd;
	//gtk_widget_override_font( this->m_handle, &pfd );
	return true;
}

bool lwBaseControl::setText( lwStringType p_text )
{
	if ( this->m_handle == nullptr ) return (false);

	return (true);
}

uint32_t lwBaseControl::getText( lwStringType& p_text )
{
	if ( this->m_handle == 0 ) return (false);

	return 0;
}

bool lwBaseControl::show( void )
{
	if ( this->m_handle == 0 ) return (false);

	gtk_widget_show( this->m_handle );

	return (true);
}

bool lwBaseControl::hide( void )
{
	if ( this->m_handle == 0 ) return (false);

	gtk_widget_hide( this->m_handle );

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

	if ( GTK_IS_WINDOW( this->m_handle ) )
	{
		gtk_window_set_default_size( (GtkWindow*)this->m_handle, p_width, p_height );
	}
}

void lwBaseControl::move( uint32_t p_x, uint32_t p_y )
{
	if ( this->m_handle == nullptr ) return;

	//gtk_widget_set_
}

void lwBaseControl::redraw()
{
	if ( this->m_handle == 0 ) return;
}

bool lwBaseControl::focus()
{
	if ( this->m_handle == 0 ) return false;

	return true;
}

void lwBaseControl::center()
{

}

void lwBaseControl::maximise()
{
}

void lwBaseControl::getArea( int32_t* x, int32_t* y, int32_t* w, int32_t* h )
{
}

void lwBaseControl::getClientArea( int32_t* x, int32_t* y, int32_t* w, int32_t* h )
{
}


/************************************************************************
** lwToolTip method implementations
*/
// TODO: clean up and optimise create() method

lwToolTip::lwToolTip()
{
	this->m_type = lwWidgetTypeEnum::LW_TOOLTIP;
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
** lwFrame method implementations
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
	if ( this->m_handle != nullptr )
		return true;

	this->m_handle = gtk_window_new( GTK_WINDOW_TOPLEVEL );

	if ( this->m_handle == nullptr )
	{
		fprintf( stderr, "lwFrame | ERROR | create | Failed to create the gtk_window!\n" );
		return false;
	}

	gtk_window_set_title( (GtkWindow*)this->m_handle, p_title );
	gtk_widget_show( this->m_handle );

	lwEventHeaderType *event = new lwEventHeaderType;
	event->widget = this;

	event->type = "destroy";
	g_signal_connect( this->m_handle, "destroy", (GCallback)GlobalEventDispatcher, event );

	return true;
}

void lwFrame::close()
{
	if ( this->m_handle == 0 ) return;

	// Send a signal as if the close button was clicked
}

void lwFrame::quit()
{
	if ( this->m_handle == nullptr ) return;

	gtk_main_quit();
}


bool lwFrame::run()
{
	if ( this->m_handle == nullptr ) return false;

	printf( "lwFrame | INFO | run | ...\n" );

	/*if ( gtk_main_iteration_do( false ) )
	{
		return false;
	}*/

	return true;
}

void lwFrame::getClientArea( int32_t* x, int32_t* y, int32_t* w, int32_t* h )
{
}

void lwFrame::center()
{
	lwBaseControl::center();
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

	this->m_handle = gtk_dialog_new();

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
	/// TODO (Rexhunter99#1#): Implement lwFileDialog

	return lwStringType( TEXT("") );
}

lwStringType lwFileDialog::save( lwBaseControl* p_window )
{
	return lwStringType( TEXT("") );
}


/************************************************************************
** lwTabGroup method implementations
*/

lwTabGroup::lwTabGroup()
{
	this->m_type = lwWidgetTypeEnum::LW_TABS;
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

	if ( this->m_handle == 0 )
	{
		printf( "lwButton | ERROR | create | Failed to create the gtk_widget\n" );
		return false;
	}

	if ( this->onCreate ) this->onCreate();

	return  true;
}

void lwTabGroup::resize( uint32_t p_width, uint32_t p_height )
{
	lwBaseControl::resize( p_width, p_height );
}

bool lwTabGroup::addTab( lwStringType p_name )
{
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
}


/************************************************************************
** lwComboBox method implementations
*/

lwComboBox::lwComboBox()
{
	this->m_type = lwWidgetTypeEnum::LW_COMBO_BOX;
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

void lwComboBox::setSelection( uint16_t p_index )
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
	this->m_type = lwWidgetTypeEnum::LW_LIST_BOX;
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
	this->m_type = lwWidgetTypeEnum::LW_LIST_VIEW;
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


/**********************************************************************
** Light Widget Tree View
*/

typedef struct __treeview_item_data
{
	GtkTreeIter             handle;
	__treeview_item_data    *parent;
	lwStringType            name;
} __treeview_item_data;

typedef struct __treeview_data
{
	GtkTreeStore*							store;
	lwImageList*							imagelist;
	map<lwStringType, __treeview_item_data>	items;
} __treeview_data;

typedef map<lwStringType, __treeview_item_data> __treeview_item_map;


lwTreeView::lwTreeView()
{
	this->m_type = lwWidgetTypeEnum::LW_TREEVIEW;
	this->m_data = new __treeview_data;
}

lwTreeView::~lwTreeView()
{
	g_object_unref( ((__treeview_data*)this->m_data)->store );
	delete reinterpret_cast<__treeview_data*>(this->m_data);
	this->destroy();
}

bool lwTreeView::create( lwBaseControl* p_parent, const char* p_title )
{
	__treeview_data		*data = reinterpret_cast<__treeview_data*>(this->m_data);

	if ( p_parent == nullptr ) return false;
	if ( !GTK_IS_CONTAINER( p_parent->m_handle) && !GTK_IS_WINDOW( p_parent->m_handle ) ) return false;

	this->m_handle = gtk_tree_view_new();

	if ( this->m_handle == 0 )
	{
		printf( "lwTreeView | ERROR | create | Failed to create the gtk_widget\n" );
		return false;
	}

	GtkTreeViewColumn *col;

	col = gtk_tree_view_column_new();
	//gtk_tree_view_column_set_title( col, "" );
	gtk_tree_view_append_column( GTK_TREE_VIEW(this->m_handle), col );

	GtkCellRenderer *ren_pixbuf = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start( col, ren_pixbuf, true );
	gtk_tree_view_column_add_attribute( col, ren_pixbuf, "pixbuf", 0 );

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title( col, p_title );
	gtk_tree_view_append_column( GTK_TREE_VIEW(this->m_handle), col );

	GtkCellRenderer *ren_text = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start( col, ren_text, true );
	gtk_tree_view_column_add_attribute( col, ren_text, "text", 1 );

	data->store = gtk_tree_store_new( 2, GDK_TYPE_PIXBUF, G_TYPE_STRING );

	if ( data->store == nullptr )
	{
		printf( "lwTreeView | ERROR | create | Failed to create the gtk_tree_store\n" );
	}

	gtk_tree_view_set_model( GTK_TREE_VIEW(this->m_handle), GTK_TREE_MODEL(data->store) );

	gtk_container_add( GTK_CONTAINER(p_parent->m_handle), this->m_handle );

	gtk_widget_show( this->m_handle );

	return  true;
}

bool lwTreeView::setImageList( lwImageList* p_image_list )
{
	__treeview_data			*data = reinterpret_cast<__treeview_data*>(this->m_data);

	if ( p_image_list == nullptr )
		return false;

	data->imagelist = p_image_list;

	return true;
}

uint32_t lwTreeView::addItemText( lwStringType p_text, lwStringType p_parent )
{
	return this->addItemTextImage( p_text, -1, p_parent );
}

uint32_t lwTreeView::addItemTextImage( lwStringType p_text, int p_image, lwStringType p_parent )
{
	GtkTreeIter				item;
	GtkTreeIter				*parent = nullptr;
	__treeview_data			*data = reinterpret_cast<__treeview_data*>(this->m_data);
	__treeview_item_data	*parent_item_data = nullptr;
	__treeview_item_map::iterator parent_it = data->items.end();
	__treeview_item_data	new_item_data;

	// -- If a parent was specified then find its data
	if ( p_parent != TEXT("") )
	{
		parent_it = data->items.find( p_parent );

		if ( parent_it != data->items.end() )
		{
			parent = &parent_it->second.handle;
		}
		else return false;
	}

	gtk_tree_store_append( data->store, &item, parent );

	if ( p_image == -1 || p_image >= data->imagelist->getImageCount() )
		gtk_tree_store_set( data->store, &item, 1, p_text.c_str(), -1 );
	else
		gtk_tree_store_set( data->store, &item, 1, p_text.c_str(), 0, data->imagelist->getImage( p_image ), -1 );

	new_item_data.handle = item;
	new_item_data.parent = &parent_it->second;
	data->items.insert( pair<lwStringType, __treeview_item_data>(p_text, new_item_data) );

	return true;
}

uint32_t lwTreeView::getBKColor()
{
	return 0xFF000000;
}


//

lwButton::lwButton()
{
	this->m_type = lwWidgetTypeEnum::LW_BUTTON;
}

lwButton::~lwButton()
{
	this->destroy();
}

bool lwButton::create( lwBaseControl* p_parent, const char *p_title )
{
	if ( p_parent == nullptr )
		return false;

	this->m_handle = gtk_button_new_with_label( p_title );

	if ( this->m_handle == 0 )
	{
		printf( "lwButton | ERROR | create | Failed to create the gtk_widget\n" );
		return false;
	}

	return  true;
}

void lwButton::setBitmap( lwImage* p_image )
{
}


//

lwTextArea::lwTextArea()
{
	this->m_type = lwWidgetTypeEnum::LW_TEXTAREA;
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
	this->m_type = lwWidgetTypeEnum::LW_GROUP;
}

lwGroup::~lwGroup()
{
	this->destroy();
}

bool lwGroup::create( lwBaseControl* p_parent, char *p_title )
{
	return false;
}


//

lwCheckBox::lwCheckBox()
{
	this->m_type = lwWidgetTypeEnum::LW_CHECK_BOX;
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
	this->m_type = lwWidgetTypeEnum::LW_STATIC;
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

bool lwMenuBar::addPopupMenu( lwStringType p_menu_name, lwStringType p_name )
{
	return true;
}

bool lwMenuBar::addItem( lwStringType p_menu_name, lwStringType p_name, lwFuncType p_event )
{
	return 0;
}

bool lwMenuBar::insertItem( lwStringType p_menu_name, uint32_t p_pos, lwStringType p_name, lwFuncType p_event )
{
	return 0;
}

void lwMenuBar::removeItem( lwStringType p_menu_name, uint32_t p_pos )
{
}

bool lwMenuBar::addSeparator( lwStringType p_menu_name )
{
	return false;
}

bool lwMenuBar::addBreak( lwStringType p_menu_name )
{
	return false;
}

void lwMenuBar::update()
{
}

/// Private Method
lwNativeWindowType lwMenuBar::i_findMenu( lwStringType p_name )
{
	return 0;
}


//

lwStatusBar::lwStatusBar()
{
	this->m_type = lwWidgetTypeEnum::LW_STATUS_BAR;
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

void lwStatusBar::setSectionText( uint16_t p_index, lwStringType p_text )
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
	this->m_type = lwWidgetTypeEnum::LW_PROGRESS_BAR;
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

bool lwToolBar::create( lwFrame* p_parent )
{
	if ( p_parent == nullptr )
		return false;

	return true;
}

void lwToolBar::setImageList( lwImageList* p_image_list )
{
}

uint32_t lwToolBar::addButton( lwStringType p_text )
{
	return 0;
}

uint32_t lwToolBar::addButtonImage( uint32_t p_bitmap, lwStringType p_text )
{
	return 0;
}


uint32_t lwToolBar::addSeparator()
{
	return 0;
}

void lwToolBar::removeButton( uint32_t p_index )
{
}

void lwToolBar::resize( uint32_t p_width, uint32_t p_height )
{
}
