#include <LEBackend.h>
#include <LETexture.h>
#include <LEMaterial.h>

template<class T>
LightEngineUI::Backend::BrowserModel<T>::BrowserModel() : selected_item_(0){}

template
LightEngineUI::Backend::BrowserModel<LightEngine::StaticTexture>::BrowserModel();

template
LightEngineUI::Backend::BrowserModel<LightEngine::Materials::BasicMaterial>::BrowserModel();

template<class T>
void LightEngineUI::Backend::BrowserModel<T>::load(T &new_element, std::string new_element_name) {
	std::shared_ptr<T> new_element_ = std::make_shared<T>(new_element);
	elements_.emplace_back(new_element_);
	names_.emplace_back(new_element_name);
}

template
void LightEngineUI::Backend::BrowserModel<LightEngine::StaticTexture>::load(LightEngine::StaticTexture &new_element, std::string new_element_name);

template<class T>
bool LightEngineUI::Backend::BrowserModel<T>::remove(int index) {
	if(index >= elements_.size())
		return false;

	elements_.erase(elements_.begin() + index);
	names_.erase(names_.begin() + index);

}

template
bool LightEngineUI::Backend::BrowserModel<LightEngine::StaticTexture>::remove(int index);


template<class T>
std::vector<const char*> LightEngineUI::Backend::BrowserModel<T>::get_names() const {
	std::vector<const char*> result;

	for(auto &name : names_)
		result.push_back(name.c_str());

	return result;
}

template
std::vector<const char*> LightEngineUI::Backend::BrowserModel<LightEngine::StaticTexture>::get_names() const;

template<class T>
std::shared_ptr<T> LightEngineUI::Backend::BrowserModel<T>::get_selected_item() const {
	if(selected_item_ >= elements_.size())
		return nullptr;

	return elements_[selected_item_];
}

template
std::shared_ptr<LightEngine::StaticTexture> LightEngineUI::Backend::BrowserModel<LightEngine::StaticTexture>::get_selected_item() const;

template<class T>
bool LightEngineUI::Backend::BrowserModel<T>::select_item(int index) {
	if(!elements_.size() || index >= elements_.size() || index < 0)
		return false;

	selected_item_ = index;
	return true;
}

template
bool LightEngineUI::Backend::BrowserModel<LightEngine::StaticTexture>::select_item(int index);

