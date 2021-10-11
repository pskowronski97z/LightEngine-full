#pragma once
#include <string>
#include <memory>
#include <ImGui/imgui.h>
#include <LEMaterial.h>
#include <LEBackend.h>

namespace LightEngineUI {

	namespace Frontend {

		class Window {
		protected:
			std::string name_;
			ImGuiWindowFlags flags_;
		public:
			Window(std::string name);
			virtual void render() = 0;
		};

		class MaterialEditor : public Window {
		private:
			class View {
			protected:
				std::vector<std::string> button_labels_;
				std::string material_name_;
			public:
				virtual void render(int window_width) = 0;
			};

			class DefaultView : public View {
			private:
				std::shared_ptr<LightEngine::DefaultMaterial> material_ptr_ = nullptr;
				LightEngine::DefaultParameters parameters_;
				bool flip_x_ = false;
				bool flip_y_ = false;
			public:
				DefaultView(std::shared_ptr<LightEngine::DefaultMaterial>& material_ptr);
				void render(int window_width) override;
			};

			class PBRView : public View {
				std::shared_ptr<LightEngine::PBRMaterial> material_ptr_ = nullptr;
				LightEngine::PBRParameters parameters_;
			public:
				PBRView(std::shared_ptr<LightEngine::PBRMaterial>& material_ptr);
				void render(int window_width) override;
			};

			std::shared_ptr<View> view_ptr = nullptr;

		public:
			MaterialEditor();
			void render() override;
			void load_material(std::shared_ptr<LightEngine::DefaultMaterial>& default_material_ptr);
			void load_material(std::shared_ptr<LightEngine::PBRMaterial>& pbr_material_ptr);
		};

		class TextureBrowser : public Window, public LightEngine::CoreUser {
		private:
			std::shared_ptr<LightEngineUI::Backend::TextureManager> texture_manager_;
			std::vector<const char*> names_;
			int selected_index_ = 0;
		public:
			TextureBrowser(LightEngineUI::Backend::TextureManager &texture_manager, std::shared_ptr<LightEngine::Core> &core_ptr);
			void render() override;
			int get_selected_item() const;
		};

	}
}