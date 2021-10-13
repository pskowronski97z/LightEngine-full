#pragma once
#include <vector>
#include <memory>
#include <string>
#include <LETexture.h>

namespace LightEngineUI {

	namespace Backend {

		template <class T>
		class Manager {
		protected:
			std::vector<std::shared_ptr<T>> elements_;
			std::vector<std::string> names_;
			Manager();
		public:
			void load(T &new_element, std::string new_element_name);
			bool remove(int index);
			std::vector<const char*> get_names() const;			
		};

		
		class TextureManager : public Manager<LightEngine::Texture> {
		private:
			int selected_texture_;
		public:
			TextureManager();
			std::shared_ptr<LightEngine::Texture> get_selected_texture() const;
			bool select_texture(int index);
		};

	}
}