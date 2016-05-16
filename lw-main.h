#pragma once

#ifndef __LIGHTWIDGET_MAIN_H__
#define __LIGHTWIDGET_MAIN_H__

// -- Includes
#include <stdint.h>
#include <string>
#include <vector>

// -- Unicode/MultiByte string
#if defined( UNICODE )
	typedef std::wstring lwStringType;
#else
	typedef std::string lwStringType;
#endif // UNICODE || UTF16

/// Define the TEXT macro to support wide and multichar string literals like WinAPI
#if defined( UNICODE ) && !defined( TEXT )
	#undef __TEXT
	#define __TEXT( s ) L##s
	#define TEXT(s) __TEXT(s)
#else
	#undef __TEXT
	#define __TEXT( s ) s
	#define TEXT(s) __TEXT(s)
#endif

#ifndef lwNativeWindowType
	#define lwNativeWindowType void*
#endif

// -- Forward declare classes
class lwBaseControl;
class lwToolBar;
class lwStatusBar;
class lwImage;
class lwImageList;
class lwFont;

// -- Typedefs
/** @typedef lwFuncType
 ** @brief A generic function pointer type
 **/
typedef void (*lwFuncType)(void);


/** @enum lwWidgetTypeEnum
 ** @brief A list of identifiers used for internal identification of widgets
 ** This enum is used internally (and externally if a developer desires) to
 ** identify a lwBaseControl instance's true class type
 **/
enum lwWidgetTypeEnum {
	LW_UNKNOWN_WIDGET				= 0,
	LW_BUTTON,
	LW_CHECK_BOX,
	LW_COMBO_BOX,
	LW_DIALOG,
	LW_FRAME,
	LW_GROUP,
	LW_LIST_BOX,
	LW_LIST_VIEW,
	LW_PROGRESS_BAR,
	LW_STATIC,
	LW_STATUS_BAR,
	LW_RADIO_BUTTON,
	LW_RIBBON_BAR,	///REBAR WIDGET
	LW_TABS,
	LW_TEXTINPUT,
	LW_TEXTAREA,
	LW_TOOL_BAR,
	LW_TOOLTIP,
	LW_TRACKBAR,
	LW_TREEVIEW,

	LW_VIDEO_CANVAS                 = 0xFF,
	// -- Any 2D/3D Canvas' must use "LW_VIDEO_CANVAS + #" where # is a unique number that helps identify it
	LW_LAST_WIDGET					= 0xFFFF,
	// -- Any custom widgets must use "LW_LAST_WIDGET + #" Where # is a unique number that helps identify it
	LW_DWORD						= 0xFFFFFFFF
};


class lwApplication
{
private:

	lwStringType m_title, m_author, m_version;

public:

	lwApplication( int p_arg_count, char* p_arguments[] );
	~lwApplication();

	void		setAuthor( lwStringType p_author );
	void		setTitle( lwStringType p_title );
	void		setVersion( lwStringType p_version );
	lwStringType	getAuthor( ) const;
	lwStringType	getTitle( ) const;
	lwStringType	getVersion( ) const;
	size_t		getWorkingDirectory( char* p_directory, size_t p_max_length );
	size_t      getApplicationDirectory( char* p_directory, size_t p_max_length );
	uint32_t	getScreenCount() const;
	uint32_t	getScreenWidth() const;
	uint32_t	getScreenHeight() const;

	bool        messageLoop();
	bool        messageLoop(void(*p_callback)(void));
};


/** @class lwBaseControl
 * @brief The base class definition for all widgets
 * lwBaseControl pure virtual class for all widget implementations
 * contains a range of standard functions, also contains
 * operators and constructor/destructors to handle the classes.
 */
class lwBaseControl
{
protected:
	void*				m_data;

public:

	// -- Properties
	uint32_t	border:1;
	uint32_t	toolWindow:1;
	uint32_t	dialogFrame:1;
	uint32_t	clientEdge:1;

