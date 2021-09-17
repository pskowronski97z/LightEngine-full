#pragma comment(lib, "opencv_world450d")
#include <WinMain.h>
#include <iostream>
#include <ImGui/imgui.h>


#define DEFAULT_WND_CLASS L"default_wnd_class"
#define CONTENT_CONTAINER L"content_container"

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

POINT AppWindow::IO::Mouse::mouse_position_{0, 0};
POINT AppWindow::IO::Mouse::mouse_delta_{0, 0};
HWND AppWindow::IO::Mouse::window_with_focus_ = nullptr;
bool AppWindow::IO::Mouse::left_button_down_ = false;
bool AppWindow::IO::Mouse::right_button_down_ = false;
bool AppWindow::IO::Mouse::middle_button_down_ = false;
int AppWindow::IO::Mouse::wheel_delta_ = 0;

HWND AppWindow::Window::resized_wnd_handle_ = nullptr;
int AppWindow::Window::resized_wnd_width_ = 0;
int AppWindow::Window::resized_wnd_height_ = 0;

HWND AppWindow::IO::Keyboard::window_with_focus_ = nullptr;
char AppWindow::IO::Keyboard::typed_char_ = 0;
bool AppWindow::IO::Keyboard::keys_down_[255]{false};
bool AppWindow::IO::Keyboard::keys_up_[255]{false};
bool AppWindow::IO::Keyboard::sys_keys_down_[255]{false};
bool AppWindow::IO::Keyboard::sys_keys_up_[255]{false};


std::string AppWindow::wstring_to_string(std::wstring wideString) {
	std::string result = "";
	for (auto& character : wideString) 
		result.push_back((char)character);

	return result;
}

std::string AppWindow::open_file_dialog(COMDLG_FILTERSPEC ComDlgFS[], int size) {
	IFileOpenDialog* fileOpenDialogPtr;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&fileOpenDialogPtr));
	std::string result="";
	if (SUCCEEDED(hr)) {
		 
		fileOpenDialogPtr->SetFileTypes(size, ComDlgFS);
		hr = fileOpenDialogPtr->Show(NULL);

		if (SUCCEEDED(hr)) {
			IShellItem* pItem;
			hr = fileOpenDialogPtr->GetResult(&pItem);

			if (SUCCEEDED(hr)) {
				PWSTR pszFilePath;
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

				if (SUCCEEDED(hr)) {
					std::wstring wPath(pszFilePath);
					result = wstring_to_string(wPath);
					CoTaskMemFree(pszFilePath);					
				}
				pItem->Release();
			}
		}
		fileOpenDialogPtr->Release();
	}
	return result;
}



std::wstring AppWindow::string_to_wstring(std::string source) {

	std::wstring result;

	for (auto &ch : source)
		result += (wchar_t)ch;
	return result;
}

std::string AppWindow::wchar_to_string(const wchar_t *text, const int &length) {

	std::string result;
	char char_buffer;

	for (int i = 0; i < length; i++) {
		char_buffer = (char)text[i];
		result += char_buffer;
	}

	return result;
}

