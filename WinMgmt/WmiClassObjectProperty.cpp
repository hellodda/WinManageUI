#include "pch.h"
#include "WmiClassObjectProperty.h"
#if __has_include("WmiClassObjectProperty.g.cpp")
#include "WmiClassObjectProperty.g.cpp"
#endif

namespace winrt::WinMgmt::implementation
{
	WmiClassObjectProperty::WmiClassObjectProperty(winrt::hstring name, Windows::Foundation::IInspectable const& value, PropertyType type)
		: m_name(name), m_value(value), m_type(type) {}

	winrt::hstring WmiClassObjectProperty::Name() const noexcept
	{
		return m_name;
	}

	winrt::Windows::Foundation::IInspectable WmiClassObjectProperty::Value() const noexcept
	{
		return m_value;
	}

	PropertyType WmiClassObjectProperty::Type() const noexcept
	{
		return m_type;
	}
}
