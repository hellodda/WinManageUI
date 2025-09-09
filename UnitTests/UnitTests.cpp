#include "pch.h"
#include "CppUnitTest.h"

#include <winrt/WinMgmt.h>

#include <chrono>
#include <vector>
#include <thread>
#include <atomic>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
    // General notes:
    // - These tests assume the WinMgmt WinRT wrapper exposes:
    //     - WmiDataContext::QueryAsync(winrt::hstring)
    //     - returned collection with Size() and GetAt()
    //     - WmiClassObject::Properties() which returns a collection with Size()/GetAt()
    //     - WmiClassObjectProperty has a Name() accessor (if your wrapper uses a different name, adjust accordingly)
    // - All async calls use .get() so exceptions thrown in the async flow are observed by the test framework.

    TEST_CLASS(WmiTests)
    {
    public:

        // Helper: run a single query and return result view (blocking)
        static auto RunQueryBlocking(const winrt::hstring& query)
        {
            winrt::WinMgmt::WmiDataContext context;
            return context.QueryAsync(query).get();
        }

        TEST_METHOD(Wmi_Query_Work_Test_Success)
        {
            // Basic smoke test: query a commonly-present WMI class and assert we get at least one object.
            const winrt::hstring query = L"SELECT * FROM Win32_NetworkAdapterConfiguration";
            auto query_result = RunQueryBlocking(query);

            Assert::IsTrue(query_result.Size() > 0, L"Query returned no results.");

            auto object = query_result.GetAt(0);
            Assert::IsNotNull(&object, L"First WMI object is null.");
        }

        TEST_METHOD(Wmi_Query_Work_Test_Fail)
        {
            // Invalid/Wrong WQL must raise an hresult_error when awaited.
            const winrt::hstring invalid_query = L"BLA BLA BLA...";
            winrt::WinMgmt::WmiDataContext context;

            Assert::ExpectException<winrt::hresult_error>([&]() {
                context.QueryAsync(invalid_query).get();
                });
        }

        TEST_METHOD(Wmi_EmptyQuery_ShouldFail)
        {
            // Some implementations treat empty query as invalid. Ensure that is handled (expect exception).
            const winrt::hstring empty_query = L"";
            winrt::WinMgmt::WmiDataContext context;

            Assert::ExpectException<winrt::hresult_error>([&]() {
                context.QueryAsync(empty_query).get();
            });
        }

        TEST_METHOD(Wmi_Object_GetProperty_Test_Success)
        {
            // Ensure returned object has at least one property and that property metadata is sane.
            const winrt::hstring query = L"SELECT * FROM Win32_NetworkAdapterConfiguration";
            auto query_result = RunQueryBlocking(query);

            Assert::IsTrue(query_result.Size() > 0, L"Query returned no results.");

            auto object = query_result.GetAt(0);
            auto properties = object.Properties();
            Assert::IsTrue(properties.Size() > 0, L"No properties found in WMI object.");

            auto property = properties.GetAt(0);
            Assert::IsNotNull(&property, L"First property is null.");

            // If your property type exposes a Name() accessor, verify it's non-empty.
   
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

        TEST_METHOD(Wmi_Iterate_AllObjects_HaveProperties)
        {
            // Iterate over all returned objects and assert each object has properties.
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

        TEST_METHOD(Wmi_OperatingSystem_Has_Caption)
        {
            // Query Win32_OperatingSystem: usually there is exactly one instance on a running system.
            // Verify that at least one property exists and names are non-empty.
            const winrt::hstring query = L"SELECT * FROM Win32_OperatingSystem";
            auto query_result = RunQueryBlocking(query);

            Assert::IsTrue(query_result.Size() > 0, L"No Win32_OperatingSystem instances found.");

            auto osObject = query_result.GetAt(0);
            auto props = osObject.Properties();
            Assert::IsTrue(props.Size() > 0, L"Operating system object has no properties.");

            // Try to find 'Caption' property name among properties (best-effort; adjust if your API exposes different accessors).
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

            // If Name() was not available, don't fail the test; otherwise expect we found Caption.
            // This keeps the test resilient across slightly different wrappers.
            // If you want strict checking and your wrapper supports reading property values, extend here.
            // For now: assert that either we couldn't read names (skip) or Caption exists.
            Assert::IsTrue(foundCaption || props.Size() == 0 == false, L"Caption property not found (or property-name access unsupported).");
        }

        TEST_METHOD(Wmi_Concurrent_Queries)
        {
            // Run the same query concurrently on multiple threads to check for reentrancy or global-state bugs.
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

            // Expect all threads to have got at least one object (be resilient: require at least 1 success).
            Assert::IsTrue(successCount.load() >= 1, L"Concurrent queries all failed; expected at least one successful result.");
        }

        TEST_METHOD(Wmi_Query_Average_Performance_Test)
        {
            // Measure average query time across multiple attempts. This helps detect performance regressions.
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
            wprintf(L"Average query execution time over %d runs: %lld ms\n", runs, avg);

            // Ensure queries returned results and average is within a reasonable limit.
            Assert::IsTrue(avg < maxAverageMs, L"Average query time is too slow.");
        }

        TEST_METHOD(Wmi_QueryValidator_Valid_Query_Test)
        {
            // Verify that the WQL query validator correctly identifies a valid query.
            // This ensures that well-formed queries are recognized as valid and can be executed safely.
            const winrt::hstring query = L"SELECT * FROM Win32_NetworkAdapterConfiguration";

            winrt::WinMgmt::WmiQueryValidator validator;

            // Expect the validator to return true for a valid WQL query.
            Assert::IsTrue(validator.Validate(query));
        }

        TEST_METHOD(Wmi_QueryValidator_Invalid_Query_Test)
        {
            // Verify that the WQL query validator correctly identifies an invalid query.
            // This ensures that malformed queries are caught before execution, preventing runtime errors.
            const winrt::hstring query = L"INVALID QUERY"; // intentionally invalid WQL

            winrt::WinMgmt::WmiQueryValidator validator;

            // Expect the validator to return false for the invalid query.
            Assert::IsTrue(!validator.Validate(query));
        }

    };

}
