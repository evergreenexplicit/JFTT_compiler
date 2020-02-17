# JFTT compiler 2019
# Konrad Grochowski, 244936
# Files description:
- compiler.l -  contains instructions for Flex lexer,
- compiler.ypp - contains instructions for Bison parser,
- CodeBuilder.cpp (with header file) - contains all the compiler logic: direct parser functions used in Bison, memory and variables operations, comparison and mathematical operations, adding commands and error handling.
# Necessary programs:
- flex v. 2.6.4
- bison v. 3.0.4
- g++ v. 7.4.0
- make v. 4.1
# Usage:
Use  "make" make command to create the compiler, use the compiler with "./compiler <input> <output>".