#ifndef FEATURE_GENERATION_GENERATOR_HPP
#define FEATURE_GENERATION_GENERATOR_HPP

#include <coroutine>

template <typename ENCAPSULATED_TYPE>
struct Couroutine_Generator
{
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;
    struct promise_type
    {
        ENCAPSULATED_TYPE current_value;

        auto get_return_object() { return Couroutine_Generator{coro_handle::from_promise(*this)}; }
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }

        auto yield_value(ENCAPSULATED_TYPE value)
        {
            current_value = value;
            return std::suspend_always{};
        }
    };
    bool move_next() { return coro ? (coro.resume(), !coro.done()) : false; }
    ENCAPSULATED_TYPE current_value() { return coro.promise().current_value; }
    Couroutine_Generator(Couroutine_Generator const &) = delete;
    Couroutine_Generator(Couroutine_Generator &&rhs) : coro(rhs.coro) { rhs.coro = nullptr; }
    ~Couroutine_Generator()
    {
        if (coro)
            coro.destroy();
    }

private:
    Couroutine_Generator(coro_handle h) : coro(h) {}
    coro_handle coro;
};

#endif  // FEATURE_GENERATION_GENERATOR_HPP