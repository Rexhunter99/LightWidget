/*************************************************************
** Win32 & COM Implementation
** Light Widget Library: Resources
**
** Resource containers
**/

#undef WINVER
#undef _WIN32_IE
#undef _WIN32_WINNT
#undef _WIN32_WINNT
#define _WIN32_IE       	0x0800      // IE 8
#define _WIN32_WINNT    	0x0601      // Windows 7
#define WINVER          	0x0601      // Windows 7

#define IDB_BITMAP1			129
#define IDB_BACKGRND		130
#define IDB_OPEN_FILE		131
#define IDB_CLOSED_FILE		132
#define IDB_DOCUMENT		133

#include <Windows.h>
#include <CommCtrl.h>
#include <Windowsx.h>

#include <cstdio>
#include <string>
#include <fstream>

#include "lw-resources.h"

using namespace std;


/************************************************************************
** lwImage : Image(BMP/TGA/PNG) resource implementation
*/

lwImage::lwImage()
{
	this->bitmap = nullptr;
}

lwImage::~lwImage()
{
	this->destroy();
}

bool lwImage::create( uint32_t p_width, uint32_t p_height, uint8_t p_bitdepth, void* p_data, uint32_t p_colorkey, uint32_t p_bgcolor )
{
	// > MSDN recommends CreateCompatibleBitmap for performance reasons and it is suited for color bitmaps
	//this->bitmap = CreateBitmap( p_width, p_height, 1, p_bitdepth, p_data );
	uint32_t *bits = nullptr;
	HDC dc = GetDC( NULL );
	HDC mdc = nullptr;
	BITMAPV5HEADER bi;

	ZeroMemory(&bi,sizeof(BITMAPV5HEADER));
	bi.bV5Size			= sizeof(BITMAPV5HEADER);
	bi.bV5Width			= p_width;
	bi.bV5Height		= p_height;
	bi.bV5Planes		= 1;
	bi.bV5BitCount		= p_bitdepth;
	bi.bV5Compression	= BI_RGB;
    bi.bV5RedMask   =  0x00FF0000;
    bi.bV5GreenMask =  0x0000FF00;
    bi.bV5BlueMask  =  0x000000FF;
    bi.bV5AlphaMask =  0xFF000000;

	this->bitmap = CreateDIBSection( dc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&bits, nullptr, 0 );
	//mdc = CreateCompatibleDC( dc );
	ReleaseDC( NULL, dc );

	if ( this->bitmap == nullptr )
	{
		fprintf( stderr, "lwImage | ERROR | Failed to create the bitmap resource. Error Code: %u\n", GetLastError() );
		return false;
	}

	memcpy( bits, p_data, p_width * p_height * ( p_bitdepth / 8 ) );

	if ( p_colorkey != 0x01000000 )
	for ( unsigned int i = 0; i < ( p_height * p_width ); i++ )
	{
		uint32_t *pix = (uint32_t*)(bits) + i;
		if ( *pix == p_colorkey )
		*( pix ) = p_bgcolor;
	}

	return true;
}

bool lwImage::createFromFile( string p_filename, uint32_t p_colorkey, uint32_t p_bgcolor )
{

	/*this->bitmap = LoadImage( NULL, p_filename.c_str(), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE );

	if ( this->bitmap == nullptr )
	{
		fprintf( stderr, "lwImage | ERROR | Failed to create the bitmap resource %d.\n", GetLastError() );
		return false;
	}

	return true;*/
	fstream file;

	file.open( p_filename.c_str(), ios::in | ios::binary );

	if ( !file.is_open() )
	{
		fprintf( stderr, "lwImage | ERROR | Failed to open the image file.\n" );
		return false;
	}

	BITMAPINFOHEADER bi;
	BITMAPFILEHEADER bf;
	file.read( (char*)&bf, sizeof(BITMAPFILEHEADER) );
	file.read( (char*)&bi, sizeof(BITMAPINFOHEADER) );

	char *data = new (std::nothrow) char [ bi.biSizeImage ];

	if ( data == nullptr )
	{
		fprintf( stderr, "lwImage | SYSTEM ERROR | Unable to allocate memory for the bitmap data.\n" );
		return false;
	}
	file.seekg( bf.bfOffBits + 1 );
	file.read( data, bi.biSizeImage );

	file.close();

	fprintf( stderr, "lwImage | INFO | %d %d %d\n", bi.biWidth, bi.biHeight, bi.biBitCount );

	bool r = this->create( bi.biWidth, bi.biHeight, bi.biBitCount, data, p_colorkey, p_bgcolor );

	delete data;

	return r;
}

