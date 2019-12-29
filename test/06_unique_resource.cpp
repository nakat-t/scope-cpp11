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

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include <catch2/catch.hpp>

TEST_CASE("demonstrate_unique_resource_with_stdio")
{
    auto fclose = [](FILE* file){ ::fclose(file); }; // not allowed to take address
    const std::string filename = "hello.txt";
    {
        auto file = scope::make_unique_resource(::fopen(filename.c_str(), "w"), fclose);
        ::fputs("Hello World!\n", file.get());
        REQUIRE(file.get() != NULL);
    }
    {
        std::ifstream input { filename };
        std::string line { };
        getline(input, line);
        REQUIRE("Hello World!" == line);
        getline(input, line);
        REQUIRE(input.eof());
    }
    ::remove(filename.c_str());
    {
        auto file = scope::make_unique_resource_checked(::fopen("nonexistingfile.txt", "r"), (FILE*)NULL, fclose);
        REQUIRE(file.get() == NULL);
    }
}
