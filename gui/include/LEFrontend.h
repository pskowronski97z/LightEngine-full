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
				std::shared_ptr<LightEngineUI::Backend::TextureManager> texture_manager_ptr_;
				std::vector<std::string> button_labels_;
				std::string material_name_;
			public:
				View(std::shared_ptr<LightEngineUI::Backend::TextureManager> &texture_manager_ptr);
				virtual void render(int window_width) = 0;
			};

			class DefaultView : public View {
			private:
				std::shared_ptr<LightEngine::DefaultMaterial> material_ptr_ = nullptr;
				LightEngine::DefaultParameters parameters_;
				bool flip_x_ = false;
				bool flip_y_ = false;
			public:
				DefaultView(std::shared_ptr<LightEngine::DefaultMaterial>& material_ptr, std::shared_ptr<LightEngineUI::Backend::TextureManager> &texture_manager);
				void render(int window_width) override;
			};

			class PBRView : public View {
				std::shared_ptr<LightEngine::PBRMaterial> material_ptr_ = nullptr;
				LightEngine::PBRParameters parameters_;
			public:
				PBRView(std::shared_ptr<LightEngine::PBRMaterial>& material_ptr, std::shared_ptr<LightEngineUI::Backend::TextureManager> &texture_manager);
				void render(int window_width) override;
			};

			std::shared_ptr<View> view_ptr = nullptr;
			std::shared_ptr<LightEngineUI::Backend::TextureManager> texture_manager_ptr_;
		public:
			MaterialEditor(std::shared_ptr<LightEngineUI::Backend::TextureManager> &texture_manager_ptr);
			void render() override;
			void load_material(std::shared_ptr<LightEngine::DefaultMaterial>& default_material_ptr);
			void load_material(std::shared_ptr<LightEngine::PBRMaterial>& pbr_material_ptr);
		};

		class TextureBrowser : public Window, public LightEngine::CoreUser {
		private:
			std::shared_ptr<LightEngineUI::Backend::TextureManager> texture_manager_ptr_;
			std::vector<const char*> names_;

		public:
			TextureBrowser(std::shared_ptr<LightEngineUI::Backend::TextureManager> &texture_manager_ptr, std::shared_ptr<LightEngine::Core> &core_ptr);
			void render() override;
		};

	}
}