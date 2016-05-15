/*************************************************************
** Win32 & COM Implementation
** Light Widget Library: Resources
**
** Resource containers
**/

#undef lwNativeImageType
#define lwNativeImageType GdkPixbuf*
#define lwNativeImageListType void*
#define lwNativeFontType void*

#include <gtk/gtk.h>

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "lw-resources.h"

using namespace std;


/************************************************************************
** lwImage : Image(BMP/TGA/PNG) resource implementation
*/

lwImage::lwImage()
{
	this->bitmap = nullptr;
}

lwImage::lwImage( const lwImage& p_image )
{
	this->bitmap = p_image.bitmap;
	const_cast<lwImage&>(p_image).bitmap = nullptr;
}

lwImage::~lwImage()
{
	this->destroy();
}

bool lwImage::create( uint32_t p_width, uint32_t p_height, uint8_t p_bitdepth, void* p_data, uint32_t p_colorkey, uint32_t p_bgcolor )
{
	if ( this->bitmap != nullptr )
	{
		this->destroy();
	}

	this->bitmap = gdk_pixbuf_new_from_data( (const guchar*)p_data,
											 GDK_COLORSPACE_RGB,
											 ( p_bitdepth == 16 || p_bitdepth == 32 ) ? true : false,
											 p_bitdepth,
											 p_width,
											 p_height,
											 p_width * ( p_bitdepth / 8 ),
											 nullptr,
											 nullptr );

	if ( this->bitmap == nullptr )
	{
		fprintf( stderr, "lwImage | ERROR | create | Failed to create the GdkPixmap!\n" );
		return false;
	}

	this->width = gdk_pixbuf_get_width( this->bitmap );
	this->height = gdk_pixbuf_get_height( this->bitmap );
	this->bitdepth = gdk_pixbuf_get_bits_per_sample( this->bitmap ) * gdk_pixbuf_get_n_channels( this->bitmap );

	return true;
}

bool lwImage::createFromFile( string p_filename, uint32_t p_colorkey, uint32_t p_bgcolor )
{
	GError* e = nullptr;

	this->bitmap = gdk_pixbuf_new_from_file( p_filename.c_str(), &e );

	if ( this->bitmap == nullptr || e != nullptr )
	{
		fprintf( stderr, "lwImage | ERROR | createFromFile | Failed to create the GdkPixmap!\n" );
		return false;
	}

	this->width = gdk_pixbuf_get_width( this->bitmap );
	this->height = gdk_pixbuf_get_height( this->bitmap );
	this->bitdepth = gdk_pixbuf_get_bits_per_sample( this->bitmap ) * gdk_pixbuf_get_n_channels( this->bitmap );

	return true;
}

void lwImage::destroy()
{
	if ( this->bitmap != nullptr )
		g_object_unref( this->bitmap );
}


/************************************************************************
** lwImageList : List of images implementation
*/

typedef struct __imagelist_data
{
	vector<lwImage> images;
	int width;
	int height;
	int bitdepth;
} __imagelist_data;

lwImageList::lwImageList()
{
	this->m_data = new __imagelist_data;
	this->image_list = nullptr;
}

lwImageList::~lwImageList()
{
	delete (__imagelist_data*)this->m_data;
	this->destroy();
}

bool lwImageList::create( uint32_t p_width, uint32_t p_height, uint8_t p_bitdepth, uint16_t p_maximum_images )
{
	__imagelist_data* data = reinterpret_cast<__imagelist_data*>(this->m_data);

	data->width = p_width;
	data->height = p_height;
	data->bitdepth = p_bitdepth;

	if ( data->images.size() > 0 )
	{
		this->destroy();
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
	// TODO (Rexhunter99#1#): Implement destroy() for lwImageList
	__imagelist_data* data = reinterpret_cast<__imagelist_data*>(this->m_data);

	data->images.clear();
}

bool lwImageList::addImage( lwImage* p_image )
{
	// TODO (Rexhunter99#1#): Implement addImage() for lwImageList
	__imagelist_data* data = reinterpret_cast<__imagelist_data*>(this->m_data);

	if ( p_image == nullptr )
		return false;
	if ( p_image->bitmap == nullptr )
		return false;
	if ( p_image->width != data->width )
		return false;
	if ( p_image->height != data->height )
		return false;

	data->images.push_back( lwImage( *p_image ) );

	return true;
}

lwNativeImageType lwImageList::getImage( uint32_t p_index )
{
	__imagelist_data* data = reinterpret_cast<__imagelist_data*>(this->m_data);

	if ( p_index >= this->getImageCount() )
		return nullptr;

	return data->images[ p_index ].bitmap;
}

uint32_t lwImageList::getImageCount( void )
{
	__imagelist_data* data = reinterpret_cast<__imagelist_data*>(this->m_data);
	return data->images.size();
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
	/*this->m_handle = new (std::nothrow) PangoFontDescription;

	if ( this->m_handle == nullptr )
	{
		fprintf( stderr, "lwFont | ERROR | create | Failed to create the font resource!\n" );
		return false;
	}

	if ( p_italic == true )
		this->m_handle->PangoStyle = PANGO_STYLE_ITALIC;*/

	return true;
}

bool lwFont::load( lwStringType p_filename )
{
	return true;
}

void lwFont::destroy()
{
	if ( this->m_handle != nullptr )
		delete this->m_handle;

	this->m_handle = nullptr;
}
