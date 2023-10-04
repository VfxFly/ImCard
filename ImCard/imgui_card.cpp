#include "imgui_card.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <chrono>
#include <memory>
#include <format>

struct ImCard
{
	ImCard() : m_id(), m_size() { }
	ImCard(ImGuiID _id, const ImVec2& _size = ImVec2(), ImCardFlags _flags = ImCardFlags::none)
	{ 
		m_id = _id;
		m_size = _size;
		m_collapsedSize = ImVec2();
		m_content = true;
		m_flags = _flags;
		m_delta = 0.15f;
	}

	ImGuiID m_id;
	ImVec2 m_pos, m_size, m_collapsedSize;

	bool m_content;

	ImCardFlags m_flags;
	float m_delta;
};

std::vector<std::shared_ptr<ImCard>> g_imCards;
std::shared_ptr<ImCard> g_card;

#pragma region _private
IMGUI_CARD_CALL AddRectFilledMultiColor(ImDrawList* draw_list, const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left, float rounding, ImDrawFlags flags)
{
	if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
		return;

	if (flags == ~0)
		flags = ImDrawFlags_RoundCornersAll;
	if (flags >= 0x01 && flags <= 0x0F)
		flags = (flags << 4);

	if ((flags & ImDrawFlags_RoundCornersMask_) == 0)
		flags |= ImDrawFlags_RoundCornersAll;

	rounding = ImMin(rounding, ImFabs(p_max.x - p_min.x) * (((flags & ImDrawCornerFlags_Top) == ImDrawCornerFlags_Top) || ((flags & ImDrawCornerFlags_Bot) == ImDrawCornerFlags_Bot) ? 0.5f : 1.0f) - 1.0f);
	rounding = ImMin(rounding, ImFabs(p_max.y - p_min.y) * (((flags & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || ((flags & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f) - 1.0f);

	if (rounding > 0.0f)
	{
		const int size_before = draw_list->VtxBuffer.Size;
		draw_list->AddRectFilled(p_min, p_max, IM_COL32_WHITE, rounding, flags);
		const int size_after = draw_list->VtxBuffer.Size;

		for (int i = size_before; i < size_after; i++)
		{
			ImDrawVert* vert = draw_list->VtxBuffer.Data + i;

			ImVec4 upr_left = ImGui::ColorConvertU32ToFloat4(col_upr_left);
			ImVec4 bot_left = ImGui::ColorConvertU32ToFloat4(col_bot_left);
			ImVec4 up_right = ImGui::ColorConvertU32ToFloat4(col_upr_right);
			ImVec4 bot_right = ImGui::ColorConvertU32ToFloat4(col_bot_right);

			float X = ImClamp((vert->pos.x - p_min.x) / (p_max.x - p_min.x), 0.0f, 1.0f);

			// 4 colors - 8 deltas

			float r1 = upr_left.x + (up_right.x - upr_left.x) * X;
			float r2 = bot_left.x + (bot_right.x - bot_left.x) * X;

			float g1 = upr_left.y + (up_right.y - upr_left.y) * X;
			float g2 = bot_left.y + (bot_right.y - bot_left.y) * X;

			float b1 = upr_left.z + (up_right.z - upr_left.z) * X;
			float b2 = bot_left.z + (bot_right.z - bot_left.z) * X;

			float a1 = upr_left.w + (up_right.w - upr_left.w) * X;
			float a2 = bot_left.w + (bot_right.w - bot_left.w) * X;


			float Y = ImClamp((vert->pos.y - p_min.y) / (p_max.y - p_min.y), 0.0f, 1.0f);
			float r = r1 + (r2 - r1) * Y;
			float g = g1 + (g2 - g1) * Y;
			float b = b1 + (b2 - b1) * Y;
			float a = a1 + (a2 - a1) * Y;
			ImVec4 RGBA(r, g, b, a);

			RGBA = RGBA * ImGui::ColorConvertU32ToFloat4(vert->col);

			vert->col = ImColor(RGBA);
		}
		return;
	}

	const ImVec2 uv = draw_list->_Data->TexUvWhitePixel;
	draw_list->PrimReserve(6, 4);
	draw_list->PrimWriteIdx((ImDrawIdx)(draw_list->_VtxCurrentIdx));
	draw_list->PrimWriteIdx((ImDrawIdx)(draw_list->_VtxCurrentIdx + 1));
	draw_list->PrimWriteIdx((ImDrawIdx)(draw_list->_VtxCurrentIdx + 2));

	draw_list->PrimWriteIdx((ImDrawIdx)(draw_list->_VtxCurrentIdx));
	draw_list->PrimWriteIdx((ImDrawIdx)(draw_list->_VtxCurrentIdx + 2));
	draw_list->PrimWriteIdx((ImDrawIdx)(draw_list->_VtxCurrentIdx + 3));

	draw_list->PrimWriteVtx(p_min, uv, col_upr_left);
	draw_list->PrimWriteVtx(ImVec2(p_max.x, p_min.y), uv, col_upr_right);
	draw_list->PrimWriteVtx(p_max, uv, col_bot_right);
	draw_list->PrimWriteVtx(ImVec2(p_min.x, p_max.y), uv, col_bot_left);
}

IMGUI_CARD_CALL ImGui::_private::BeginGridEx(const char* name, ImGuiID id, const ImVec2& size_arg, const ImCardStyle* params) -> bool
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* parent_window = g.CurrentWindow;

	ImGuiWindowFlags flags = params ? params->flags : NULL;
	flags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow | (params ? ImGuiWindowFlags_NoBackground : NULL);
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
		AddRectFilledMultiColor(child_window->DrawList, child_window->Pos, child_window->Pos + size, accent_color, main_color, main_color, main_color, params ? params->rounding : 0.0f, NULL);
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

bool ImGui::_private::ToggleSwitch(const char* label, bool* v)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	float height = GetFrameHeight();
	const ImVec2 pos = window->DC.CursorPos;

	float width = height * 2.f;
	float radius = height * 0.50f;

	const ImRect bb(pos, ImVec2(pos.x + width + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), pos.y + label_size.y + style.FramePadding.y * 2.0f));

	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	float last_active_id_timer = g.LastActiveIdTimer;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);
	if (pressed)
	{
		*v = !(*v);
		MarkItemEdited(id);
		g.LastActiveIdTimer = 0.f;
	}

	if (g.LastActiveIdTimer == 0.f && g.LastActiveId == id && !pressed)
		g.LastActiveIdTimer = last_active_id_timer;

	float t = *v ? 1.0f : 0.0f;
	float circle_t = held ? 1.0f : 0.0f;

	if (g.LastActiveId == id)
	{
		float t_anim = ImSaturate(g.LastActiveIdTimer / 0.16f);
		t = *v ? (t_anim) : (1.0f - t_anim);
		circle_t = held ? (t_anim) : (1.0f - t_anim);
	}

	ImU32 col_bg = GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);

	const ImRect frame_bb(pos, ImVec2(pos.x + width, pos.y + height));

	RenderFrame(frame_bb.Min, frame_bb.Max, col_bg, true, height * 0.5f);
	RenderNavHighlight(bb, id);

	ImVec2 label_pos = ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y);
	RenderText(label_pos, label);
	window->DrawList->AddCircleFilled(ImVec2(pos.x + radius + t * (width - radius * 2.0f), pos.y + radius), radius - (circle_t * radius) / 5.f, GetColorU32(ImGuiCol_CheckMark), 16);

	return pressed;
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

