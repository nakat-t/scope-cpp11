// MIT License
// 
// Copyright (c) 2019 nakat-t <armaiti.wizard@gmail.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#ifndef NAKATT_SCOPE_HPP_
#define NAKATT_SCOPE_HPP_

#include <cstddef>
#include <exception>
#include <type_traits>

#define SCOPE_VERSION_MAJOR 0
#define SCOPE_VERSION_MINOR 5
#define SCOPE_VERSION_PATCH 0

#define SCOPE_STR(v) #v
#define SCOPE_VERSION_(major, minor, patch) SCOPE_STR(major) "." SCOPE_STR(minor) "." SCOPE_STR(patch)
#define SCOPE_VERSION SCOPE_VERSION_(SCOPE_VERSION_MAJOR, SCOPE_VERSION_MINOR, SCOPE_VERSION_PATCH)

#if defined(__cpp_lib_uncaught_exceptions)
#   define SCOPE_USE_SUCCESS_FAIL
#endif

#if defined(__cpp_deduction_guides)
    #define SCOPE_USE_DEDUCTION_GUIDE
#endif

namespace scope {

namespace detail {

template<class T>
using remove_cvref_t = typename std::remove_cv<std::remove_reference<T>>::type;

template<bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

struct strategy_exit
{
    constexpr bool call_when_dtor() const noexcept { return true; }
    constexpr bool call_when_construct_failed() const noexcept { return true; }
};

#if defined(SCOPE_USE_SUCCESS_FAIL)
struct strategy_success
{
    int uncaught_on_creation_{std::uncaught_exceptions()};
    bool           call_when_dtor() const noexcept { return std::uncaught_exceptions() <= uncaught_on_creation_; }
    constexpr bool call_when_construct_failed() const noexcept { return false; }
};

struct strategy_fail
{
    int uncaught_on_creation_{std::uncaught_exceptions()};
    bool           call_when_dtor() const noexcept { return std::uncaught_exceptions() > uncaught_on_creation_; }
    constexpr bool call_when_construct_failed() const noexcept { return true; }
};
#endif // defined(SCOPE_USE_SUCCESS_FAIL)

template <typename EF, typename Strategy>
struct is_dtor_noexcept_t : public std::true_type {};

#if defined(SCOPE_USE_SUCCESS_FAIL)
template <typename EF>
struct is_dtor_noexcept_t<EF, strategy_success>
    : public std::conditional<noexcept(std::declval<EF>()()), std::true_type, std::false_type>::type
{};
#endif // defined(SCOPE_USE_SUCCESS_FAIL)

template <typename EF, typename Strategy>
class scope_guard
{
public:
    template <
        typename EFP,
        enable_if_t<std::is_constructible<EF, EFP>::value, std::nullptr_t> = nullptr,
        enable_if_t<(!std::is_lvalue_reference<EFP>::value && std::is_nothrow_constructible<EF, EFP>::value), std::nullptr_t> = nullptr
    >
    explicit scope_guard(EFP&& f) noexcept
        : exit_function_{std::forward<EFP>(f)}
    {}

    template <
        typename EFP,
        enable_if_t<std::is_constructible<EF, EFP>::value, std::nullptr_t> = nullptr,
        enable_if_t<!(!std::is_lvalue_reference<EFP>::value && std::is_nothrow_constructible<EF, EFP>::value), std::nullptr_t> = nullptr,
        enable_if_t<std::is_nothrow_constructible<EF, EFP>::value || std::is_nothrow_constructible<EF, EFP&>::value, std::nullptr_t> = nullptr
    >
    explicit scope_guard(EFP&& f) noexcept
        : exit_function_{f}
    {}

    template <
        typename EFP,
        enable_if_t<std::is_constructible<EF, EFP>::value, std::nullptr_t> = nullptr,
        enable_if_t<!(!std::is_lvalue_reference<EFP>::value && std::is_nothrow_constructible<EF, EFP>::value), std::nullptr_t> = nullptr,
        enable_if_t<!(std::is_nothrow_constructible<EF, EFP>::value || std::is_nothrow_constructible<EF, EFP&>::value), std::nullptr_t> = nullptr
    >
    explicit scope_guard(EFP&& f)
    try
        : exit_function_{f}
    {}
    catch(...)
    {
        if(Strategy().call_when_construct_failed()) {
            f();
        }
        throw;
    }

