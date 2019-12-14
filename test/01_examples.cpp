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

#include <sstream>

#include <catch2/catch.hpp>

#if defined(SCOPE_USE_SUCCESS_FAIL) && defined(SCOPE_USE_DEDUCTION_GUIDE)
TEST_CASE("standard paper p0052 demo")
{
    using namespace scope;
    std::ostringstream out{};
    auto lam = [&]{out << "called ";};
    try {
        auto v = scope_exit([&]{out << "always ";});
        auto w = scope_success([&]{out << "not ";}); // not called
        auto x = scope_fail(lam); // called
        throw 42;
    } catch(...) {
        auto y = scope_fail([&]{out << "not ";}); // not called
        auto z = scope_success([&]{out << "handled";}); // called
    }
    REQUIRE("called always handled" == out.str());
}
#endif // defined(SCOPE_USE_SUCCESS_FAIL) && defined(SCOPE_USE_DEDUCTION_GUIDE)

TEST_CASE("scope_exit called on destruction")
{
    int x = 0;
    {
        auto lam = [&]{ ++x; };
        scope::scope_exit<decltype(lam)> guard{lam};
        // In C++17 or later
        // scope::scope_exit guard{[&]{ ++x; }};
        // or,
        // auto guard = scope::scope_exit{[&]{ ++x; }};
    }
    REQUIRE(x == 1);
}

TEST_CASE("you can use make_scope_exit() instead in C++11/14")
{
    int x = 0;
    {
        auto guard = scope::make_scope_exit([&]{ ++x; });
    }
    REQUIRE(x == 1);
}

#if defined(SCOPE_USE_SUCCESS_FAIL)

TEST_CASE("scope_success called on destruction, not called on exception")
{
    int x = 0;

    SECTION("block finished successfully") {
        {
            auto guard = scope::make_scope_success([&]{ ++x; });
        }
        REQUIRE(x == 1);
    }

    SECTION("block failed on exception") {
        try {
            auto guard = scope::make_scope_success([&]{ ++x; });
            throw 42;
        }
        catch(...) {
        }
        REQUIRE(x == 0);
    }
}

TEST_CASE("scope_fail not called on destruction, called on exception")
{
    int x = 0;

    SECTION("block finished successfully") {
        {
            auto guard = scope::make_scope_fail([&]{ ++x; });
        }
        REQUIRE(x == 0);
    }

    SECTION("block failed on exception") {
        try {
            auto guard = scope::make_scope_fail([&]{ ++x; });
            throw 42;
        }
        catch(...) {
        }
        REQUIRE(x == 1);
    }
}

#endif // defined(SCOPE_USE_SUCCESS_FAIL)