void lwImage::destroy()
{
	DeleteObject( this->bitmap );
}


/************************************************************************
** lwImageList : List of images implementation
*/

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
	{
		fprintf( stderr, "lwImageList | ERROR | Failed to create the imagelist resource.\n" );
		return false;
	}

	return true;
}

bool lwImageList::createFromFile( uint32_t p_width, uint32_t p_height, uint32_t p_maximum_images, string p_filename )
{
	fstream file;

	file.open( p_filename.c_str(), ios::in | ios::binary );

	if ( !file )
	{
		// -- error
		return false;
	}

	file.close();

	if ( !this->create( 1,1, 16, 1 ) )
	{
		// -- Error
		return false;
	}

	// TODO (Rexhunter99#1#): Implement imagestrip loading for lwImageList

	return true;
}

void lwImageList::destroy()
{
	ImageList_Destroy( (HIMAGELIST)this->image_list );
}

bool lwImageList::addImage( lwImage* p_image )
{
	ImageList_Add( (HIMAGELIST)this->image_list, (HBITMAP)p_image->bitmap, nullptr );
	return true;
}

/*bool lwImageList::addSystemImage( uint32_t p_image )
{
	HBITMAP hbmp = LoadBitmap( GetModuleHandle( nullptr ), MAKEINTRESOURCE( IDB_DOCUMENT ));
    ImageList_Add( (HIMAGELIST)this->image_list, hbmp, (HBITMAP)NULL);
    DeleteObject(hbmp);

    return true;
}*/

uint32_t lwImageList::getImageCount( void )
{
	return ImageList_GetImageCount( (HIMAGELIST)this->image_list );
}


/************************************************************************
** lwBaseControl method & operator implementations
*/

lwFont::lwFont()
{
	this->m_handle = nullptr;
	this->m_resource = "";
}

lwFont::~lwFont()
{
	this->destroy();
}

bool lwFont::create( lwStringType p_fontname, uint32_t p_size, bool p_bold, bool p_italic )
{
	HDC hDC = GetDC( HWND_DESKTOP );
	this->m_handle = CreateFont(-MulDiv(p_size, GetDeviceCaps(hDC, LOGPIXELSY), 72),
								0,				// Width
								0,				// Escapement
								0,				// Orientation
								(p_bold)?700:0,	// Boldness
								p_italic,
								0,				// Underline
								0,				// Strikeout
								ANSI_CHARSET,	// Charset
								OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,
								DEFAULT_QUALITY,
								DEFAULT_PITCH | FF_DONTCARE,
								p_fontname.c_str() );

	if ( this->m_handle == nullptr )
		return false;

	return true;
}

bool lwFont::load( lwStringType p_filename )
{
	if ( AddFontResource( p_filename.c_str() ) == 0 )
	{
		printf( "lwFont::load() - Failed to load the font as a resource!\n" );
		return false;
	}

	SendMessage( HWND_BROADCAST, WM_FONTCHANGE, 0, 0 );

	return true;
}

void lwFont::destroy()
{
	if ( this->m_handle != nullptr )
		DeleteObject( (HANDLE)this->m_handle );

	if ( this->m_resource == "" )
		RemoveFontResource( this->m_resource.c_str() );

	this->m_handle = nullptr;
	this->m_resource = "";
}
