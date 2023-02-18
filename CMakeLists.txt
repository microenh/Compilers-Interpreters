cmake_minimum_required(VERSION 3.12)

add_executable(list
  list.c
)

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
