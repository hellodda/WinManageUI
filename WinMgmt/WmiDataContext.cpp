#include "pch.h"
#include "WmiDataContext.h"
#if __has_include("WmiDataContext.g.cpp")
#include "WmiDataContext.g.cpp"
#endif

#include "WmiQuerySink.h"

namespace winrt::WinMgmt::implementation
{
    WmiDataContext::WmiDataContext()
    {
        initialize();
    }

    void WmiDataContext::initialize()
    {
        winrt::com_ptr<IWbemLocator> locator;
        winrt::check_hresult(CoCreateInstance(
            CLSID_WbemLocator,
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(IWbemLocator),
            locator.put_void()
        ));

        winrt::check_hresult(locator->ConnectServer(
            _bstr_t(m_namespace.c_str()),
            NULL,
            NULL,
            0,
            NULL,
            0,
            0,
            m_services.put()
        ));

        winrt::check_hresult(CoSetProxyBlanket(
            m_services.get(),
            RPC_C_AUTHN_WINNT,
            RPC_C_AUTHZ_NONE,
            NULL,
            RPC_C_AUTHN_LEVEL_CALL,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,
            EOAC_NONE
        ));
    }

    void WmiDataContext::Namespace(hstring const& value)
    {
        if (value != m_namespace) [[likely]]
        {
            m_namespace = value;
        }
    }

    [[nodiscard]] hstring WmiDataContext::Namespace() const noexcept
    {
        return m_namespace;
    }

    [[nodiscard]] winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVectorView<winrt::WinMgmt::WmiClassObject>>WmiDataContext::QueryAsync(hstring const& query)
    {
        if (!m_services) [[unlikely]]
            throw winrt::hresult_error(E_POINTER, L"data context services is null!");

        auto sink = winrt::make_self<WmiQuerySink>();
        winrt::check_hresult(m_services->ExecQueryAsync(
            _bstr_t(L"WQL"),
            _bstr_t(query.c_str()),
            0,
            NULL,
            sink.get()
        ));

        co_await sink->WaitAsync();

        co_return sink->Results();
    }
}
