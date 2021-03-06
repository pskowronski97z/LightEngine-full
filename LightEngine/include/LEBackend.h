#pragma once
#include <vector>
#include <memory>
#include <string>

namespace LightEngineUI {

	namespace Backend {

		template <class T>
		class BrowserModel {
		private:
			std::vector<std::shared_ptr<T>> elements_;
			std::vector<std::string> names_;
			int selected_item_;		
		public:
			BrowserModel();
			void load(T &new_element, std::string new_element_name);
			bool remove(int index);
			std::vector<const char*> get_names() const;			
			std::shared_ptr<T> get_selected_item() const;
			bool select_item(int index);
		};
		
	}
}