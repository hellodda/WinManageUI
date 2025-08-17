#pragma once

#include "WmiClassObject.g.h"
#include "WmiClassObjectProperty.h"

namespace winrt::WinMgmt::implementation
{
    struct WmiClassObject : WmiClassObjectT<WmiClassObject>
    {
        WmiClassObject() = default;
        WmiClassObject(IWbemClassObject* pObject);

        Windows::Foundation::Collections::IVectorView<WinMgmt::WmiClassObjectProperty> Properties() const noexcept;
        WinMgmt::WmiClassObjectProperty GetProperty(hstring const& name);

    private:
        winrt::com_ptr<IWbemClassObject> m_object{ nullptr };
    };
}

namespace winrt::WinMgmt::factory_implementation
{
    struct WmiClassObject : WmiClassObjectT<WmiClassObject, implementation::WmiClassObject>
    {
    };
}