	lwWidgetTypeEnum	m_type;
	lwNativeWindowType	m_handle;

	lwBaseControl();
	~lwBaseControl();

	/** @fn destroy()
	 ** Close/Destroy and cleanup the widget
	 */
	virtual void	destroy( void );

	/** @fn setText( lwStringType text )
	 ** @param text A C++ string of variable width to set the widget's text to.
	 ** @return True if able to set the text, false if not able to or an error ocurred.
	 ** Set's the widget's internal string to a new string, this can be a window title, button label, text area contents, etc.
	 */
	virtual bool	setText( lwStringType p_text );

	/** @fn setFont( lwFont *font )
	 ** @param font A pointer to a Light Widget Font object, or nullptr for the system default.
	 ** @return True if able to set the font, false if unable or an error ocurred.
	 ** Set the widget's internal rendering font for text
	 */
	virtual bool	setFont( lwFont* p_font );

	/** @fn getText( lwStringType &text )
	 ** @param text A reference to a C++ string object to copy the widget's text into
	 ** @return The number of characters copied into the reference object
	 ** Acquire the internal string for the widget and store it in the reference string.
	 */
	virtual size_t	getText( lwStringType& p_text );

	virtual void	getArea( int32_t* x, int32_t* y, int32_t* w, int32_t* h );
	virtual void	getClientArea( int32_t* x, int32_t* y, int32_t* w, int32_t* h );
	lwNativeWindowType		getNativeHandle() const;
	virtual bool	show( void );
	virtual bool	hide( void );
	virtual bool	enable( void );
	virtual bool	disable( void );
	virtual bool	focus( void );
	virtual void	resize( uint32_t p_width, uint32_t p_height );
	virtual void	move( uint32_t p_x, uint32_t p_y );
	virtual void	redraw( void );
	virtual void	center();
	virtual void	maximise();

	// -- Event Callbacks
	void (*onCreate)( void );
	void (*onDestroy)( void );
	void (*onClose)( void );
	void (*onEnable)( void );
	void (*onDisable)( void );
	void (*onClick)( void );
	void (*onRClick)( void );
	void (*onKeyPress)( uint16_t );
	void (*onKeyRelease)( uint16_t );
	void (*onResize)( int32_t, int32_t );
	void (*onMove)( int32_t, int32_t );
	void (*onShow)( void );
	void (*onHide)( void );
};


/** @class lwFrame
 * @brief A window frame for displaying widgets to the user
 * A customiseable, runtime built, window.
 /// @todo (Rexhunter99#1#): Complete lwFrame implementation
 */
class lwFrame : public lwBaseControl
{
protected:
	friend class lwToolBar;
	friend class lwStatusBar;

	lwBaseControl	*m_tool_bar;

public:

	lwBaseControl	*m_status_bar;

	uint32_t	titleBar:1;
	uint32_t	maximizeButton:1;
	uint32_t	minimizeButton:1;
	uint32_t	closeButton;
	uint32_t	sizeable:1;
	uint32_t	acceptFiles:1;

	lwFrame();
	~lwFrame();

	void center();
	void close();
	bool create( lwBaseControl* p_parent, const char* p_title );
	void getClientArea( int32_t* x, int32_t* y, int32_t* w, int32_t* h );
	void quit();
	bool run();
};


/** @class lwDialog
 * @brief Dialog window implementation
 * A customiseable, runtime built, dialog window.
 * TODO: Complete lwDialog implementation
 */
class lwDialog : public lwBaseControl
{
private:

public:

	uint32_t	titleBar:1;
	uint32_t	maximizeButton:1;
	uint32_t	minimizeButton:1;
	uint32_t	closeButton;
	uint32_t	sizeable:1;
	uint32_t	acceptFiles:1;

	lwDialog();
	~lwDialog();

	/** @fn create( lwBaseControl* parent, lwStringType title )
	 ** @param parent The widget that this widget belongs to
	 ** @param title The text that is assigned to the title bar of this widget
	 ** @return bool True if succeed, false if failed.
	 **/
	bool create( lwBaseControl* p_parent, lwStringType p_title );
};


