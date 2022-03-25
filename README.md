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
Still a few FPs were reported from running clang-tidy + Semgrep + CodeQL.

19 (11+8) FPs, 16 (12+2+2) TPs, in addition to 6 duplicate TPs.

Note that many of the FPs were of the same type and easily ignored (Semgrep) or (mainly for for clang-tidy)
some were reasonable complaints that is not a problem in the specific context / already checked indirectly later ...
and thus the critique to check them directly or more clearly could very well be valid.

This also highlights that these static code analysis tools should be present from the start of development, to force a better code pattern (or a pattern more in line with the tool..?) thoughout.

### GCC
No warnings from GCC

### Valgrind
Each binary has been tested with Valgrind, and only two have memory issues: positives.c ptrs.c

### Static Analysis Tools

__CodeChecker:__ 12 unique, 2 duplicates (semgrep). ~8/22 false positives (including 2 TP duplicates = matched twice via other codepath)

__Semgrep:__ 2 unique, 3 duplicates (codechecker 2, codeql 1). Additionally, it found ~11 false positives (I think), but this was still somewhat useful as it warns why these functions can be unsafe, although it does not know whether it is ok in this specific case.. might be annoying after a while.

__CodeQL:__ 2 unique, 1 duplicate (semgrep)

It is a bit scary that each of the three tools gave different warnings, with very little overlap.
So running fewer tools we would not have found all these faults, what are we missing that we could find with other tools?

#### clang-tidy/clang-sa (CodeChecker)
CodeChecker reports many useful errors (memory errors and other important problems), but some false positives.
As we use its sensitive setting, we get a few extra LOW and MEDIUM warnings (can be tweaked a lot).

```
-----------------------------------------------------------------------
Checker name                             | Severity | Number of reports  : True Positives (my approximation)
-----------------------------------------------------------------------
core.uninitialized.Branch                | HIGH     |                 4  : 3
core.CallAndMessage                      | HIGH     |                 1  : ALL
clang-diagnostic-sometimes-uninitialized | MEDIUM   |                 4  : 2
unix.Malloc                              | MEDIUM   |                 2  : ALL
unix.MallocSizeof                        | MEDIUM   |                 2  : NONE
unix.API                                 | MEDIUM   |                 3  : 1
bugprone-narrowing-conversions           | MEDIUM   |                 1  : ALL
cert-err34-c                             | LOW      |                 3  : ALL
deadcode.DeadStores                      | LOW      |                 2  : 2
-----------------------------------------------------------------------

Unique warnings
  [MED.] 3/3.c:25           [unix.Malloc]                    TP? (might be problem if hits=0?) Use of memory after it is freed
  [MED.] 4/4.c:109          [...-sometimes-uninitialized]    FP (???) Variable 'boards' is used uninitialized whenever 'if' condition is true 
  [MED.] 4/4.c:112          [...-sometimes-uninitialized]    FP? (same as above)
  [MED.] 4/4.c:126          [unix.API]                       ~TP (shouldn't really be a problem) Call to 'calloc' has an allocation size of 0 bytes
  [HIGH] 4/4.c:192          [core.uninitialized.Branch]      TP (goto->free before declaration!) Branch condition evaluates to a garbage value

  [MED.] enum.c:26          [bugprone-narrowing-conversions] TP, Narrowing conversion from 'double' to 'enum'
  [MED.] positives.c:44     [unix.Malloc]                    TP, Use of memory after it is freed 
  [MED.] positives.c:45     [...-sometimes-uninitialized]    TP, Variable 'data_2' is used uninitialized whenever 'if' condition is true 
  [HIGH] positives.c:66     [core.uninitialized.Branch]      TP, Branch condition evaluates to a garbage value
  [HIGH] ptrs.c:73          [core.CallAndMessage]            TP, 2nd function call argument is an uninitialized value
  
  [MED.] read_input.c:139   [...-sometimes-uninitialized]    TP, variable 'nums' is used uninitialized whenever 'if' condition is true 
  [MED.] read_input.c:144   [unix.API]                       FP, Call to 'malloc' has an allocation size of 0 bytes 
  [MED.] read_input.c:144   [unix.API]                       FP, DUPLICATE (same as above)
  [MED.] read_input.c:144   [unix.MallocSizeof]              FP? Result of 'malloc' is converted to a pointer of type 'int' ... incompatible with sizeof ('unsigned int')
  [MED.] read_input.c:208   [unix.MallocSizeof]              FP? Result of 'malloc' is converted to a pointer of type 'int', which is incompatible with sizeof operand type 'unsigned int' 
  [HIGH] read_input.c:172   [core.uninitialized.Branch]      TP, Branch condition evaluates to a garbage value
  [HIGH] read_input.c:172   [core.uninitialized.Branch]      TP, DUPLICATE (same as above) 

  [LOW]  read_input.c:58    [cert-err34-c]                   TP, 'fscanf' used to convert a string to an integer value, ... will not report conversion errors (use 'strtol' instead) (1/3)
  [LOW]  bit_manip.c:16     [deadcode.DeadStores]            TP, minor, Value stored to 'bits_inverse' is never read
  [LOW]  bit_manip.c:17     [deadcode.DeadStores]            TP, minor, (same as above)

Detected by other tools (semgrep)
//These two are some kind of duplicate, although funnily enough CodeChecker only warns on integers and fscanf, and Semgrep only explains why strings and fscanf are problematic (Semgrep warns on both, but fails to mention why integers are problematic)
  [LOW]  read_input.c:151   [cert-err34-c]                   TP, 'fscanf' (2/3)
  [LOW]  read_input.c:217   [cert-err34-c]                   TP, 'fscanf' (3/3)
```

