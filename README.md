# Static Analysis Example

Some quick code I wrote, mainly solution to different code exercises.

They all work but have different bugs, which hopefully are detected by the supported static analysis tools.

Let's see.


## Run tests

```
cmake . -DCMAKE_BUILD_TYPE=Debug
```

### Compilation Warnings

```
make -j10
```

### Memory Leaks (dyn. analysis)

```
ctest -T memcheck
```

### Static Analysis

#### CI/CD Pipeline

Semgrep, CodeQL, and a few other tools (mainly clang-tidy) are run automatically on Github every commit.

These might not be public, but anyway;

https://semgrep.dev/orgs/eliot-roxbergh/findings

https://github.com/Eliot-Roxbergh/static_analysis/security/code-scanning

https://github.com/Eliot-Roxbergh/static_analysis/actions/workflows/cmake.yml


#### Manually

It's also possible to run clang-tidy locally via make, such as:

```
make codechecker-html
```

And for interactive view:

```
CodeChecker server &
CodeChecker store ./reports -n my-project
firefox localhost:8001
```


## Summary

I would call this somewhat simple or straightforward code, so we cannot accept many false positives.

### GCC
No warnings from GCC

### Valgrind
Each binary has been tested with Valgrind, and only two have memory issues: positives.c ptrs.c

### clang-tidy/clang-sa (CodeChecker)
CodeChecker reports a few useful errors, no real false positives.
Although we get some less important warnings as we use --enable sensitive.

```
Checker name 				Number of reports
bugprone-narrowing-conversions  		1
cert-err34-c					3
clang-diagnostic-sometimes-uninitialized 	4
core.CallAndMessage 				1
core.uninitialized.Branch 			2
deadcode.DeadStores 				2
unix.Malloc 					1
```

Interesting that we get a narrowing conversion warning not caught by -Wconversion, maybe enums is a special case?

### Semgrep

Semgrep found quite a few warnings (31?)

### CodeQL

CodeQL found two bugs:

```
Comparison of narrow type with wide type in loop condition
[High]
3/3.c:77
3/3.c:43
```

Interesting that this was not discovered by GCC -Wsign-compare, or by clang-tidy.

## Extras

### Additional Methods of Testing

Not looked into here, but static analysis is not enough really.

Ideally, we'd want to have unit tests, good code coverage, etc.
Perhaps also mock certain functions to see how the program
behaves for certain edge-cases.

For dynamic analysis we also have fuzzy testing.

Bonus: note also the different options to harden the binary itself (see https://github.com/Eliot-Roxbergh/examples/blob/master/c_programming/development_tips/gcc_flags.md)

#### Unit Tests

Google Test

Criterion

Check

For CMake see also CTest, and CDash (e.g. to be used in conjunction with Google Test)

#### Code Coverage

gcov & lcov (included in GCC)

#### Mocking

gmock (Google Test)

cmocka

(could ofc also naively mock a function manually by using #defines or look at the dynamic linking)

#### Fuzzy Testing

AFL++
