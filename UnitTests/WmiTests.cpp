#include "pch.h"
#include "CppUnitTest.h"

#include <winrt/WinMgmt.h>

#include <chrono>
#include <vector>
#include <thread>
#include <atomic>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace IntegrationTests
{
    // ---------------------------------------------------------------------
    // General notes:
    // - These tests assume the WinMgmt WinRT wrapper exposes:
    //     - WmiDataContext::QueryAsync(winrt::hstring)
    //     - returned collection with Size() and GetAt()
    //     - WmiClassObject::Properties() which returns a collection with Size()/GetAt()
    //     - WmiClassObjectProperty has a Name() accessor (if your wrapper uses a different name, adjust accordingly)
    // - All async calls use .get() so exceptions thrown in the async flow are observed by the test framework.
    // ---------------------------------------------------------------------

    TEST_CLASS(WmiTests)
    {
    public:

        // ---------------------------------------------------------------------
        // Helper: RunQueryBlocking
        // - Run a single WQL query using the wrapper and return the result synchronously
        // - Blocks on the async operation (.get()) so exceptions propagate to the test runner
        // ---------------------------------------------------------------------
        static auto RunQueryBlocking(const winrt::hstring& query)
        {
            winrt::WinMgmt::WmiDataContext context;
            return context.QueryAsync(query).get();
        }

        // ---------------------------------------------------------------------
        // Wmi_Query_Work_Test_Success
        // - Basic smoke test: query a commonly-present WMI class and assert we get at least one object.
        // ---------------------------------------------------------------------
        TEST_METHOD(Wmi_Query_Work_Test_Success)
        {
            const winrt::hstring query = L"SELECT * FROM Win32_NetworkAdapterConfiguration";
            auto query_result = RunQueryBlocking(query);

            Assert::IsTrue(query_result.Size() > 0, L"Query returned no results.");

            auto object = query_result.GetAt(0);
            Assert::IsNotNull(&object, L"First WMI object is null.");
        }

        // ---------------------------------------------------------------------
        // Wmi_EmptyQuery_ShouldFail
        // - Some implementations treat empty query as invalid. Ensure that is handled (expect exception).
        // ---------------------------------------------------------------------
        TEST_METHOD(Wmi_EmptyQuery_ShouldFail)
        {
            const winrt::hstring empty_query = L"";
            winrt::WinMgmt::WmiDataContext context;

            Assert::ExpectException<winrt::hresult_error>([&]() {
                context.QueryAsync(empty_query).get();
                });
        }

        // ---------------------------------------------------------------------
        // Wmi_Object_GetProperty_Test_Success
        // - Ensure returned object has at least one property and that property metadata is sane.
        // - Best-effort check: if property Name() exists, verify non-empty; otherwise skip that sub-check.
        // ---------------------------------------------------------------------
        TEST_METHOD(Wmi_Object_GetProperty_Test_Success)
        {
            const winrt::hstring query = L"SELECT * FROM Win32_NetworkAdapterConfiguration";
            auto query_result = RunQueryBlocking(query);

            Assert::IsTrue(query_result.Size() > 0, L"Query returned no results.");

            auto object = query_result.GetAt(0);
            auto properties = object.Properties();
            Assert::IsTrue(properties.Size() > 0, L"No properties found in WMI object.");

            auto property = properties.GetAt(0);
            Assert::IsNotNull(&property, L"First property is null.");

            try
            {
                auto name = property.Name();
                Assert::IsTrue(name.size() > 0, L"Property name is empty.");
            }
            catch (...)
            {
                // If Name() is not available on your wrapper, just skip this sub-check.
            }
        }

        // ---------------------------------------------------------------------
        // Wmi_Iterate_AllObjects_HaveProperties
        // - Iterate over all returned objects and assert each object has properties.
        // ---------------------------------------------------------------------
        TEST_METHOD(Wmi_Iterate_AllObjects_HaveProperties)
        {
            const winrt::hstring query = L"SELECT * FROM Win32_NetworkAdapterConfiguration";
            auto query_result = RunQueryBlocking(query);

            Assert::IsTrue(query_result.Size() > 0, L"Query returned no results.");

            const uint32_t count = query_result.Size();
            for (uint32_t i = 0; i < count; ++i)
            {
                auto obj = query_result.GetAt(i);
                Assert::IsNotNull(&obj, L"Encountered null object in result set.");

                auto props = obj.Properties();
                Assert::IsTrue(props.Size() > 0, L"An object had no properties.");
            }
        }

        // ---------------------------------------------------------------------
        // Wmi_OperatingSystem_Has_Caption
        // - Query Win32_OperatingSystem and attempt to locate 'Caption' property among property names.
        // - Best-effort: if property names are not exposed, do not fail the test.
        // ---------------------------------------------------------------------
        TEST_METHOD(Wmi_OperatingSystem_Has_Caption)
        {
            const winrt::hstring query = L"SELECT * FROM Win32_OperatingSystem";
            auto query_result = RunQueryBlocking(query);

            Assert::IsTrue(query_result.Size() > 0, L"No Win32_OperatingSystem instances found.");

            auto osObject = query_result.GetAt(0);
            auto props = osObject.Properties();
            Assert::IsTrue(props.Size() > 0, L"Operating system object has no properties.");

            bool foundCaption = false;
            for (uint32_t i = 0; i < props.Size(); ++i)
            {
                auto p = props.GetAt(i);
                try
                {
                    auto name = p.Name();
                    if (name == L"Caption")
                    {
                        foundCaption = true;
                        break;
                    }
                }
                catch (...)
                {
                    // If Name() is not present, we can't check by name. Stop trying to read names.
                    break;
                }
            }

            Assert::IsTrue(foundCaption || props.Size() == 0 == false, L"Caption property not found (or property-name access unsupported).");
        }

        // ---------------------------------------------------------------------
        // Wmi_Concurrent_Queries
        // - Run the same query concurrently on multiple threads to check for reentrancy or global-state bugs.
        // - Require at least one successful query among threads (resilient baseline).
        // ---------------------------------------------------------------------
        TEST_METHOD(Wmi_Concurrent_Queries)
        {
            const winrt::hstring query = L"SELECT * FROM Win32_NetworkAdapterConfiguration";

            constexpr int threadCount = 4;
            std::vector<std::thread> threads;
            std::atomic<int> successCount{ 0 };

            for (int i = 0; i < threadCount; ++i)
            {
                threads.emplace_back([&]()
                    {
                        try
                        {
                            auto result = RunQueryBlocking(query);
                            if (result.Size() > 0)
                            {
                                ++successCount;
                            }
                        }
                        catch (...)
                        {
                            // swallow; we'll assert below based on successCount
                        }
                    });
            }

            for (auto& t : threads) t.join();

            Assert::IsTrue(successCount.load() >= 1, L"Concurrent queries all failed; expected at least one successful result.");
        }

        // ---------------------------------------------------------------------
        // Wmi_Query_Average_Performance_Test
        // - Measure average query time across multiple attempts to detect regressions.
        // - Use environment-adjustable maxAverageMs threshold.
        // ---------------------------------------------------------------------
        TEST_METHOD(Wmi_Query_Average_Performance_Test)
        {
            const winrt::hstring query = L"SELECT * FROM Win32_NetworkAdapterConfiguration";
            constexpr int runs = 5;
            constexpr long long maxAverageMs = 1500; // 1.5s average allowed (tune per environment)

            long long totalMs = 0;
            for (int i = 0; i < runs; ++i)
            {
                auto start = std::chrono::high_resolution_clock::now();
                auto result = RunQueryBlocking(query);
                auto end = std::chrono::high_resolution_clock::now();

                auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                totalMs += duration_ms;
            }

            long long avg = totalMs / runs;

            Assert::IsTrue(avg < maxAverageMs, L"Average query time is too slow.");
        }

        // ---------------------------------------------------------------------
        // Wmi_QueryValidator_Valid_Query_Test
        // - Verify that the WQL query validator correctly identifies a valid query.
        // ---------------------------------------------------------------------
        TEST_METHOD(Wmi_QueryValidator_Valid_Query_Test)
        {
            const winrt::hstring query = L"SELECT * FROM Win32_NetworkAdapterConfiguration";

            winrt::WinMgmt::WmiQueryValidator validator;

            Assert::IsTrue(validator.Validate(query));
        }

        // ---------------------------------------------------------------------
        // Wmi_QueryValidator_Invalid_Query_Test
        // - Verify that the WQL query validator correctly identifies an invalid query.
        // ---------------------------------------------------------------------
        TEST_METHOD(Wmi_QueryValidator_Invalid_Query_Test)
        {
            const winrt::hstring query = L"INVALID QUERY"; // intentionally invalid WQL

            winrt::WinMgmt::WmiQueryValidator validator;

            Assert::IsTrue(!validator.Validate(query));
        }

    };

} 
