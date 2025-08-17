#include "pch.h"
#include "WmiClassObject.h"
#if __has_include("WmiClassObject.g.cpp")
#include "WmiClassObject.g.cpp"
#endif

#include "Utils/PropertyParser.h"

namespace winrt::WinMgmt::implementation
{
    WmiClassObject::WmiClassObject(IWbemClassObject* pObject)
    {
        m_object.copy_from(pObject);
    }
    Windows::Foundation::Collections::IVectorView<WinMgmt::WmiClassObjectProperty> WmiClassObject::Properties() const noexcept
    {
        winrt::check_hresult(m_object->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY));

        _bstr_t name;
        _variant_t var;

        auto props = single_threaded_vector<WinMgmt::WmiClassObjectProperty>();

        struct scope_exit
        {
            IWbemClassObject* obj;
            ~scope_exit() { if (obj) obj->EndEnumeration(); }
        } guard{ m_object.get() };

        while (true)
        {
            HRESULT hr = m_object->Next(0, &name.GetBSTR(), &var, nullptr, nullptr);
            if (hr == WBEM_S_NO_ERROR)
            {
                props.Append(PropertyParser::CreateFromVartype(name, var));
            }
            else
            {
                break;
            }
        }
        return props.GetView();
    }

    WinMgmt::WmiClassObjectProperty WmiClassObject::GetProperty(hstring const& name)
    {
        _variant_t var;
        winrt::check_hresult(m_object->Get(name.c_str(), 0, &var, nullptr, nullptr));

        return PropertyParser::CreateFromVartype(name.c_str(), var);
    }
}
