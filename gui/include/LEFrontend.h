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
				std::shared_ptr<LightEngineUI::Backend::BrowserModel<LightEngine::Texture>> texture_browser_model_ptr_;
				std::vector<std::string> button_labels_;
				std::string material_name_;
			public:
				View(std::shared_ptr<LightEngineUI::Backend::BrowserModel<LightEngine::Texture>> &texture_browser_model_ptr);
				virtual void render(int window_width) = 0;
			};

			class BasicView : public View {
			private:
				std::shared_ptr<LightEngine::Materials::BasicMaterial> material_ptr_ = nullptr;
				LightEngine::DefaultParameters parameters_;
				bool flip_x_ = false;
				bool flip_y_ = false;
			public:
				BasicView(
					std::shared_ptr<LightEngine::Materials::BasicMaterial> &material_ptr, 
					std::shared_ptr<LightEngineUI::Backend::BrowserModel<LightEngine::Texture>> &texture_browser_model_ptr);
				void render(int window_width) override;
			};

			class PBRView : public View {
				std::shared_ptr<LightEngine::Materials::PBRMaterial> material_ptr_ = nullptr;
				LightEngine::PBRParameters parameters_;
			public:
				PBRView(
					std::shared_ptr<LightEngine::Materials::PBRMaterial>& material_ptr, 
					std::shared_ptr<LightEngineUI::Backend::BrowserModel<LightEngine::Texture>> &texture_browser_model_ptr);
				void render(int window_width) override;
			};

			std::shared_ptr<View> view_ptr = nullptr;
			std::shared_ptr<LightEngineUI::Backend::BrowserModel<LightEngine::Texture>> texture_browser_model_ptr_;
		public:
			MaterialEditor(std::shared_ptr<LightEngineUI::Backend::BrowserModel<LightEngine::Texture>> &model_ptr);
			void render() override;
			void load_material(std::shared_ptr<LightEngine::Materials::BasicMaterial> &default_material_ptr);
			void load_material(std::shared_ptr<LightEngine::Materials::PBRMaterial> &pbr_material_ptr);
		};

		template<class T>
		class Browser : public Window {
		protected:
			std::shared_ptr<LightEngineUI::Backend::BrowserModel<T>> browser_model_ptr_;
			std::vector<const char*> names_;
			Browser(std::string name, std::shared_ptr<LightEngineUI::Backend::BrowserModel<T>> &browser_model_ptr);
		
		};

		class TextureBrowser : public Browser<LightEngine::Texture>, public LightEngine::CoreUser {
		public:
			TextureBrowser(
				std::shared_ptr<LightEngineUI::Backend::BrowserModel<LightEngine::Texture>> &texture_browser_model_ptr,
				std::shared_ptr<LightEngine::Core> &core_ptr);
			void render() override;
		};
		
		class BasicMaterialsBrowser : public Browser<LightEngine::Materials::BasicMaterial>, public LightEngine::CoreUser {
		public:
			BasicMaterialsBrowser(std::shared_ptr<LightEngineUI::Backend::BrowserModel<LightEngine::Materials::BasicMaterial>> &bm_browser_model_ptr,
				std::shared_ptr<LightEngine::Core> &core_ptr);
			void render() override;	
		};
		
	}
}