    template <
        typename EFP = EF,
        enable_if_t<std::is_nothrow_move_constructible<EFP>::value, std::nullptr_t> = nullptr
    >
    scope_guard(scope_guard&& rhs) noexcept
        : execute_on_destruction_{rhs.execute_on_destruction_}
        , strategy_{rhs.strategy_}
        , exit_function_{std::forward<EF>(rhs.exit_function_)}
    {
        rhs.release();
    }

    template <
        typename EFP = EF,
        enable_if_t<!std::is_nothrow_move_constructible<EFP>::value, std::nullptr_t> = nullptr,
        enable_if_t<std::is_copy_constructible<EFP>::value, std::nullptr_t> = nullptr
    >
    scope_guard(scope_guard&& rhs) noexcept(std::is_nothrow_copy_constructible<EF>::value)
        : execute_on_destruction_{rhs.execute_on_destruction_}
        , strategy_{rhs.strategy_}
        , exit_function_{rhs.exit_function_}
    {
        rhs.release();
    }

    ~scope_guard() noexcept(is_dtor_noexcept_t<EF, Strategy>::value)
    {
        if(execute_on_destruction_ && strategy_.call_when_dtor()) {
            exit_function_();
        }
    }

    void release() noexcept
    {
        execute_on_destruction_ = false;
    }

    scope_guard(const scope_guard&) = delete;
    scope_guard& operator=(const scope_guard&) = delete;
    scope_guard& operator=(scope_guard&&) = delete;

private:
    bool execute_on_destruction_{true};
    Strategy strategy_{};
    EF exit_function_;
};

template <typename EF>
class scope_exit : public scope_guard<EF, strategy_exit>
{
    using base_type = enable_if_t<!std::is_same<detail::remove_cvref_t<EF>, scope_exit>::value, scope_guard<EF, strategy_exit>>;
public:
    using base_type::base_type;
};

#if defined(SCOPE_USE_DEDUCTION_GUIDE)
template <class EF>
scope_exit(EF) -> scope_exit<EF>;
#endif // defined(SCOPE_USE_DEDUCTION_GUIDE)

#if defined(SCOPE_USE_SUCCESS_FAIL)

template <typename EF>
class scope_fail : public scope_guard<EF, strategy_fail>
{
    using base_type = enable_if_t<!std::is_same<detail::remove_cvref_t<EF>, scope_fail>::value, scope_guard<EF, strategy_fail>>;
public:
    using base_type::base_type;
};

#if defined(SCOPE_USE_DEDUCTION_GUIDE)
template <class EF>
scope_fail(EF) -> scope_fail<EF>;
#endif // defined(SCOPE_USE_DEDUCTION_GUIDE)

template <typename EF>
class scope_success : public scope_guard<EF, strategy_success>
{
    using base_type = enable_if_t<!std::is_same<detail::remove_cvref_t<EF>, scope_success>::value, scope_guard<EF, strategy_success>>;
public:
    using base_type::base_type;
};

#if defined(SCOPE_USE_DEDUCTION_GUIDE)
template <class EF>
scope_success(EF) -> scope_success<EF>;
#endif // defined(SCOPE_USE_DEDUCTION_GUIDE)

#endif // defined(SCOPE_USE_SUCCESS_FAIL)

template <class EF>
scope_exit<EF> make_scope_exit(EF&& f)
{
    return scope_exit<EF>(std::forward<EF>(f));
}

#if defined(SCOPE_USE_SUCCESS_FAIL)
template <class EF>
scope_fail<EF> make_scope_fail(EF&& f)
{
    return scope_fail<EF>(std::forward<EF>(f));
}

template <class EF>
scope_success<EF> make_scope_success(EF&& f)
{
    return scope_success<EF>(std::forward<EF>(f));
}
#endif // defined(SCOPE_USE_SUCCESS_FAIL)

} // namespace detail

using detail::scope_exit;
using detail::make_scope_exit;

#if defined(SCOPE_USE_SUCCESS_FAIL)
using detail::scope_fail;
using detail::make_scope_fail;
using detail::scope_success;
using detail::make_scope_success;
#endif // defined(SCOPE_USE_SUCCESS_FAIL)

} // namespace scope

#endif // NAKATT_SCOPE_HPP_
