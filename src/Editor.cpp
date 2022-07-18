#include "Editor.h"

#include <imgui_internal.h.>
#include <backends/imgui_impl_vulkan.h>

static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse;

namespace Editor
{
	ImTextureID Editor::ViewportTexture;
}

void Editor::DrawEditor()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Editor", nullptr, window_flags);
	ImGui::PopStyleVar();
	ImGui::PopStyleVar(2);

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("Editor");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		static auto first_time = true;
		if (first_time)
		{
			first_time = false;

			ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
			ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

			// split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
			//   window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
			//                                                              out_id_at_dir is the id of the node in the direction we specified earlier, out_id_at_opposite_dir is in the opposite direction
			auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
			auto dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.8f, nullptr, &dockspace_id);
			auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.25f, nullptr, &dock_id_right);


			// we now dock our windows into the docking node we made above
			ImGui::DockBuilderDockWindow("Log", dock_id_down);
			ImGui::DockBuilderDockWindow("SceneGraph", dock_id_left);
			ImGui::DockBuilderDockWindow("Viewport", dock_id_right);
			ImGui::DockBuilderFinish(dockspace_id);
		}
	}

	ImGui::End();

	DrawViewport();
	DrawSceneGraph();
	DrawLog();
}

void Editor::DrawViewport()
{
	ImGui::Begin("Viewport");
	ImGui::Text("Hello, left!");
	//ImGui::Image(Editor::ViewportTexture, ImGui::GetWindowSize());
	ImGui::End();
}

void Editor::DrawSceneGraph()
{
	ImGui::Begin("SceneGraph");
	ImGui::Text("Hello, left!");
	ImGui::End();
}

void Editor::DrawLog()
{
	ImGui::Begin("Log");
	ImGui::End();
}
