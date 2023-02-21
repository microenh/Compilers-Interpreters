cmake_minimum_required(VERSION 3.12)

project(CompInterp)

# chapter 1

add_executable(list
  list.c
)

# chapter 2

add_executable(token1
  token1.c
)

add_executable(token2
  token2.c
  scanner.c 
  error.c
)

add_executable(compact
  compact.c
  scanner.c 
  error.c
)

# chapter 3

add_executable(xref
  xref.c
  symtab.c
  scanner.c
  error.c
)

add_executable(crunch
  crunch.c
  symtab.c 
  scanner.c 
  error.c
)

add_executable(uncrunch
  uncrunch.c
  symtab.c 
  scanner.c 
  error.c
)

# chapter 4

add_executable(postfix
  postfix.c 
  scanner.c 
  error.c
)

add_executable(calc
  calc.c 
  scanner.c 
  error.c
  symtab.c
)

add_executable(syntax1
  syntax1.c 
  scanner.c
  error.c 
  symtab.c 
  stmt.c 
  expr.c
)

add_executable(analyze
  analyze.c
  scanner.c
  error.c 
  symtab.c 
  stmt.c 
  expr.c
  decl.c
)

add_executable(syntax2
  syntax2.c
  scanner.c
  error.c 
  symtab.c 
  stmt.c 
  expr.c
  decl.c
)

add_executable(syntax3
  syntax3.c
  scanner.c
  error.c 
  symtab.c 
  stmt.c 
  expr.c
  decl.c
  routine.c
  standard.c 
)