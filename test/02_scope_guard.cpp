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

TEST_CASE("scope_guard (scope_guard && rhs)")
{
    {
        auto g = scope::make_scope_exit(func);
        auto g2{std::move(g)};
        value_of_func = 0;
    }
    REQUIRE(value_of_func == 1);
}

TEST_CASE("scope_guard (scope_guard && rhs): noexpect if is_nothrow_move_constructible_v<EF>")
{
    REQUIRE(std::is_nothrow_move_constructible<scope::scope_exit<void_func_t>>::value);
}

TEST_CASE("scope_guard (scope_guard && rhs): noexpect if is_nothrow_copy_constructible_v<EF>")
{
    struct EF
    {
        EF() noexcept {}
        EF(const EF&) noexcept {}
        EF(EF&&) { throw TestException(); }
        void operator()() noexcept {}
    };
    REQUIRE(std::is_nothrow_move_constructible<scope::scope_exit<EF>>::value);
}

TEST_CASE("scope_guard (scope_guard && rhs): Any exception thrown during the initialization of exit_function")
{
    struct EF
    {
        EF() noexcept {}
        EF(const EF&) { throw TestException(); }
        void operator()() noexcept {}
    };
    try {
        scope::scope_exit<EF> g(EF{});
        scope::scope_exit<EF> g2{std::move(g)}; // throw exception
        REQUIRE(false); // not reached
    }
    catch(TestException&) {
        // Test OK
    }
    catch(...) {
        REQUIRE(false);
    }
}

TEST_CASE("release(): Equivalent to execute_on_destruction = false.")
{
    {
        auto g = scope::make_scope_exit(func);
        g.release();
        value_of_func = 0;
    }
    REQUIRE(value_of_func == 0);
}
