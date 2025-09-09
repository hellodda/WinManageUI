#include "pch.h"
#include "SettingsHelper.h"

const std::wstring_view SettingsHelper::m_themeKey = L"Theme";

winrt::Microsoft::UI::Xaml::ElementTheme SettingsHelper::LoadTheme()
{
	auto settings{ GetApplicationSettings() };
	auto value{ winrt::unbox_value_or<int32_t>(settings.Lookup(m_themeKey.data()), 0) };
	return winrt::Microsoft::UI::Xaml::ElementTheme{ value };
}

void SettingsHelper::SetTheme(winrt::Microsoft::UI::Xaml::XamlRoot const& element, winrt::Microsoft::UI::Xaml::ElementTheme theme)
{
	element.Content().try_as<winrt::Microsoft::UI::Xaml::FrameworkElement>().RequestedTheme(theme);
	StoreTheme(theme);
}

winrt::hstring SettingsHelper::GetLangTagName(winrt::hstring const& tag)
{
	if (tag.starts_with(L"en")) [[likely]]
		return { L"English" };
	else if (tag.starts_with(L"ru"))
		return { L"Russian" };

	assert(false);
	std::unreachable();
}

winrt::Windows::Foundation::Collections::IPropertySet SettingsHelper::GetApplicationSettings()
{
	return winrt::Windows::Storage::ApplicationData::Current().LocalSettings().Values();
}

winrt::Windows::Storage::StorageFolder SettingsHelper::GetDataFolder()
{
	return winrt::Windows::Storage::ApplicationData::Current().LocalFolder();
}

void SettingsHelper::StoreTheme(winrt::Microsoft::UI::Xaml::ElementTheme theme)
{
	auto settings{ GetApplicationSettings() };
	auto value{ static_cast<int32_t>(theme) };
	settings.Insert(m_themeKey, winrt::box_value(value));
}

