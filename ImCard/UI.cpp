#include "UI.h"

#include <string_view>
#include <imgui_card.h>

void UI::DrawUI(const ImVec2& size, const ImVec2& dpi)
{
	ImGui::Begin("Test UI", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	{
		ImGui::BeginCard("Card Header");
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

		ImGui::BeginCard("Second Card Header");
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