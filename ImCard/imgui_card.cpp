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

IMGUI_CARD_CALL ImGui::BeginCard(const std::string& header, ImColor color, float rounding) -> void
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
	PushStyleColor(ImGuiCol_ChildBg, ImVec4(color));
	PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);

	BeginChild(id, card->m_size); 
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