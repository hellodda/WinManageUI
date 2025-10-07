#include "pch.h"
#include "CppUnitTest.h"
#include "../WinManageUI/Utils/DependencyContainer.h"

#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <future>
#include <sstream>
#include <stdexcept>
#include <memory>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
    // Simple interface/implementations used by the tests
    struct IEmpty { virtual ~IEmpty() = default; virtual int Value() const = 0; };

    struct ImplDefault : IEmpty {
        ImplDefault() : v(1) {}
        int Value() const override { return v; }
        int v;
    };

    struct ImplWithArgs : IEmpty {
        ImplWithArgs(int a, std::string s) : a(a), s(std::move(s)) {}
        int Value() const override { return a + static_cast<int>(s.size()); }
        int a; std::string s;
    };

    struct ImplCounter : IEmpty {
        static std::atomic<int> instances;
        ImplCounter() { ++instances; }
        ~ImplCounter() { --instances; }
        int Value() const override { return 42; }
    };
    std::atomic<int> ImplCounter::instances{ 0 };

    struct ImplThrows : IEmpty {
        ImplThrows() { throw std::runtime_error("ctor fail"); }
        int Value() const override { return 0; }
    };

    // Helper: small delay to increase the chance of races in multithreaded tests
    static void tiny_sleep_ms(unsigned ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

} 

using namespace UnitTests;

namespace UnitTests
{
    TEST_CLASS(DependencyContainerTests)
    {
    public:

        // ---------------------------------------------------------------------
        // Register_DefaultInstance_And_TryResolve
        // - Register an instance created by the container via default ctor
        // - Ensure TryResolve returns a non-empty optional and Value() is correct
        // ---------------------------------------------------------------------
        TEST_METHOD(Register_DefaultInstance_And_TryResolve)
        {
            Containers::DependencyContainer c{ nullptr };
            c.RegisterInstance<ImplDefault>(); // default-constructed singleton

            auto opt = c.TryResolve<ImplDefault>();
            Assert::IsTrue(opt.has_value());
            Assert::IsNotNull(opt->get());
        }

        // ---------------------------------------------------------------------
        // NamedRegistrations_AreIndependent
        // - Two named registrations of different concrete types must not interfere
        // ---------------------------------------------------------------------
        TEST_METHOD(NamedRegistrations_AreIndependent)
        {
            Containers::DependencyContainer c{ nullptr };
            c.RegisterInstance<ImplDefault>("one");
            c.RegisterInstance<ImplCounter>("two");

            auto a = c.TryResolve<ImplDefault>("one");
            auto b = c.TryResolve<ImplCounter>("two");

            Assert::IsTrue(a.has_value());
            Assert::IsTrue(b.has_value());
        }

        // ---------------------------------------------------------------------
        // RemoveRegistration_MakesResolveFail
        // - Register, verify, remove, then ensure TryResolve returns empty optional
        // ---------------------------------------------------------------------
        TEST_METHOD(RemoveRegistration_MakesResolveFail)
        {
            Containers::DependencyContainer c{ nullptr };
            c.RegisterInstance<ImplDefault>("rm");

            auto sp = c.TryResolve<ImplDefault>("rm");
            Assert::IsTrue(sp.has_value());

            bool erased = c.Remove<ImplDefault>("rm");
            Assert::IsTrue(erased);

            auto maybe = c.TryResolve<ImplDefault>("rm");
            Assert::IsFalse(maybe.has_value());
        }

        // ---------------------------------------------------------------------
        // Clear_Removes_All
        // - Register several services, call Clear, ensure none remain
        // ---------------------------------------------------------------------
        TEST_METHOD(Clear_Removes_All)
        {
            Containers::DependencyContainer c{ nullptr };
            c.RegisterInstance<ImplDefault>("a");
            c.RegisterInstance<ImplDefault>("b");

            c.Clear();

            Assert::IsFalse(c.TryResolve<ImplDefault>("a").has_value());
            Assert::IsFalse(c.TryResolve<ImplDefault>("b").has_value());
        }

        // ---------------------------------------------------------------------
        // Resolve_Throws_For_Missing
        // - Resolve should throw when a requested service is not registered
        // ---------------------------------------------------------------------
        TEST_METHOD(Resolve_Throws_For_Missing)
        {
            Containers::DependencyContainer c{ nullptr };
            auto lambda = [&]() { c.Resolve<ImplDefault>("missing"); };
            Assert::ExpectException<std::runtime_error>(lambda);
        }

        // ---------------------------------------------------------------------
        // TryResolve_Returns_Empty_For_Missing
        // - TryResolve should return empty optional when the registration is absent
        // ---------------------------------------------------------------------
        TEST_METHOD(TryResolve_Returns_Empty_For_Missing)
        {
            Containers::DependencyContainer c{ nullptr };
            auto opt = c.TryResolve<ImplDefault>("nope");
            Assert::IsFalse(opt.has_value());
        }

        // ---------------------------------------------------------------------
        // RegisterInstance_ThatThrows_BubblesDuringRegistration
        // - If the constructor throws, RegisterInstance should propagate the exception
        // ---------------------------------------------------------------------
        TEST_METHOD(RegisterInstance_ThatThrows_BubblesDuringRegistration)
        {
            Containers::DependencyContainer c{ nullptr };
            auto lambda = [&]() { c.RegisterInstance<ImplThrows>(); };
            Assert::ExpectException<std::runtime_error>(lambda);
        }

    };
} 
