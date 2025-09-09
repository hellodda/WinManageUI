#include "pch.h"
#include "PropertyParser.h"

winrt::WinMgmt::WmiClassObjectProperty PropertyParser::CreateFromVartype(_bstr_t const& name, _variant_t const& var)
{
    using namespace winrt;

    switch (var.vt)
    {
    case VT_I1:
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, box_value(var.bVal), WinMgmt::PropertyType::Int8 };

    case VT_I2: // short / Int16
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, box_value(var.iVal), WinMgmt::PropertyType::Int16 };

    case VT_I4: // int / Int32
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, box_value(var.intVal), WinMgmt::PropertyType::Int32 };

    case VT_I8: // INT64
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, box_value(var.llVal), WinMgmt::PropertyType::Int64 };

    case VT_UI1:
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, box_value(var.bVal), WinMgmt::PropertyType::UInt8 };

    case VT_UI2: // UInt16
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, box_value(var.uiVal), WinMgmt::PropertyType::UInt16 };

    case VT_UI4: // UInt32
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, box_value(var.uintVal), WinMgmt::PropertyType::UInt32 };

    case VT_UI8: // UInt64
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, box_value(var.ullVal), WinMgmt::PropertyType::UInt64 };

    case VT_R4: // float
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, box_value(var.fltVal), WinMgmt::PropertyType::Float };

    case VT_R8: // double
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, box_value(var.dblVal), WinMgmt::PropertyType::Double };

    case VT_BOOL:
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, box_value(var.boolVal == VARIANT_TRUE), WinMgmt::PropertyType::Boolean };

    case VT_BSTR:
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, box_value(hstring{var.bstrVal}), WinMgmt::PropertyType::String };

    case VT_EMPTY:
    case VT_NULL:
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, nullptr, WinMgmt::PropertyType::Null };

    default:
        return WinMgmt::WmiClassObjectProperty{ hstring{name}, nullptr, WinMgmt::PropertyType::Unknown };
    }
}
