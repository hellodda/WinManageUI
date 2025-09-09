#include "pch.h"
#include "WmiQuerySink.h"

[[nodiscard]] winrt::Windows::Foundation::Collections::IVectorView<winrt::WinMgmt::WmiClassObject> WmiQuerySink::Results() noexcept
{
    return m_results.GetView();
}

HRESULT STDMETHODCALLTYPE WmiQuerySink::Indicate(LONG lObjectCount, IWbemClassObject** apObjArray) noexcept
{
    if (!apObjArray) [[unlikely]]
        return E_POINTER;

    for (LONG i = 0; i < lObjectCount; i++) [[likely]]
    {
        m_results.Append(winrt::make<winrt::WinMgmt::implementation::WmiClassObject>(apObjArray[i]));
    }
    return WBEM_S_NO_ERROR;
}

HRESULT STDMETHODCALLTYPE WmiQuerySink::SetStatus(LONG lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject* pObjParam) noexcept
{
    return ::SetEvent(m_event.get()) ? WBEM_S_NO_ERROR : WBEM_E_FAILED;
}

winrt::Windows::Foundation::IAsyncAction WmiQuerySink::WaitAsync()
{
    if (!m_event) [[unlikely]]
        throw winrt::hresult_error(E_POINTER, L"Event not initialized");

    co_await winrt::resume_on_signal(m_event.get());
}
