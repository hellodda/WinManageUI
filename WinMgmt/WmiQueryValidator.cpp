#include "pch.h"
#include "WmiQueryValidator.h"
#if __has_include("WmiQueryValidator.g.cpp")
#include "WmiQueryValidator.g.cpp"
#endif

namespace winrt::WinMgmt::implementation
{
	WmiQueryValidator::WmiQueryValidator()
	{
		initialize();
	}

	void WmiQueryValidator::initialize()
	{
		winrt::check_hresult(CoCreateInstance(
			CLSID_WbemQuery,
			NULL,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWbemQuery),
			m_query.put_void()
		));
	}

	bool WmiQueryValidator::Validate(winrt::hstring const& query)
	{
		return SUCCEEDED(m_query->Parse(L"WQL", query.c_str(), 0));
	}
}
