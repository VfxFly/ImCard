#include "UI.h"

#include <string_view>
#include <imgui_card.h>


static ImCardStyle card_style;
void UI::DrawUI(const ImVec2& size, const ImVec2& dpi)
{
	ImGui::Begin("Test UI", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	{
		if (ImGui::CollapsingHeader("Card Style"))
		{
			ImGui::ColorEdit4("Color", (float*)&card_style.color);

			static bool use_accent = false;
			if (ImGui::Checkbox("Use accent", &card_style.accent))
				card_style.rounding = 0.0f;

			ImGui::BeginDisabled(!card_style.accent);
			ImGui::ColorEdit4("Accent Color", (float*)&card_style.accent_color);
			ImGui::EndDisabled();

			ImGui::Checkbox("Use border", &card_style.border);
			ImGui::BeginDisabled(!card_style.border);
			ImGui::ColorEdit4("Border Color", (float*)&card_style.border_color);
			ImGui::DragFloat("Thickness", &card_style.thickness, 0.1f, 0.0f);
			ImGui::EndDisabled();

			ImGui::BeginDisabled(card_style.accent);
			ImGui::DragFloat("Rounding", &card_style.rounding, 0.1f, 0.0f);
			ImGui::EndDisabled();
		}

		ImGui::BeginCard("Card Header", &card_style);
		{
			ImGui::Button("Button", { size.x - ImGui::GetStyle().WindowPadding.x * 2.0f, size.y });
			static bool check_box = false;
			ImGui::Checkbox("Check Box", &check_box);
			static char buffer[1024] = "Type a text or remove this...";
			ImGui::PushItemWidth(size.x - ImGui::GetStyle().WindowPadding.x * 2.0f);
			ImGui::InputTextMultiline("##input_text_multiline", buffer, 1024);
			ImGui::PopItemWidth();
			if (!std::string_view(buffer).empty())
				ImGui::Text(buffer);
		}
		ImGui::EndCard();

		ImGui::BeginCard("Second Card Header", &card_style);
		{
			ImGui::Button("Button", { size.x - ImGui::GetStyle().WindowPadding.x * 2.0f, size.y });
			static float slider = 0.0f;
			ImGui::PushItemWidth(size.x - ImGui::GetStyle().WindowPadding.x * 2.0f - ImGui::CalcTextSize("Slider").x);
			ImGui::SliderFloat("Slider", &slider, 0.0f, 100.0f, "%.2f");
		}
		ImGui::EndCard();
	}
	ImGui::End();
}