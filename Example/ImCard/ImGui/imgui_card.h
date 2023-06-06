#ifndef _H__IMGUI_CARD_H
#define _H__IMGUI_CARD_H

#ifndef IMGUI_CARD_CALL
#define IMGUI_CARD_CALL auto
#endif

#include <imgui.h>

#include <string>
#include <vector>
#include <memory>

struct ImCardStyle
{
	ImColor color;

	bool accent;
	ImColor accent_color;

	bool border;
	ImColor border_color;
	float thickness;

	float rounding;
	ImGuiWindowFlags flags;
};

namespace ImGui
{
	/// <summary>
	/// Opens the stack auto-expandable card view with changeable background color. Used as Begin()/End()
	/// </summary>
	/// <param name="header - Card name and text in upper field"></param>
	/// <param name="color - Background color"></param>
	/// <param name="rounding - Background ñorners rounding scale"></param>
	IMGUI_CARD_CALL BeginCard(const std::string& header, const ImCardStyle* params = nullptr) -> void;

	/// <summary>
	/// Closes the stack of card view interface elements
	/// </summary>
	IMGUI_CARD_CALL EndCard() -> void;

	// Internal Functions
	namespace _private
	{
		IMGUI_CARD_CALL BeginGrid(ImGuiID id, const ImVec2& size_arg = ImVec2(0.0f, 0.0f), const ImCardStyle* params = nullptr) -> bool;
		IMGUI_CARD_CALL BeginGridEx(const char* name, ImGuiID id, const ImVec2& size_arg = ImVec2(0.0f, 0.0f), const ImCardStyle* params = nullptr) -> bool;
	}
}
#endif