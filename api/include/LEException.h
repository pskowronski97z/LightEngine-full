#pragma once
#include <exception>
#include <string>
#include <Windows.h>

namespace LightEngine {

	class __declspec(dllexport) LECoreException : std::exception {
	private:
		int line_;
		unsigned long code_;
		std::string file_name_;
		std::string description_;
		LPTSTR message_;
	public:
		LECoreException(std::string description, std::string file_name, int line, unsigned long code);
		char const *what() const override;
		wchar_t *get_message() const;
	};

}
