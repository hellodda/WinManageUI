#pragma once

#include "WmiDataContext.g.h"
#include "WmiClassObject.h"

namespace winrt::WinMgmt::implementation
{
    struct WmiDataContext : WmiDataContextT<WmiDataContext>
    {
        WmiDataContext();

        hstring Namespace() const noexcept;

        void Namespace(hstring const& value);

        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVectorView<winrt::WinMgmt::WmiClassObject>> QueryAsync(hstring const& query);

    private:

        void initialize();
        
    private:
        winrt::com_ptr<IWbemServices> m_services{ nullptr };
        hstring m_namespace{ L"ROOT\\CIMV2" };
    };
}

namespace winrt::WinMgmt::factory_implementation
{
    struct WmiDataContext : WmiDataContextT<WmiDataContext, implementation::WmiDataContext>
    {
    };
}