LRESULT CALLBACK AppWindow::Window::wnd_proc(HWND wnd_handle, UINT msg, WPARAM w_param, LPARAM l_param) {
	
	if (ImGui_ImplWin32_WndProcHandler(wnd_handle, msg, w_param, l_param))
        return true;

	IO::Mouse::wheel_delta_ = 0;
	IO::Keyboard::typed_char_ = 0;

	switch (msg) {
	case WM_SIZE:
		resized_wnd_handle_ = wnd_handle;
		resized_wnd_width_ = LOWORD(l_param);
		resized_wnd_height_ = HIWORD(l_param);
		break;
	case WM_CHAR:
		IO::Keyboard::typed_char_ = (char)w_param;
		break;
	case WM_MOUSEWHEEL:

		if (HIWORD(w_param) == 120)
			IO::Mouse::wheel_delta_ = 1;
		else
			IO::Mouse::wheel_delta_ = -1;

		break;
		
	case WM_MOUSEMOVE:

		SetFocus(wnd_handle);
		IO::Mouse::window_with_focus_ = wnd_handle;
		IO::Keyboard::window_with_focus_ = wnd_handle;
		IO::Mouse::mouse_delta_.x = IO::Mouse::mouse_position_.x - LOWORD(l_param);
		IO::Mouse::mouse_delta_.y = IO::Mouse::mouse_position_.y - HIWORD(l_param);
		IO::Mouse::mouse_position_.x = LOWORD(l_param);
		IO::Mouse::mouse_position_.y = HIWORD(l_param);
		
		break;
		
	case WM_LBUTTONDOWN:
		IO::Mouse::left_button_down_ = true;
		break;

	case WM_LBUTTONUP:
		IO::Mouse::left_button_down_ = false;
		break;

	case WM_RBUTTONDOWN:
		IO::Mouse::right_button_down_ = true;
		break;

	case WM_RBUTTONUP:
		IO::Mouse::right_button_down_ = false;
		break;

	case WM_MBUTTONDOWN:

		IO::Mouse::middle_button_down_=true;
		break;

	case WM_MBUTTONUP:
		IO::Mouse::middle_button_down_=false;
		break;
		
	case WM_KEYDOWN:
		IO::Keyboard::keys_down_[(byte)w_param] = true;
		IO::Keyboard::keys_up_[(byte)w_param] = false;
		break;
		
	case WM_KEYUP:
		IO::Keyboard::keys_down_[(byte)w_param] = false;
		IO::Keyboard::keys_up_[(byte)w_param] = true;
		break;

	case WM_SYSKEYDOWN:
		IO::Keyboard::sys_keys_down_[(byte)w_param] = true;
		IO::Keyboard::sys_keys_up_[(byte)w_param] = false;
		break;

	case WM_SYSKEYUP:
		IO::Keyboard::sys_keys_down_[(byte)w_param] = false;
		IO::Keyboard::sys_keys_up_[(byte)w_param] = true;
		break;

	case WM_CLOSE:
		PostQuitMessage(404);
		break;

	default: break;
	}

	return DefWindowProc(wnd_handle, msg, w_param, l_param);
}

AppWindow::Window::Window(std::string title, const int &width, const int &height) 
	: title_(std::move(title)),
	  parent_wnd_handle_(nullptr),
	  wnd_class_(),
	  width_(width),
	  height_(height) {

	if(title_.empty())
		title_ = "Window 1";

	if(width_ < 50)
		width_ = 100;

	if(height_ < 50)
		height_ = 100;

	
	instance_ = GetModuleHandle(nullptr);

	wnd_class_.lpszClassName = DEFAULT_WND_CLASS;
	wnd_class_.lpfnWndProc = wnd_proc;
	wnd_class_.hInstance = instance_;
	wnd_class_.hCursor = nullptr;
	wnd_class_.hIcon = nullptr;
	wnd_class_.hbrBackground = (HBRUSH)(COLOR_BTNSHADOW + 1);
	wnd_class_.lpszMenuName = nullptr;
	wnd_class_.style = 0;
	wnd_class_.cbClsExtra = 0;
	wnd_class_.cbWndExtra = 0;

	RegisterClass(&wnd_class_);
	

	wnd_style_ = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;

	
	std::wstring w_title = string_to_wstring(title_);

	wnd_handle_ = CreateWindowExW(
		WS_EX_WINDOWEDGE,
		DEFAULT_WND_CLASS,
		w_title.c_str(),
		wnd_style_,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width_,
		height_,
		nullptr,
		nullptr,
		instance_,
		0);

}

AppWindow::Window::Window(Window &parent, const int &x, const int &y, const int &width, const int &height) noexcept
	: parent_wnd_handle_(parent.get_handle()),
	  wnd_class_(),
	  instance_(parent.get_instance()),
	  wnd_style_(WS_CHILD),
	  width_(width),
	  height_(height) {

	if(width_ < 50)
		width_ = 100;

	if(height_ < 50)
		height_ = 100;
	
	wnd_class_.lpszClassName = L"content_container";
	wnd_class_.lpfnWndProc = wnd_proc;
	wnd_class_.hInstance = instance_;
	wnd_class_.hCursor = nullptr;
	wnd_class_.hIcon = nullptr;
	wnd_class_.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wnd_class_.lpszMenuName = nullptr;
	wnd_class_.style = 0;
	wnd_class_.cbClsExtra = 0;
	wnd_class_.cbWndExtra = 1;

	RegisterClass(&wnd_class_);
	
	wnd_handle_ = CreateWindow(
		CONTENT_CONTAINER,
		L"",
		wnd_style_,
		x,
		y,
		width_,
		height_,
		parent_wnd_handle_,
		nullptr,
		instance_,
		nullptr);

}

