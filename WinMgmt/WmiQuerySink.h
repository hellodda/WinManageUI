#pragma once
#include "WmiClassObject.h"

struct WmiQuerySink : winrt::implements<WmiQuerySink, IWbemObjectSink>
{
	winrt::Windows::Foundation::Collections::IVectorView<winrt::WinMgmt::WmiClassObject> Results() noexcept;

	HRESULT STDMETHODCALLTYPE Indicate(LONG lObjectCount, IWbemClassObject** apObjArray) noexcept override;

	HRESULT STDMETHODCALLTYPE SetStatus([[maybe_unused]] LONG lFlags, [[maybe_unused]] HRESULT hResult, [[maybe_unused]] BSTR strParam, [[maybe_unused]] IWbemClassObject* pObjParam) noexcept override;

	winrt::Windows::Foundation::IAsyncAction WaitAsync();

private:
	winrt::handle m_event{ ::CreateEventW(NULL, TRUE, FALSE, NULL) };
	winrt::Windows::Foundation::Collections::IVector<winrt::WinMgmt::WmiClassObject> m_results = winrt::single_threaded_vector<winrt::WinMgmt::WmiClassObject>();
};