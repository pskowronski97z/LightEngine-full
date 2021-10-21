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
			int selected_item_;
			Manager();
		public:
			void load(T &new_element, std::string new_element_name);
			bool remove(int index);
			std::vector<const char*> get_names() const;			
			std::shared_ptr<T> get_selected_item() const;
			bool select_item(int index);
		};

		
		class TextureManager : public Manager<LightEngine::Texture> {
		public:
			TextureManager();
		};

	}
}