IMGUI_CARD_CALL ImGui::BeginCard(const std::string& header, const ImCardStyle* params, bool collapsible) -> void { BeginCardEx(header, params, collapsible ? ImCardFlags::collapsible : ImCardFlags::none); }

IMGUI_CARD_CALL ImGui::BeginCardEx(const std::string& header, const ImCardStyle* params, ImCardFlags flags) -> void
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
		g_imCards.push_back(std::make_shared<ImCard>(id, ImVec2(), flags));

	auto card = get_card(id);
	if (!card)
		return;

	auto style = GetStyle();
	const ImVec4 bg_color = params ? ImVec4(ImColor(GetColorU32(ImGuiCol_ChildBg, 0.0f))) : ImVec4(0.33f, 0.33f, 0.33f, 0.33f);
	PushStyleColor(ImGuiCol_ChildBg, bg_color);
	PushStyleVar(ImGuiStyleVar_ChildRounding, params ? params->rounding : 15.0f);

	if (params)
		card->m_delta = ImClamp(params->collapsing_delta, 0.0f, 1.0f);

	_private::BeginGrid(id, card->m_size, params);
	{
		card->m_pos = GetCursorPos() + style.WindowPadding;
		SetCursorPos(card->m_pos);

		BeginGroup();
		Text(header.c_str());
		if (card->m_flags & (int)ImCardFlags::collapsible) {
			SameLine(card->m_pos.x + card->m_size.x - (GetFrameHeight() * 2.0f + style.WindowPadding.x * 3.0f));
			_private::ToggleSwitch(std::format("##toggle_switch_{}", header.c_str()).c_str(), &card->m_content);
		}

		card->m_collapsedSize = GetItemRectSize() + style.WindowPadding * 2.0f;

		PushStyleColor(ImGuiCol_Separator, GetColorU32(ImGuiCol_Separator, ImLerp(1.0f, 0.0f, card->m_collapsedSize.y / card->m_size.y)));
		Separator();
		PopStyleColor();
	}
}

IMGUI_CARD_CALL ImGui::EndCard() -> void
{
	EndGroup();
	ImVec2 size = GetItemRectMax() - GetItemRectMin() + GetStyle().WindowPadding * 2.0f;
	EndChild();

	size.x = ImClamp(GetItemRectSize().x, 0.0f, GetCurrentWindow()->Size.x - GetStyle().WindowPadding.x * 2.0f);

	auto id = GetItemID();
	auto card = get_card(id);

	if (!card->m_content)
		card->m_size.y = ImLerp(card->m_size.y, card->m_collapsedSize.y, card->m_delta);
	else
		card->m_size.y = ImLerp(card->m_size.y, size.y, card->m_delta);
	if (card->m_content)
		card->m_size.x = size.x;

	PopStyleVar();
	PopStyleColor();
}