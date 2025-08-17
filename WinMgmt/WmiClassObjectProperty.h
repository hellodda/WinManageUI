#pragma once

#include "WmiClassObjectProperty.g.h"

namespace winrt::WinMgmt::implementation
{
    struct WmiClassObjectProperty : WmiClassObjectPropertyT<WmiClassObjectProperty>
    {
        WmiClassObjectProperty() = default;
        WmiClassObjectProperty(winrt::hstring name, winrt::Windows::Foundation::IInspectable const& value, PropertyType type);

        winrt::hstring Name() const noexcept;

        Windows::Foundation::IInspectable Value() const noexcept;
        
        PropertyType Type() const noexcept;

    private:
        winrt::hstring m_name;
        PropertyType m_type;
        Windows::Foundation::IInspectable m_value{ nullptr };
    };
}

namespace winrt::WinMgmt::factory_implementation
{
    struct WmiClassObjectProperty : WmiClassObjectPropertyT<WmiClassObjectProperty, implementation::WmiClassObjectProperty>
    {
    };
}
