#pragma once

#include "lw-main.h"

#if defined( BACKEND_W32 )
	#define lwNativeImageType void*
	#define lwNativeImageListType void*
	#define lwNativeFontType void*
#elif defined( BACKEND_GTK ) && !defined( lwNativeImageType )
	#define lwNativeImageListType void*
	#define lwNativeImageType void*
	#define lwNativeFontType void*
#endif


/******************************************************************
* lwResource Base Interface
*/
class lwResource
{
protected:
	void	*m_data;

public:

	virtual void destroy() = 0;
};


/******************************************************************
* lwImage Resource
* A container for system-specific pixel maps
// TODO: Implement in X11 and under GTK
*/
class lwImage : public lwResource
{
public:
	lwNativeImageType	bitmap;
	uint32_t			width,
						height,
						bitdepth;

	lwImage();
	lwImage( const lwImage& p_image );
	~lwImage();

	bool create( uint32_t p_width, uint32_t p_height, uint8_t p_bitdepth, void* p_data, uint32_t p_colorkey = 0x01000000, uint32_t p_bgcolor = 0xFF000000 );
	bool createFromFile( lwStringType p_filename, uint32_t p_colorkey = 0x01000000, uint32_t p_bgcolor = 0xFF000000  );
	void destroy();
};


/******************************************************************
* lwImageList Resource
* A container for a list of images or lwImage instances
*/
class lwImageList : public lwResource
{
public:
	lwNativeImageListType	image_list;

	lwImageList();
	~lwImageList();

	bool create( uint32_t p_width, uint32_t p_height, uint8_t p_bitdepth, uint16_t p_maximum_images );
	bool createFromFile( uint32_t p_width, uint32_t p_height, uint32_t p_maximum_images, std::string p_filename );
	void destroy();
	bool addImage( lwImage* p_image );
	lwNativeImageType getImage( uint32_t p_index );
	uint32_t getImageCount( void );
};


/******************************************************************
* lwFont Resource
* A container for font resources
// WARNING: On Win32 & COM implementations this can cause hangs (unknown reason)
*/
class lwFont
{
public:
	lwNativeFontType	m_handle;
	lwStringType	m_resource;

	lwFont();
	~lwFont();

	bool create( lwStringType p_fontname, uint32_t p_size, bool p_bold, bool p_italic );
	void destroy();
	bool load( lwStringType p_filename );
};
