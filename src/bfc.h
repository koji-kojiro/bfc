#pragma once

#define VERSION "0.0.1"

typedef struct
{
  gcc_jit_context *ctxt;

  gcc_jit_type *byte_type;
  gcc_jit_type *ptr_type;
  gcc_jit_type *mem_type;
  
  gcc_jit_lvalue *memory;
  gcc_jit_lvalue *pointer;
  
  gcc_jit_function *main_func;
  gcc_jit_function *putchar_func;
  gcc_jit_function *getchar_func;
  
  gcc_jit_rvalue *byte_zero;
  gcc_jit_rvalue *byte_one;
  gcc_jit_rvalue *int_one;
  
  gcc_jit_block *block;
  gcc_jit_block **loop_test;
  gcc_jit_block **loop_body;
  gcc_jit_block **loop_after;
  size_t num_paren;
  size_t max_paren;  
  gcc_jit_result *result;
} bfc_t;

bfc_t *bfc_new (size_t, size_t, size_t);
int bfc_exec_string (bfc_t *, char *);
void bfc_compile_string (bfc_t *, char *, char *);
int bfc_exec_file (bfc_t *, char *);
void bfc_compile_file (bfc_t *, char *, char *);
void bfc_release (bfc_t *);
