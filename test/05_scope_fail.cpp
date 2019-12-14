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
#include "scope/scope.hpp"

#include <type_traits>

#include <catch2/catch.hpp>
#include "helper.hpp"

using namespace helper;

#if defined(SCOPE_USE_SUCCESS_FAIL)

TEST_CASE("scope_fail accepts function like object")
{
    SECTION("function pointer") {
        try {
            auto g = scope::make_scope_fail(func);
            value_of_func = 0;
            throw 0;
        }
        catch(...) {
        }
        REQUIRE(value_of_func == 1);
    }

    SECTION("std::function") {
        try {
            auto g = scope::make_scope_fail(std::function<void()>(func));
            value_of_func = 0;
            throw 0;
        }
        catch(...) {
        }
        REQUIRE(value_of_func == 1);
    }

    SECTION("functor object") {
        try {
            auto g = scope::make_scope_fail(Functor{});
            Functor::value = 0;
            throw 0;
        }
        catch(...) {
        }
        REQUIRE(Functor::value == 1);
    }

    SECTION("lambda") {
        int x = 0;
        try {
            auto g = scope::make_scope_fail([&]{ x++; });
            throw 0;
        }
        catch(...) {
        }
        REQUIRE(x == 1);
    }

    SECTION("std::bind") {
        value_of_func = 0;
        try {
            auto bf = std::bind(bind_func, 42);
            auto g = scope::make_scope_fail(bf);
            throw 0;
        }
        catch(...) {
        }
        REQUIRE(value_of_func == 42);
    }

    SECTION("std::bind member function") {
        bind_struct bs;
        try {
            auto bf = std::bind(&bind_struct::mem_fun, std::ref(bs));
            auto g = scope::make_scope_fail(bf);
            throw 0;
        }
        catch(...) {
        }
        REQUIRE(bs.value == 1);
    }

    SECTION("reference_wrapper") {
        int x = 0;
        try {
            auto lam = [&]{ x++; };
            auto g = scope::make_scope_fail(std::ref(lam));
            throw 0;
        }
        catch(...) {
        }
        REQUIRE(x == 1);
    }
}

TEST_CASE("scope_fail::release()")
{
    int x = 0;

    try {
        auto guard = scope::make_scope_fail([&]{ ++x; });
        guard.release();
        throw 42;
    }
    catch(...) {
    }
    REQUIRE(x == 0);
}

TEST_CASE("scope_fail::scope_fail(EFP&& f): noexcept if is_nothrow_constructible_v<EF, EFP> || is_nothrow_constructible_v<EF, EFP&>")
{
    REQUIRE(std::is_nothrow_constructible<scope::scope_fail<void_func_t>, void_func_t>::value);
    REQUIRE(std::is_nothrow_constructible<scope::scope_fail<void_func_t>, void_func_t&>::value);
}

TEST_CASE("scope_fail::scope_fail(EFP&& f): If the initialization of exit_function throws an exception, calls f().")
{
    value_of_func = 0;
    try {
        scope::scope_fail<BadFunctor> g(func); // throw exception
        REQUIRE(false); // not reached
    }
    catch(TestException&) {
        REQUIRE(value_of_func == 1);
    }
    catch(...) {
        REQUIRE(false);
    }
}

TEST_CASE("~scope_fail() is noexcept")
{
    REQUIRE(std::is_nothrow_destructible<scope::scope_fail<void_func_t>>::value);
}

#if defined(SCOPE_USE_DEDUCTION_GUIDE)
TEST_CASE("scope_fail deduction guide")
{
    int x = 0;

    SECTION("block finished successfully") {
        {
            scope::scope_fail g1{[&]{ ++x; }};
            auto g2 = scope::scope_fail([&]{ ++x; });
        }
        REQUIRE(x == 0);
    }

    SECTION("block failed on exception") {
        try {
            scope::scope_fail g1{[&]{ ++x; }};
            auto g2 = scope::scope_fail([&]{ ++x; });
            throw 42;
        }
        catch(...) {
        }
        REQUIRE(x == 2);
    }
}
#endif

#endif // defined(SCOPE_USE_SUCCESS_FAIL)
