#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <stdio.h>
#include <stdlib.h>


#define DBG(...) \
  do \
  { \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  } \
  while(0);

  #define DBGX(...) \
  do \
  { \
  fprintf(stderr, "\n ERROR  (%s | %s | %d): \n\t", __FILE__, __func__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  } \
  while(0);

#define ERROR(...) \
  do \
  { \
  fprintf(stderr, "\n ERROR  (%s | %s | %d): \n\t", __FILE__, __func__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  } \
  while(0);
#endif