Interesting that we get a narrowing conversion warning not caught by -Wconversion, maybe enums is a special case?

#### Semgrep

Semgrep gave 16 warnings, with many false positives (a bit gray area).
Semgrep seems very basic, atleast with the relatively few default rulesets for C.
However, as I understand the benefit with Semgrep is the ease of writing new rules.
So far it has only complained on the use of unsafe standard functions, some are FPs or not a big deal, but still
good input that other tools largely ignored.

For instance, a bit silly to always complain on the use of strlen or memcpy, but still might be a good idea to use the safer strlen_s and memcpy_s.


```
Unique
  [LOW]   array.c:30        bcopy-1    //FP. Claims all memcpy is unsafe (but we know that destination is large enough). An alternative is memcpy_s from C11.
  [HIGH]  read_input.c:295  vswscanf-1   //TP! fscanf is unsafe if used with %s and no size limit, can overflow target buffer
  [HIGH]  read_input.c:58   vswscanf-1   //^
  [LOW]   read_input.c:74   _gettc-1  //FP. Claims fgetc is unsafe ("need to manually check buffer boundries"), but we don't even use the returned value from fgets. Ok?
  [LOW]   read_input.c:133  _gettc-1  //^
  [LOW]   read_input.c:194  _gettc-1  //^
  [LOW]   read_input.c:274  _gettc-1  //^
  [LOW]   read_input.c:31   _gettc-1  //^
  [LOW]   3.c:25            _mbslen-1   //FP. Claims strlen is unsafe since it over-reads if not null-terminated. (there is strlen_s)
  [LOW]   read_input.c:19   open-1    //FP? Claims on fopen is unsafe if an attacker can by symlink read or write to arbitrary file (we only open with read permissions)
  [LOW]   read_input.c:265  open-1    //^
  [LOW]   read_input.c:184  open-1    //^
  [LOW]   read_input.c:123  open-1    //^

Detected by other tools
  [HIGH]  positives.c:22    _vstprintf-1  //TP! Use snprintf(/sprintf_s) instead of sprintf.
  [HIGH]  read_input.c:151  vswscanf-1 // ~TP, Warns but does not describe why integers are a problem, which CodeChecker does see [cert-err34-c]
  [HIGH]  read_input.c:217  vswscanf-1 // ^
```

(apparently you can't see the warnings for more than 1 month if you don't pay them 40$/month, oops. Can run it locally though..)

#### CodeQL

CodeQL found three bugs, none of which were discovered by clang-tidy (or vice-versa):

```
Unique
  [High] advent2021/3/3.c:77 //Comparison of narrow type with wide type in loop condition
  [High] advent2021/3/3.c:43
  //afaik comparison is usually ok but in certain loop expressions there's the possibility of infinite loop (even if the smaller variable is promoted).
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
