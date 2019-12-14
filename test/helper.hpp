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
#ifndef NAKATT_HELPER_HPP_
#define NAKATT_HELPER_HPP_

#include <exception>
#include <functional>

namespace helper {

#if __cplusplus >= 201703L
typedef void (*void_func_t)() noexcept;
#else
typedef void (*void_func_t)();
#endif

extern int value_of_func;
inline void func() noexcept { value_of_func++; }
inline void bind_func(int x) noexcept { value_of_func += x; }

struct Functor
{
    static int value;
    Functor() noexcept {}
    void operator()() noexcept { value++; }
};

struct TestException : public std::exception {};

struct BadFunctor
{
    explicit BadFunctor(std::function<void()>) { throw TestException(); }
    void operator()() noexcept {}
};

struct bind_struct
{
    int value = 0;
    void mem_fun() noexcept { value++; }
};

} // namespace helper

#endif // NAKATT_HELPER_HPP_
