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

TEST_CASE("scope_exit accepts function like object")
{
    SECTION("function pointer") {
        {
            auto g = scope::make_scope_exit(func);
            value_of_func = 0;
        }
        REQUIRE(value_of_func == 1);
    }

    SECTION("std::function") {
        {
            auto g = scope::make_scope_exit(std::function<void()>(func));
            value_of_func = 0;
        }
        REQUIRE(value_of_func == 1);
    }

    SECTION("functor object") {
        {
            auto g = scope::make_scope_exit(Functor{});
            Functor::value = 0;
        }
        REQUIRE(Functor::value == 1);
    }

    SECTION("lambda") {
        int x = 0;
        {
            auto g = scope::make_scope_exit([&]{ x++; });
        }
        REQUIRE(x == 1);
    }

    SECTION("std::bind") {
        value_of_func = 0;
        {
            auto bf = std::bind(bind_func, 42);
            auto g = scope::make_scope_exit(bf);
        }
        REQUIRE(value_of_func == 42);
    }

    SECTION("std::bind member function") {
        bind_struct bs;
        {
            auto bf = std::bind(&bind_struct::mem_fun, std::ref(bs));
            auto g = scope::make_scope_exit(bf);
        }
        REQUIRE(bs.value == 1);
    }

    SECTION("reference_wrapper") {
        int x = 0;
        {
            auto lam = [&]{ x++; };
            auto g = scope::make_scope_exit(std::ref(lam));
        }
        REQUIRE(x == 1);
    }
}

TEST_CASE("scope_exit::release()")
{
    int x = 0;
    {
        auto guard = scope::make_scope_exit([&]{ ++x; });
        guard.release();
    }
    REQUIRE(x == 0);
}

TEST_CASE("released scope_guard does not call exit_function")
{
    int x = 0;
    {
        auto g1 = scope::make_scope_exit([&]{ ++x; });
        auto g2 = scope::make_scope_exit([]{ throw 42; });
        g2.release();
    }
    REQUIRE(x == 1);
}

TEST_CASE("scope_exit::scope_exit(EFP&& f): noexcept if is_nothrow_constructible_v<EF, EFP> || is_nothrow_constructible_v<EF, EFP&>")
{
    REQUIRE(std::is_nothrow_constructible<scope::scope_exit<void_func_t>, void_func_t>::value);
    REQUIRE(std::is_nothrow_constructible<scope::scope_exit<void_func_t>, void_func_t&>::value);
}

TEST_CASE("scope_exit::scope_exit(EFP&& f): If the initialization of exit_function throws an exception, calls f().")
{
    value_of_func = 0;
    try {
        scope::scope_exit<BadFunctor> g(func); // throw exception
        REQUIRE(false); // not reached
    }
    catch(TestException&) {
        REQUIRE(value_of_func == 1);
    }
    catch(...) {
        REQUIRE(false);
    }
}

TEST_CASE("~scope_exit() is noexcept")
{
    REQUIRE(std::is_nothrow_destructible<scope::scope_exit<void_func_t>>::value);
}

#if defined(SCOPE_USE_DEDUCTION_GUIDE)
TEST_CASE("scope_exit deduction guide")
{
    int x = 0;
    {
        auto lam = [&]{ ++x; };
        scope::scope_exit g1{lam};
        auto g2 = scope::scope_exit{lam};
    }
    REQUIRE(x == 2);
}
#endif
