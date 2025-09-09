#pragma once
struct Win32Helper
{
	static void DisableMultiInstanceEntry(std::wstring_view const appname, UINT const exitcode);
	static HWND GetHandleFromWindow(winrt::Microsoft::UI::Xaml::Window const& window);
	static void DisableMultiInstanceWindow(winrt::Microsoft::UI::Xaml::Window const& window, std::wstring_view const appname);
};

