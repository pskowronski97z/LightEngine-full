#include <LEBackend.h>

template<class T>
LightEngineUI::Backend::Manager<T>::Manager() {}

template<class T>
void LightEngineUI::Backend::Manager<T>::load(T &new_element, std::string new_element_name) {
	std::shared_ptr<T> new_element_ = std::make_shared<T>(new_element);
	elements_.emplace_back(new_element_);
	names_.emplace_back(new_element_name);
}

template
void LightEngineUI::Backend::Manager<LightEngine::Texture>::load(LightEngine::Texture &new_element, std::string new_element_name);

template<class T>
bool LightEngineUI::Backend::Manager<T>::remove(int index) {
	if(index >= elements_.size())
		return false;

	elements_.erase(elements_.begin() + index);
	names_.erase(names_.begin() + index);

}

template
bool LightEngineUI::Backend::Manager<LightEngine::Texture>::remove(int index);


template<class T>
std::vector<const char*> LightEngineUI::Backend::Manager<T>::get_names() const {
	std::vector<const char*> result;

	for(auto &name : names_)
		result.push_back(name.c_str());

	return result;
}

template
std::vector<const char*> LightEngineUI::Backend::Manager<LightEngine::Texture>::get_names() const;

template<class T>
std::shared_ptr<T> LightEngineUI::Backend::Manager<T>::get_element(int index) const {
	if(index >= elements_.size())
		return nullptr;

	return elements_[index];
}

template
std::shared_ptr<LightEngine::Texture> LightEngineUI::Backend::Manager<LightEngine::Texture>::get_element(int index) const;

LightEngineUI::Backend::TextureManager::TextureManager() : Manager() {}
