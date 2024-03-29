# Static Analysis Example

_**For a quick example of a C project with static analysis setup see:** https://github.com/Eliot-Roxbergh/task_portknocker_

This is a small demo analyzing C code, in particular static analysis.
The code in question involve a dozen or so smaller C files which I wrote, test files or solution to different code exercises.

These small programs all "work" but according to static analysis tools they have many bugs (some I wilfully introduced).
It is interesting to see that even with many GCC warnings enabled (CMakeLists.txt) no warnings were reported,
and except for two memory errors I introduced (positives.c and ptrs.c), valgrind has no complaints.

The code was compiled with GCC 8.4.0 and relevant flags include: _"-Wall -Wextra -pedantic -Werror -Wformat=2  -Wconversion -Wdouble-promotion -O0 -g"_

Results from analysis below, also summarized in c\_testing\_slides.pdf.


### Addendum
More GCC flags which we could have added (would have detected some additional bugs): _"-Wshadow -Wundef -Og"_,
additionally, GCC can provide runtime checks with e.g. _"-fsanitize=undefined"_[1] (not sure how this compares to the Valgrind suite).

On later releases GCC (>=10) also provides some static analysis functionality with **_"-fanalyzer"_** [2]. On this code base, GCC (12.1.0) -fanalyzer finds only two issues, namely the double frees in _positives.c_. No FPs. 


I can also add that **Cppcheck** looks somewhat useful and found _one_ bug not discovered by any of the other tools. (Cppcheck is FOSS)
Cppcheck was not evaluated below, so I provide a very brief summary here,

```
Unique bug found:
 [array.c:30]: (warning, inconclusive) Array 'my_apa' is filled incompletely. Did you forget to multiply the size given to 'memcpy()' with 'sizeof(*my_apa)'?

Otherwise Cppcheck gave mostly "style" advice, such as
 (style) The scope of the variable 'X' can be reduced
 (style) Variable 'X' is reassigned a value before the old one has been used.
 (style) The scope of the variable 'X' can be reduced
 (style) Condition 'X>Y' is always true
 (style) Parameter 'X' can be declared with const
 (style) Function 'X' argument 2 names different: declaration 'input' definition 'user_input'.
These were generally not found with CodeChecker (clang-tidy) (or the other tools tested), even with sensitivity=extreme

Or in other cases I have seen it useful to detect incorrect checks or assumptions, such as this one warning (in another codebase):
 Assuming that condition 'ptr != NULL' is not redundant
 Null pointer dereference [on a few lines earlier]

Cppcheck 1.82
Example (if included cmake file doesn't work): cppcheck . --enable=all --inconclusive
```

SonarCloud (**SonarQube**) is another tool not evaluated here, which detected some interesting stuff when tested on another code base (similar to CodeQL few hits, but also very few FP -> that's usable).
(It is proprietary but free to use for open-source software, https://sonarcloud.io/)

There seems to be a never ending list of static analysis tools (especially if we include proprietary and/or expensive products),
the ones briefly tested most often only add one or two interesting unique warnings... the question is when to stop.
So far these five tools (clang-tidy, semgrep, codeql, cppcheck, sonar) seem reasonable, although potential overhead with
duplicates and FPs is concerning. Only choosing one, the tool of choice would be clang-tidy (CodeChecker) based on evaluation below.

Additional SAST tools were not tested, but are worth to mention, **flawfinder, infer, frama-c** (and also **splint** but is only useful if coded with their style in mind?). TODO: Codechecker supports e.g. infer, cppcheck, and clang-tidy - can we combine these to one workflow? TODO: flawfinder and frama-c seem to be the next candidates to try based on "my gut feeling" and "the word on the street".

[1] - https://blogs.oracle.com/linux/post/improving-application-security-with-undefinedbehaviorsanitizer-ubsan-and-gcc,
https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html \
[2] - https://gcc.gnu.org/onlinedocs/gcc/Static-Analyzer-Options.html#index-fanalyzer, https://developers.redhat.com/articles/2022/04/12/state-static-analysis-gcc-12-compiler


## Software Used

```
clang-7 / clang-tidy-7
GCC 8.4.0
Valgrind 3.13.0
Semgrep and CodeQL via CI pipeline (2022-03-22):
    Semgrep 0.86.5  (/w all C specific rules, about 85)
    CodeQL 2.8.1 (../CodeQL/0.0.0-20220214/..)
(Ubuntu 18.04)
```

Note that newer versions of the compiler and the static analysis tools will likely detect even more warnings
(clang-7 is fairly old).

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

It is also possible to run clang-tidy locally (as opposed to in the CI pipeline)

```
make codechecker-html
```
or
```
make clean
CodeChecker log --build "make" --output ./compile_commands.json
CodeChecker analyze ./compile_commands.json --enable sensitive --ctu --clean --output ./reports
```

And for interactive view:

```
CodeChecker server &
CodeChecker store ./reports -n my-project
firefox localhost:8001 &
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

I was suprised to see that it did not complain of cert-err33-c, i.e. we do not check the return values of snprintf (et al.). Perhaps I have a too old version, as I've seen this warning in other situations. I think this is a warning which we could try to enable in the future...

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

_CodeChecker 6.19.1, clang(-tidy) 7.0.0_


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

_(ran online via this Github CI pipeline, 2022-03-22)_

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

_(ran online via this Github CI pipeline, 2022-03-22)_

## Extras

### Additional Methods of Testing

Not looked into here, but static analysis is not enough really.

Ideally, we'd want to have unit tests, good code coverage, etc.
Perhaps also mock certain functions to see how the program
behaves for certain edge-cases.

Additionally, with black-box testing such as memory check with Valgrind we also want to have better code coverage.
Valgrind only checks the program under test, so it is possible that memory leaks are missed if that path is not taken
and additionally we do not currently test the library in _react_exercise_ for this reason.

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


## TODO

Q: How can (if) these bugs be exploited in the wild?

Q: Fix the bugs

Q: How long to filter out false positives? etc.

Bonus Q: Can we integrate all useful tools into one build flow? E.g. CodeChecker (clang-tidy, cppcheck, infer, (sparse)) + Semgrep looks quite powerful. But no CI right now, thats beyond our scope.

Bonus: Try flawfinder and frama-c (or whatever seems promising)

(didn't mention code-format and other things semi relevant)
