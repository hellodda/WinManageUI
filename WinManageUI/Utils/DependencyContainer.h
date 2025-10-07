#pragma once

#include <typeindex>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <functional>
#include <string>
#include <tuple>
#include <utility>
#include <stdexcept>
#include <type_traits>
#include <variant>
#include <optional>
#include <concepts>

template<typename T>
concept WinRTProjection = std::is_convertible_v<T, winrt::Windows::Foundation::IInspectable>;

template<typename T>
concept NativeService = !WinRTProjection<T>;

template<typename F, typename Interface>
concept NativeFactory = std::invocable<F> && std::convertible_to<std::invoke_result_t<F>, std::shared_ptr<std::decay_t<Interface>>>;

template<typename F>
concept WinRTFactory = std::invocable<F> && std::convertible_to<std::invoke_result_t<F>, winrt::Windows::Foundation::IInspectable>;

enum class Lifetime { Singleton, Transient };

namespace Containers {

    class DependencyContainer {
    public:

        DependencyContainer() = default;
        DependencyContainer(std::nullptr_t) {}
        DependencyContainer(const DependencyContainer&) = delete;
        DependencyContainer& operator=(const DependencyContainer&) = delete;
        DependencyContainer(DependencyContainer&&) = default;
        DependencyContainer& operator=(DependencyContainer&&) = default;

        template<NativeService T>
        void RegisterInstance(Lifetime life = Lifetime::Singleton, std::string const& name = {}) {
            Key k{ std::type_index(typeid(std::decay_t<T>)), name };
            auto e = std::make_shared<Entry>();
            e->type = Entry::Type::Native;

            if (life == Lifetime::Singleton)
            {
                e->instance = std::static_pointer_cast<void>(std::make_shared<T>());
            }
            else
            {
                e->factory = []() -> std::shared_ptr<void>
                {
                    return std::make_shared<T>();
                };
            }

            std::lock_guard<std::mutex> lk(m_mapMutex);
            m_map[k] = std::move(e);
        }

        template<WinRTProjection T>
        void RegisterInstance(Lifetime life = Lifetime::Singleton, std::string const& name = {}) {
            Key k{ std::type_index(typeid(std::decay_t<T>)), name };
            auto e = std::make_shared<Entry>();
            e->type = Entry::Type::WinRT;

            if (life == Lifetime::Singleton)
            {
                e->instance = winrt::Windows::Foundation::IInspectable{ T{} };
            }
            else
            {
                e->factory = []() -> winrt::Windows::Foundation::IInspectable
                {
                    return winrt::Windows::Foundation::IInspectable{ T{} };
                };
            }
            std::lock_guard<std::mutex> lk(m_mapMutex);
            m_map[k] = std::move(e);
        }

        template<NativeService Interface, NativeFactory F>
        void RegisterInstance(F&& factory, Lifetime life = Lifetime::Transient, std::string const& name = {})
        {
            Key k{ std::type_index(typeid(std::decay_t<Interface>)), std::move(name) };
            auto e = std::make_shared<Entry>();
            e->type = Entry::Type::Native;

            if (life == Lifetime::Singleton)
            {
                auto inst = std::invoke(std::forward<F>(factory));
                e->instance = std::static_pointer_cast<void>(inst);
            }
            else
            {
                e->factory = [f = std::forward<F>(factory)]() -> std::shared_ptr<void> {
                    return std::static_pointer_cast<void>(std::invoke(f));
                };
            }

            std::lock_guard<std::mutex> lk(m_mapMutex);
            m_map[k] = std::move(e);
        }

        template<WinRTProjection Interface, WinRTFactory F>
        void RegisterInstance(F&& factory, Lifetime life = Lifetime::Transient, std::string const& name = {})
        {
            Key k{ std::type_index(typeid(std::decay_t<Interface>)), std::move(name) };
            auto e = std::make_shared<Entry>();
            e->type = Entry::Type::WinRT;

            if (life == Lifetime::Singleton)
            {
                auto inst = std::invoke(std::forward<F>(factory)); 
                e->instance = winrt::Windows::Foundation::IInspectable{ inst };
            }
            else
            {
                e->factory = [f = std::forward<F>(factory)]() -> winrt::Windows::Foundation::IInspectable {
                    return winrt::Windows::Foundation::IInspectable{ std::invoke(f) };
                };
            }

            std::lock_guard<std::mutex> lk(m_mapMutex);
            m_map[k] = std::move(e);
        }

