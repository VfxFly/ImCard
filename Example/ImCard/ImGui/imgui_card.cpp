#include "imgui_card.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

struct ImCard
{
	ImCard() : m_id(), m_size() { }
	ImCard(ImGuiID _id, const ImVec2& _size = ImVec2()) : m_id(_id), m_size(_size) { }

	ImGuiID m_id;
	ImVec2 m_size;
};

std::vector<std::shared_ptr<ImCard>> g_imCards;
std::shared_ptr<ImCard> g_card;

#pragma region Child
IMGUI_CARD_CALL ImGui::_private::BeginGridEx(const char* name, ImGuiID id, const ImVec2& size_arg, const ImCardStyle* params) -> bool
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* parent_window = g.CurrentWindow;

	ImGuiWindowFlags flags = params ? params->flags : NULL;
	flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow | (params ? ImGuiWindowFlags_NoBackground : NULL);
	flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove);  // Inherit the NoMove flag

	// Size
	const ImVec2 content_avail = GetContentRegionAvail();
	ImVec2 size = ImFloor(size_arg);
	const int auto_fit_axises = ((size.x == 0.0f) ? (1 << ImGuiAxis_X) : 0x00) | ((size.y == 0.0f) ? (1 << ImGuiAxis_Y) : 0x00);
	if (size.x <= 0.0f)
		size.x = ImMax(content_avail.x + size.x, 4.0f); // Arbitrary minimum child size (0.0f causing too much issues)
	if (size.y <= 0.0f)
		size.y = ImMax(content_avail.y + size.y, 4.0f);
	SetNextWindowSize(size);

	// Build up name. If you need to append to a same child from multiple location in the ID stack, use BeginChild(ImGuiID id) with a stable value.
	const char* temp_window_name;
	if (name)
		ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s_%08X", parent_window->Name, name, id);
	else
		ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%08X", parent_window->Name, id);

	const float backup_border_size = g.Style.ChildBorderSize;
	g.Style.ChildBorderSize = 0.0f;
	bool ret = Begin(temp_window_name, NULL, flags);
	g.Style.ChildBorderSize = backup_border_size;

	ImGuiWindow* child_window = g.CurrentWindow;
	child_window->ChildId = id;
	child_window->AutoFitChildAxises = (ImS8)auto_fit_axises;

	const ImU32 main_color = GetColorU32(ImVec4(params ? params->color : ImColor(ImGuiCol_ChildBg)));
	const ImU32 bord_color = GetColorU32(ImVec4(params && params->border ? params->border_color : ImColor(ImGuiCol_Border)));

	if (params && params->accent) {
		const ImU32 accent_color = GetColorU32(ImVec4(params && params->accent ? params->accent_color : ImColor(ImGuiCol_ChildBg)));
		child_window->DrawList->AddRectFilledMultiColor(child_window->Pos, child_window->Pos + size, accent_color, main_color, main_color, main_color);
	}
	if (params && !params->accent)
		child_window->DrawList->AddRectFilled(child_window->Pos, child_window->Pos + size, main_color, params ? params->rounding : 0.0f);
	child_window->DrawList->AddRect(child_window->Pos, child_window->Pos + size, bord_color, params ? params->rounding : 0.0f, NULL, params ? params->thickness : 1.0f);

	// Set the cursor to handle case where the user called SetNextWindowPos()+BeginChild() manually.
	// While this is not really documented/defined, it seems that the expected thing to do.
	if (child_window->BeginCount == 1)
		parent_window->DC.CursorPos = child_window->Pos;

	// Process navigation-in immediately so NavInit can run on first frame
	if (g.NavActivateId == id && !(flags & ImGuiWindowFlags_NavFlattened) && (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavHasScroll))
	{
		FocusWindow(child_window);
		NavInitWindow(child_window, false);
		SetActiveID(id + 1, child_window); // Steal ActiveId with another arbitrary id so that key-press won't activate child item
		g.ActiveIdSource = ImGuiInputSource_Nav;
	}
	return ret;
}

bool ImGui::_private::BeginGrid(ImGuiID id, const ImVec2& size_arg, const ImCardStyle* params)
{
	IM_ASSERT(id != 0);
	return BeginGridEx(NULL, id, size_arg, params);
}
#pragma endregion

IMGUI_CARD_CALL get_card(ImGuiID id) -> std::shared_ptr<ImCard>
{
	for (auto card : g_imCards) {
		if (card->m_id == id)
			g_card = card;
	}
	return g_card;
}

IMGUI_CARD_CALL resize_card(ImGuiID _id, const ImVec2& _size) -> void
{
	if (auto card = get_card(_id))
		card->m_size = _size;
}

IMGUI_CARD_CALL ImGui::BeginCard(const std::string& header, const ImCardStyle* params) -> void
{
	auto window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiID id = window->GetID(header.c_str());

	bool has_current = false;
	for (auto card : g_imCards) {
		if (card->m_id == id) {
			has_current = true;
			break;
		}
	}

	if (!has_current)
		g_imCards.push_back(std::make_shared<ImCard>(id));

	auto card = get_card(id);
	if (!card)
		return;

	auto style = GetStyle();
	const ImVec4 bg_color = params ? ImVec4(ImColor(GetColorU32(ImGuiCol_ChildBg, 0.0f))) : ImVec4(0.33f, 0.33f, 0.33f, 0.33f);
	PushStyleColor(ImGuiCol_ChildBg, bg_color);
	PushStyleVar(ImGuiStyleVar_ChildRounding, params ? params->rounding : 15.0f);

	_private::BeginGrid(id, card->m_size, params);
	{
		SetCursorPos(GetCursorPos() + style.WindowPadding);
		BeginGroup();
		Text(header.c_str());
		Separator();
	}
}

IMGUI_CARD_CALL ImGui::EndCard() -> void
{
	EndGroup();
	ImVec2 size = GetItemRectMax() - GetItemRectMin() + GetStyle().WindowPadding * 2.0f;
	EndChild();

	resize_card(GetItemID(), size);

	PopStyleVar();
	PopStyleColor();
}