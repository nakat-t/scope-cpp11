# scope-cpp11: A single header-only scope guard library for C++11 or later

scope-cpp11 is an implementation of `scope_exit`, `scope_success`, `scope_fail` and `unique_resource` proposed for the C++ standard at by [P0052r10](http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2019/p0052r10.pdf).

`scope_exit`, `unique_resource` is available in C++11 or later. `scope_success` and `scope_fail` are available in C++17 or later (because these require std::uncaught_exceptions()).

## Example

```cpp
#include <scope/scope.hpp>

using namespace scope;

void demo_scope_exit_fail_success_in_p0052() {
    std::ostringstream out{};
    auto lam = [&]{out << "called ";};
    try {
        auto v = scope_exit([&]{out << "always ";});
        auto w = scope_success([&]{out << "not ";}); // not called
        auto x = scope_fail(lam); // called
        throw 42;
    } catch(...) { // v, x called at this point
        auto y = scope_fail([&]{out << "not ";}); // not called
        auto z = scope_success([&]{out << "handled";}); // called
    } // z called at this point
    assert("called always handled" == out.str());
}

void demonstrate_unique_resource_with_stdio_in_p0052() {
    auto fclose = [](auto file){ ::fclose(file); }; // not allowed to take address
    const std::string filename = "hello.txt";
    {   auto file = scope::make_unique_resource(::fopen(filename.c_str(), "w"), fclose);
        ::fputs("Hello World!\n", file.get());
        assert(file.get() != NULL);
    }
    {   std::ifstream input { filename };
        std::string line { };
        getline(input, line);
        assert("Hello World!" == line);
        getline(input, line);
        assert(input.eof());
    }
    ::remove(filename.c_str());
    {   auto file = scope::make_unique_resource_checked(::fopen("nonexistingfile.txt", "r"), (FILE*)NULL, fclose);
        assert(file.get() == NULL);
    }
}
```

## Install

scope-cpp11 is a single header-only library. Copy `scope.hpp` into your project.

## Dependencies

scope-cpp11 depends on the C++ standard library only. (I use [catch2](https://github.com/catchorg/Catch2) for unit test)

## Usage

Refer to [P0052r10](http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2019/p0052r10.pdf) for detailed documentation.

### Difference from P0052r10

* Header file name is `scope.hpp` instead of `<scope>`.
* Namespace is `scope::` instead of `std::`.
* Added `make_scope_exit()`, `make_scope_success()`, `make_scope_fail()`, `make_unique_resource()` functions. This extension is for C++11/14 that doesn't have a template deduction guide.

## Reference

Scope guard concept was proposed [by Petru Marginean and Andrei Alexandrescu](https://www.drdobbs.com/cpp/generic-change-the-way-you-write-excepti/184403758).

It has been proposed for standardization by [P0052r10](http://www.open-std.org/JTC1/SC22/WG21/docs/papers/2019/p0052r10.pdf), and adopted in 2019-03.

## Appendix

### 1. Test status

scope-cpp11 passed the [test](https://github.com/PeterSommerlad/SC22WG21_Papers/blob/master/workspace/P0052_scope_exit/src/Test.cpp) included in the original proposal.
