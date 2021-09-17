#include <LEException.h>
#include <Windows.h>

wchar_t unicode_oem852_dictionary[170]{0};

void unicode_to_oem852(wchar_t *source) {

	if (!unicode_oem852_dictionary[0]) {
		unicode_oem852_dictionary[0] = 224;
		unicode_oem852_dictionary[32] = 162;
		unicode_oem852_dictionary[49] = 164;
		unicode_oem852_dictionary[50] = 165;
		unicode_oem852_dictionary[51] = 142;
		unicode_oem852_dictionary[52] = 134;
		unicode_oem852_dictionary[69] = 168;
		unicode_oem852_dictionary[70] = 169;
		unicode_oem852_dictionary[110] = 157;
		unicode_oem852_dictionary[111] = 136;
		unicode_oem852_dictionary[112] = 227;
		unicode_oem852_dictionary[113] = 228;
		unicode_oem852_dictionary[135] = 152;
		unicode_oem852_dictionary[136] = 152;
		unicode_oem852_dictionary[166] = 141;
		unicode_oem852_dictionary[167] = 171;
		unicode_oem852_dictionary[168] = 189;
		unicode_oem852_dictionary[169] = 190;
	}

	int dictionary_index;
	int length = wcslen(source);

	for (int itr = 0; itr < length; itr++) {
		dictionary_index = source[itr] - 211;

		if(!unicode_oem852_dictionary[dictionary_index]) 
			continue;
			
		if (dictionary_index < 170 && dictionary_index >= 0) 
			source[itr] = unicode_oem852_dictionary[dictionary_index];
			
	}
	
}


LightEngine::LECoreException::LECoreException(std::string description, std::string file_name, int line, unsigned long code)
	: line_(line), file_name_(std::move(file_name)), description_(std::move(description)), code_(code) {

	FormatMessage(
	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		code,
		GetUserDefaultLangID(),
		(LPTSTR)&message_,
		0,
		nullptr);
	
	unicode_to_oem852(message_);
	
}

char const* LightEngine::LECoreException::what() const {
	std::string result = description_ + "<File " + file_name_ + " at line " + std::to_string(line_) +">: "+ "\n" + "|-> " ;
	char *c_str_result = new char[result.size()+1];
	strcpy_s(c_str_result,result.size()+1,result.c_str());
	return c_str_result;
}

wchar_t *LightEngine::LECoreException::get_message() const { return message_; }
