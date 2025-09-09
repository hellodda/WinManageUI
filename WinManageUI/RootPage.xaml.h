#pragma once

#include "RootPage.g.h"

namespace winrt::WinManageUI::implementation
{
    struct RootPage : RootPageT<RootPage>
    {
        RootPage() = default;
    };
}

namespace winrt::WinManageUI::factory_implementation
{
    struct RootPage : RootPageT<RootPage, implementation::RootPage>
    {
    };
}