/** @class lwFileDialog
 * @brief A commonly used dialog window for opening and saving files
 * Contains the Open/Save File Dialog and it's properties in a set
 * of easily manageable methods.
 */
class lwFileDialog
{
private:
	lwStringType		m_path_filename;
	lwStringType		m_extension_filter;
	lwStringType		m_default_extension;
	lwStringType		m_initial_directory;
	lwStringType		m_title;

public:

	uint32_t	explorer:1;
	uint32_t	allowSizing:1;
	uint32_t	pathMustExist:1;
	uint32_t	fileMustExist:1;
	uint32_t	hideReadOnly:1;
	uint32_t	readOnly;
	uint32_t	allowMultiSelect:1;
	uint32_t	createFilePrompt:1;
	uint32_t	overwriteFilePrompt:1;

	lwFileDialog( );

	void setInitialDirectory( lwStringType p_initial_directory );
	void setDefaultExt( lwStringType p_default_extension );
	void setFilters( lwStringType p_filters );
	void setTitle( lwStringType p_title );
	lwStringType open( lwBaseControl* p_window );
	lwStringType save( lwBaseControl* p_window );
};


/** @class lwTextInput
 * @brief A basic, single line text input widget
 * Text input for a single line of text, supports
 * digits only and also masking characters with '*'
 * for password input.
 */
class lwTextInput : public lwBaseControl
{
public:

	uint32_t	readOnly:1;
	uint32_t	passwordInput:1;
	uint32_t	numbersInput:1;
	uint32_t	alignLeft:1;
	uint32_t	alignCenter:1;
	uint32_t	alignRight:1;

	lwTextInput();
	~lwTextInput();

	bool create( lwBaseControl* p_parent );
};


/** @class lwTextArea
 * @brief A basic, multi line text input widget
 * Text input for a multiple lines of text, supports
 * digits only and also masking characters with '*'
 * for password input.
 */
class lwTextArea : public lwBaseControl
{
public:

	uint32_t	readOnly:1;
	uint32_t	passwordInput:1;
	uint32_t	numbersInput:1;
	uint32_t	alignLeft:1;
	uint32_t	alignCenter:1;
	uint32_t	alignRight:1;

	lwTextArea();
	~lwTextArea();

	bool create( lwBaseControl* p_parent );
};


/** @class lwTabGroup
 * @brief A tab widget
 * lwTab is a more complex widget internally than most other widgets
 * as it keeps track of its state and hides any widgets that are in a
 * tab that is not active, yet unhides any that are in the active
 * tab. This abstracts away a level of complexity from the GUI API
 * that developers would normally have to implement themselves.
 */
class lwTabGroup : public lwBaseControl
{
public:

	class lwTab
	{
	public:
		lwStringType name;
		std::vector<lwBaseControl*> children;
	};

	std::vector<lwTab> m_tabs;

	uint32_t image:1;

    lwTabGroup();
    ~lwTabGroup();

    bool create( lwBaseControl* p_parent );

    void resize( uint32_t p_width, uint32_t p_height );
    bool addTab( lwStringType );
    bool removeTab( const char* p_name );
    bool addControlToTab( lwStringType p_name, lwBaseControl* p_control );
    bool removeControlFromTab( const char* p_tab_name, lwBaseControl* p_control );
    void getClientArea( int32_t& p_left, int32_t& p_top, int32_t& p_width, int32_t& p_height );
};


/** @class lwComboBox
 ** @brief A list of items capable of multi-selection
 ** ComboBox widget used to allow end users to select multiple items in the box
 ** @todo (James#1): Change all const char* types to lwStringType
 **/
class lwComboBox : public lwBaseControl
{
public:
    lwComboBox();
	~lwComboBox();

    bool create( lwBaseControl* p_parent, const char *p_title );

