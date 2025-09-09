#pragma once
#include "WmiClassObjectProperty.h"

struct PropertyParser
{
	static winrt::WinMgmt::WmiClassObjectProperty CreateFromVartype(_bstr_t const& name, _variant_t const& var);
};