AppWindow::Window::Window(const HWND &parent_handle, const HINSTANCE &instance, const int &x, const int &y, const int &width, const int &height)
	: parent_wnd_handle_(parent_handle),
	  wnd_class_(),
	  instance_(instance),
	  wnd_style_(WS_CHILD),
	  width_(width),
	  height_(height) {

	if(width_ < 50)
		width_ = 100;

	if(height_ < 50)
		height_ = 100;
	
	wnd_class_.lpszClassName = L"content_container";
	wnd_class_.lpfnWndProc = wnd_proc;
	wnd_class_.hInstance = instance_;
	wnd_class_.hCursor = nullptr;
	wnd_class_.hIcon = nullptr;
	wnd_class_.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wnd_class_.lpszMenuName = nullptr;
	wnd_class_.style = 0;
	wnd_class_.cbClsExtra = 0;
	wnd_class_.cbWndExtra = 1;

	RegisterClass(&wnd_class_);

	
	wnd_handle_ = CreateWindow(
		CONTENT_CONTAINER,
		L"",
		wnd_style_,
		x,
		y,
		width_,
		height_,
		parent_wnd_handle_,
		nullptr,
		instance_,
		nullptr);

}

bool AppWindow::Window::was_resized() noexcept {
	if(resized_wnd_handle_ == wnd_handle_) {
		resized_wnd_handle_ = nullptr;
		width_ = resized_wnd_width_;
		height_ = resized_wnd_height_;
		return true;
	}
	return false;
}

HWND AppWindow::Window::get_handle() const noexcept { return wnd_handle_; }

HINSTANCE AppWindow::Window::get_instance() const noexcept { return instance_; }

int AppWindow::Window::get_width() const noexcept { return width_; }

int AppWindow::Window::get_height() const noexcept { return height_; }

bool AppWindow::Window::set_icon(std::string file_path) const {
	std::wstring file_path_w = string_to_wstring(std::move(file_path));
	HANDLE icon = LoadImage(
		nullptr,
		file_path_w.c_str(),
		IMAGE_ICON,
		LR_DEFAULTSIZE,
		LR_DEFAULTSIZE,
		LR_DEFAULTCOLOR | LR_LOADFROMFILE);

	if (icon == nullptr)
		return false;

	SendMessage(wnd_handle_,WM_SETICON,ICON_BIG, (LPARAM)icon);
	return true;
}

void AppWindow::Window::show_window() const noexcept { ShowWindow(wnd_handle_, SW_SHOW); }

void AppWindow::Window::hide_window() const noexcept { ShowWindow(wnd_handle_, SW_HIDE); }

void AppWindow::Window::set_cursor(Cursor cursor_type) const noexcept {
	HCURSOR cursor = LoadCursor(nullptr, MAKEINTRESOURCE(cursor_type));
	SetClassLongPtr(wnd_handle_,GCLP_HCURSOR, (LONG_PTR)cursor);
}

void AppWindow::Window::set_style(Style wnd_style) noexcept {
	wnd_style_ |= (DWORD)wnd_style;
	SetWindowLongPtr(wnd_handle_,GWL_STYLE, (LONG_PTR)wnd_style_);
}

bool AppWindow::Window::set_menu(unsigned int resource_id) const noexcept {
	HMENU hmenu = LoadMenu(instance_, MAKEINTRESOURCE(resource_id));

	return SetMenu(wnd_handle_, hmenu);
}



int AppWindow::IO::Mouse::get_wheel_delta() {int buffer = wheel_delta_; wheel_delta_ = 0; return buffer; }

int AppWindow::IO::Mouse::get_position_x() { return mouse_position_.x; }

int AppWindow::IO::Mouse::get_position_y() { return mouse_position_.y; }

int AppWindow::IO::Mouse::get_position_dx() { return mouse_delta_.x; }

int AppWindow::IO::Mouse::get_position_dy() { return mouse_delta_.y; }

bool AppWindow::IO::Mouse::left_button_down() { return left_button_down_; }

bool AppWindow::IO::Mouse::right_button_down() { return right_button_down_; }

bool AppWindow::IO::Mouse::middle_button_down() { return middle_button_down_; }



bool AppWindow::IO::Keyboard::key_down(Key key) { return keys_down_[(byte)key]; }

bool AppWindow::IO::Keyboard::key_up(Key key) { return keys_up_[(byte)key]; }

bool AppWindow::IO::Keyboard::sys_key_down(Key key) { return sys_keys_down_[(byte)key]; }

bool AppWindow::IO::Keyboard::sys_key_up(Key key) { return sys_keys_up_[(byte)key]; }

char AppWindow::IO::Keyboard::get_typed_char() { return typed_char_; }

void AppWindow::IO::Keyboard::key_up_flush(Key key) { keys_up_[(byte)key] = false; }

void AppWindow::IO::Keyboard::sys_key_up_flush(Key key) { sys_keys_up_[(byte)key] = false; }