	/** @fn addItem(const char *p_item)
	** @param p_item A string defining the name of the item
	**/
	void addItem( const char* p_item );

	/** @fn setSelection(uint16_t p_index)
	** @param p_index The index of an item to select
	**/
	void setSelection( uint16_t p_index );

	/** @fn getSelection( void )
	** @return The index of the item that is selected
	**/
	uint16_t getSelection( void );

	/** @fn clearItems( void)
	** @brief Remove all items from the list
	**/
    void clearItems( void );
};


/** @class lwListBox
 ** @brief A list of items that the user can select from
 ** List Box widget used to allow end users to select an item from a list,
 ** displayed in an easy to navigate box that consumes less space
 ** @todo (James#1): Change all const char* types to lwStringType
 **/
class lwListBox : public lwBaseControl
{
public:
    lwListBox();
	~lwListBox();

    bool create( lwBaseControl* p_parent, char *p_title );

    /** @fn addItem(const char* p_name)
	** @param p_name A string defining the name of the item
	**/
	void addItem( const char* p_name );

	/** @fn setSelection(uint16_t p_index)
	** @param p_index The index of an item to select
	**/
	void setSelection( uint16_t p_index );

	/** @fn getSelection( void )
	** @return The index of the item that is selected
	**/
	uint16_t getSelection( void );

	/** @fn clearItems( void)
	** @brief Remove all items from the list
	**/
    void clearItems( void );
};


/******************************************************************
* lwListView Control
* A standard list view control
*/
class lwListView : public lwBaseControl
{
public:
	lwListView();
	~lwListView();

    bool create( lwBaseControl* p_parent, const char* p_title );
	bool setImageList( lwImageList* p_image_list );
    uint32_t addItemText( const char* p_text, int p_item );
	uint32_t addItemTextImage( const char* p_text, int p_image, int p_item );
};


/******************************************************************
* lwTreeView Control
* A standard tree view control
*/
class lwTreeView : public lwBaseControl
{
private:

	std::vector<lwNativeWindowType>	m_tree_items;

public:

	uint32_t	itemEditable:1;
	uint32_t	itemButtons:1;

	lwTreeView();
	~lwTreeView();

	bool create( lwBaseControl* p_parent, const char* p_title );
	bool setImageList( lwImageList* p_image_list );
	uint32_t addItemText( lwStringType p_text, lwStringType p_parent = "" );
	uint32_t addItemTextImage( lwStringType p_text, int p_image = -1, lwStringType p_parent = "" );
	uint32_t getBKColor();

	bool (*onItemEditBegin)( lwStringType p_string );
	bool (*onItemEditEnd)( lwStringType p_string );
};



/******************************************************************
 * @class lwButton
 * A standard pushbutton widget
 */
class lwButton : public lwBaseControl
{
public:

    lwButton();
	~lwButton();

    bool create( lwBaseControl* p_parent, const char *p_title );
    void setBitmap( lwImage* p_image );
};


/******************************************************************
* @class lwGroup
* A standard group container widget
*/
class lwGroup : public lwBaseControl
{
public:

    lwGroup();
    ~lwGroup();

	bool create( lwBaseControl* p_parent, char *p_title );
};


/******************************************************************
 * @class lwCheckBox
 * A standard checkbox widget
 */
class lwCheckBox : public lwBaseControl
{
public:

    lwCheckBox();
	~lwCheckBox();

	bool create( lwBaseControl* p_parent, const char* p_title );
    void setCheck( bool p_check );
	bool getCheck( void );
};


/******************************************************************
 * @class lwStatic
 * A standard static widget
 */
class lwStatic : public lwBaseControl
{
public:

	uint32_t image:1;
    lwStatic();
    ~lwStatic();

	bool create( lwBaseControl* p_parent, const char* p_title );
	void setImage( lwImage* p_image );
};


/******************************************************************
 * @class lwSysLink
 * A standard system link widget
 */
class lwSysLink : public lwBaseControl
{
public:

