#pragma once

#ifndef __LW_RICHEDIT_H__
#define __LW_RICHEDIT_H__
#include "lw-main.h"


/** @class lwRichEdit
 ** @brief A widget with formatable text
 ** @author Rexhunter99
 ** @version 1
 ** @date __DATE__
 ** A widget that contains text that is usually formatted with color,
 ** font styles and images.
 ** @todo (James#1#): Actually implement this in "lw-richedit-w32.cpp"
 **/
class lwRichEdit
{
private:

public:

	lwRichEdit();
	~lwRichEdit();

	uint32_t	disable_scrollbars:1;
	uint32_t	disable_dragndrop:1;
	uint32_t	preserve_selection:1;
	uint32_t	border_sunken:1;
	uint32_t	read_only:1;
	uint32_t	password_mask:1;
	uint32_t	digits_only:1;
	uint32_t	align_left:1;
	uint32_t	align_center:1;
	uint32_t	align_right:1;
	uint32_t	auto_scroll:1;

	bool create( lwBaseControl* p_parent );
	lwStringType getSelectedText();
	lwStringType getAllText();
	bool setText( lwStringType p_text );
};

#endif // __LW_RICHEDIT_H__
