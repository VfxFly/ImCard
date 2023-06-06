#ifndef _H__UI_H
#define _H__UI_H
#include "imgui.h"
#include "imgui_internal.h"

class UI
{
public:
	static void DrawUI(const ImVec2& size, const ImVec2& dpi);
};
#endif