    lwSysLink();
    ~lwSysLink();

	bool create( lwBaseControl* p_parent, const char* p_text );
};


/******************************************************************
 * @class lwMenuBar
 * A standard menu bar widget
 */
class lwMenuBar
{
private:
    lwNativeWindowType   			m_handle;
    lwBaseControl*		m_parent;
    std::vector< std::pair<lwStringType,lwNativeWindowType> >	m_submenus;

	lwNativeWindowType i_findMenu( lwStringType p_name );

public:
	std::vector< lwFuncType > m_events;

    lwMenuBar();
    ~lwMenuBar();

    bool create( lwBaseControl* p_parent );
    bool addPopupMenu( lwStringType p_menu_name, lwStringType p_name );
	bool addItem( lwStringType p_menu_name, lwStringType p_name, lwFuncType p_event );
	bool insertItem( lwStringType p_menu_name, uint32_t p_pos, lwStringType p_name, lwFuncType p_event );
    void removeItem( lwStringType p_menu_name, uint32_t p_pos );
    bool addSeparator( lwStringType p_menu_name );
	bool addBreak( lwStringType p_menu_name );
	void update();
};


/******************************************************************
 * @class lwStatusBar
 * Standard status bar widget
 */
class lwStatusBar : public lwBaseControl
{
public:

    lwStatusBar();
    ~lwStatusBar();

    bool create( lwFrame* p_parent, uint16_t p_parts, int32_t* p_part_width );
    void setSectionText( uint16_t p_index, lwStringType p_text );
    void resize( uint32_t p_width, uint32_t p_height );
    uint32_t getHeight();
};


/******************************************************************
 * @class lwProgressBar
 * Standard progress bar widget
 */
class lwProgressBar : public lwBaseControl
{
public:

	lwProgressBar();
	~lwProgressBar();

    bool create( lwBaseControl* p_parent );
    void setMarquee( bool p_marquee, uint32_t p_update_ms );
	void setRange( uint16_t p_min, uint16_t p_max );
	void setPosition( uint16_t p_index );
	void setState( uint16_t p_state );
	void setStep( uint16_t p_step_size );
	void step( void );
	void setBackgroundColor( uint32_t p_rgba );
	void setBarColor( uint32_t p_rgba );
};


/******************************************************************
 * @class lwTrackBar
 * Standard slider bar
 */
class lwTrackBar : public lwBaseControl
{
public:

	lwTrackBar();
	~lwTrackBar();

	uint32_t auto_ticks:1;
	uint32_t vertical:1;
	uint32_t ticks_left:1;
	uint32_t ticks_right:1;
	uint32_t ticks_top:1;
	uint32_t ticks_bottom:1;

    bool create( lwBaseControl* p_parent );
	void setRange( uint16_t p_min, uint16_t p_max );
	void setPosition( uint16_t p_index );
	uint16_t getPosition() const;
};


/******************************************************************
 * @class lwToolBar
 * Standard tool bar widget
 */
class lwToolBar : public lwBaseControl
{
private:
    std::vector<void*> m_buttons;

public:

    lwToolBar();
    ~lwToolBar();

    bool create( lwFrame* p_parent );
    bool create( lwDialog* p_parent );
	void setImageList( lwImageList* p_image_list );
	uint32_t addButton( std::string p_text );
	uint32_t addButtonImage( uint32_t p_bitmap, std::string p_text );
    uint32_t addSeparator( );
    void removeButton( uint32_t p_index );
    void resize( uint32_t p_width, uint32_t p_height );
};


/******************************************************************
 * @class lwToolTip
 * Standard tool tip widget
 * @todo This widget doesn't display
 */
class lwToolTip : public lwBaseControl
{
private:
	lwBaseControl* control;

public:

	lwToolTip();
	~lwToolTip();

	bool create( lwBaseControl* p_parent, lwBaseControl* p_target_control, const char* p_tip_text);
};

#endif // __LIGHTWIDGET_MAIN_H__
