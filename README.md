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
Luckily with running clang-tidy + Semgrep + CodeQL none / very few false positives were found.

### GCC
No warnings from GCC

### Valgrind
Each binary has been tested with Valgrind, and only two have memory issues: positives.c ptrs.c

### Static Analysis Tools

__CodeChecker:__ 16 unique, 2 duplicates (semgrep). ~0/16 false positives

__Semgrep:__ 2 unique, 3 duplicates (codechecker 2, codeql 1). Additionally, it found ~11 false positives (I think), but this was still somewhat useful as it warns why these functions can be unsafe, although it does not know whether it is ok in this specific case.. might be annoying after a while.

__CodeQL:__ 2 unique, 1 duplicate (semgrep)

It is a bit scary that each of the three tools gave different warnings, with very little overlap.
So running fewer tools we would not have found all these faults, what are we missing that we could find with other tools?

#### clang-tidy/clang-sa (CodeChecker)
CodeChecker reports a few useful errors, no real false positives.
Although we get some less important warnings as we use --enable sensitive.

```
-----------------------------------------------------------------------
Checker name                             | Severity | Number of reports
-----------------------------------------------------------------------
bugprone-narrowing-conversions           | MEDIUM   |                 1
unix.Malloc                              | MEDIUM   |                 1
core.uninitialized.Branch                | HIGH     |                 3
cert-err34-c                             | LOW      |                 3
clang-diagnostic-sometimes-uninitialized | MEDIUM   |                 4
deadcode.DeadStores                      | LOW      |                 2
unix.MallocSizeof                        | MEDIUM   |                 2
unix.API                                 | MEDIUM   |                 1
core.CallAndMessage                      | HIGH     |                 1
-----------------------------------------------------------------------

Unique warnings
  4/4.c:109
  4/4.c:112
  4/4.c:192
  bit_manip.c:16
  bit_manip.c:17
  enum.c:26
  positives.c:44
  positives.c:45
  positives.c:66
  ptrs.c:73
  read_input.c:139
  read_input.c:144
  read_input.c:144 (again)
  read_input.c:172
  read_input.c:208
  read_input.c:58

Detected by other tools (semgrep)
  read_input.c:151 //kind of duplicate, although funnily enough CodeChecker only warns on integers and fscanf, and Semgrep only explains why strings and fscanf are problematic (Semgrep warns on both, but fails to mention why integers are problematic)
  read_input.c:217
```

Interesting that we get a narrowing conversion warning not caught by -Wconversion, maybe enums is a special case?

#### Semgrep

Semgrep gave 16 warnings, with many false positives (a bit gray area).
Semgrep seems very basic, atleast with the relatively few default rulesets for C.
So far it has only complained on the use of unsafe standard functions, some are FPs or not a big deal, but still
good input that other tools largely ignored.

For instance, a bit silly to always complain on the use of strlen or memcpy, but still might be a good idea to use the safer strlen_s and memcpy_s.


```
Unique
  [LOW]   array.c:30        bcopy-1  //FP. Claims all memcpy is unsafe (but we know that destination is large enough). An alternative is memcpy_s from C11.
  [HIGH]  read_input.c:295  vswscanf-1 //Bug! fscanf is unsafe if used with %s and no size limit, can overflow target buffer
  [HIGH]  read_input.c:58   vswscanf-1 
  [LOW]   read_input.c:74   _gettc-1  //FP. Claims fgetc is unsafe ("need to manually check buffer boundries"), but we don't even use the returned value from fgets. Ok?
  [LOW]   read_input.c:133  _gettc-1 
  [LOW]   read_input.c:194  _gettc-1 
  [LOW]   read_input.c:274  _gettc-1 
  [LOW]   read_input.c:31   _gettc-1 
  [LOW]   3.c:25            _mbslen-1  //FP. Claims strlen is unsafe since it over-reads if not null-terminated. (there is strlen_s)
  [LOW]   read_input.c:19   open-1     //FP? Claims on fopen is unsafe if an attacker can by symlink read or write to arbitrary file (we only open with read permissions)
  [LOW]   read_input.c:265  open-1 
  [LOW]   read_input.c:184  open-1 
  [LOW]   read_input.c:123  open-1 

Detected by other tools
  [HIGH]  positives.c:22    _vstprintf-1  //Bug! Use snprintf(/sprintf_s) instead of sprintf.
  [HIGH]  read_input.c:151  vswscanf-1 // Warns but does not describe why integers are a problem, which CodeChecker does see [cert-err34-c]
  [HIGH]  read_input.c:217  vswscanf-1 
```

(apparently you can't see the warnings for more than 1 month if you don't pay them 40$/month, oops. Can run it locally though..)

#### CodeQL

CodeQL found three bugs, none of which were discovered by clang-tidy (or vice-versa):

```
Unique
  [High] advent2021/3/3.c:77 //Comparison of narrow type with wide type in loop condition
  [High] advent2021/3/3.c:43
  //Interesting that this was not discovered by GCC -Wsign-compare, or by clang-tidy.

Detected by other tools
  [Critical] positives.c:22 // Bug! Likely overrunning write
```



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
