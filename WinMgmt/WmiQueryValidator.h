#pragma once

#include "WmiQueryValidator.g.h"

namespace winrt::WinMgmt::implementation
{
    struct WmiQueryValidator : WmiQueryValidatorT<WmiQueryValidator>
    {
        WmiQueryValidator();

        bool Validate(winrt::hstring const& query);

    private:

        void initialize();

    private:
        winrt::com_ptr<IWbemQuery> m_query;
    };
}

namespace winrt::WinMgmt::factory_implementation
{
    struct WmiQueryValidator : WmiQueryValidatorT<WmiQueryValidator, implementation::WmiQueryValidator>
    {
    };
}
