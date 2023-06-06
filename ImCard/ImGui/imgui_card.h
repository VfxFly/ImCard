#ifndef _H__IMGUI_CARD_H
#define _H__IMGUI_CARD_H

#ifndef IMGUI_CARD_CALL
#define IMGUI_CARD_CALL auto
#endif

#include <imgui.h>

#include <string>
#include <vector>
#include <memory>

namespace ImGui
{
	/// <summary>
	/// Opens the stack auto-expandable card view with changeable background color. Used as Begin()/End()
	/// </summary>
	/// <param name="header - Card name and text in upper field"></param>
	/// <param name="color - Background color"></param>
	/// <param name="rounding - Background ñorners rounding scale"></param>
	IMGUI_CARD_CALL BeginCard(const std::string& header, ImColor color = ImVec4(0.33, 0.33, 0.33, 0.33), float rounding = 15.0f) -> void;

	/// <summary>
	/// Closes the stack of card view interface elements
	/// </summary>
	IMGUI_CARD_CALL EndCard() -> void;
}
#endif