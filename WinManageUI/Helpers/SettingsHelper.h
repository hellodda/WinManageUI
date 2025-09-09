#pragma once
struct SettingsHelper
{
	static winrt::Microsoft::UI::Xaml::ElementTheme LoadTheme();

	static void SetTheme(winrt::Microsoft::UI::Xaml::XamlRoot const& element, winrt::Microsoft::UI::Xaml::ElementTheme theme);

	static winrt::hstring GetLangTagName(winrt::hstring const& tag);

private:
	static winrt::Windows::Foundation::Collections::IPropertySet GetApplicationSettings();

	static winrt::Windows::Storage::StorageFolder GetDataFolder();

	static void StoreTheme(winrt::Microsoft::UI::Xaml::ElementTheme theme);

	static const std::wstring_view m_themeKey;
};