        template<WinRTProjection T>
        T Resolve(std::string const& name = {}) {
            auto entry = GetEntry(typeid(std::decay_t<T>), name);
            if (!entry) throw std::runtime_error(std::string("Service not registered: ") + typeid(T).name() + (name.empty() ? "" : "@" + name));
            if (entry->type != Entry::Type::WinRT) throw std::runtime_error("Requested WinRT resolution but entry is Native kind");

            if (auto p = std::get_if<winrt::Windows::Foundation::IInspectable>(&entry->instance)) {
                return p->as<T>();
            }

            if (std::holds_alternative<std::function<winrt::Windows::Foundation::IInspectable()>>(entry->factory))
                return std::get<std::function<winrt::Windows::Foundation::IInspectable()>>(entry->factory)().as<T>();

            throw std::runtime_error("Stored entry does not contain WinRT instance");
        }

        template<WinRTProjection T>
        std::optional<T> TryResolve(std::string const& name = {}) noexcept {
            try { return Resolve<T>(name); }
            catch (...) { return std::nullopt; }
        }

        template<NativeService T>
        std::shared_ptr<std::decay_t<T>> Resolve(std::string const& name = {}) {
            auto entry = GetEntry(typeid(std::decay_t<T>), name);
            if (!entry) throw std::runtime_error(std::string("Service not registered: ") + typeid(T).name() + (name.empty() ? "" : "@" + name));
            if (entry->type != Entry::Type::Native) throw std::runtime_error("Requested native resolution but entry is WinRT kind");

            if (auto p = std::get_if<std::shared_ptr<void>>(&entry->instance)) {
                return std::static_pointer_cast<std::decay_t<T>>(*p);
            }

            if (std::holds_alternative<std::function<std::shared_ptr<void>()>>(entry->factory))
                return std::static_pointer_cast<std::decay_t<T>>(std::get<std::function<std::shared_ptr<void>()>>(entry->factory)());

            throw std::runtime_error("Stored entry does not contain native instance");
        }

        template<NativeService T>
        std::optional<std::shared_ptr<std::decay_t<T>>> TryResolve(std::string const& name = {}) noexcept {
            try { return Resolve<T>(name); }
            catch (...) { return std::nullopt; }
        }

        template<typename T>
        bool Remove(std::string const& name = {}) {
            Key k{ std::type_index(typeid(std::decay_t<T>)), name };
            std::lock_guard<std::mutex> lk(m_mapMutex);
            return m_map.erase(k) > 0;
        }

        void Clear() {
            std::lock_guard<std::mutex> lk(m_mapMutex);
            m_map.clear();
        }

    private:


        using InstanceVariant = std::variant<std::shared_ptr<void>, winrt::Windows::Foundation::IInspectable>;
        using FactoryVariant = std::variant<std::function<std::shared_ptr<void>()>, std::function<winrt::Windows::Foundation::IInspectable()>>;

        struct Key {
            std::type_index type;
            std::string name;
            bool operator==(Key const& o) const noexcept { return type == o.type && name == o.name; }
        };

        struct KeyHash {
            std::size_t operator()(Key const& k) const noexcept {
                auto h1 = k.type.hash_code();
                auto h2 = std::hash<std::string>{}(k.name);
                return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
            }
        };

        struct Entry {
            enum class Type { Native, WinRT } type = Type::Native;
            InstanceVariant instance;
            FactoryVariant factory;
        };

        std::shared_ptr<Entry> GetEntry(std::type_index t, std::string const& name) {
            Key k{ std::move(t), name };
            std::lock_guard<std::mutex> lk(m_mapMutex);
            auto it = m_map.find(k);
            if (it == m_map.end()) return nullptr;
            return it->second;
        }

        mutable std::mutex m_mapMutex;
        std::unordered_map<Key, std::shared_ptr<Entry>, KeyHash> m_map;
    };

} 
