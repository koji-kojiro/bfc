#include <stdio.h>
#include <stdlib.h>
#include <libgccjit.h>
#include "bfc.h"

static gcc_jit_lvalue *_bfc_get_value (bfc_t *);
static gcc_jit_lvalue *_bfc_get_pointer (bfc_t *);
static void _bfc_fatal_error (bfc_t *, const char *);
static void _bfc_compile (bfc_t *);
static void _bfc_compile_to_file (bfc_t *, char *);
static int _bfc_compile_and_run (bfc_t *);
static void _bfc_handle_plus (bfc_t *);
static void _bfc_handle_minus (bfc_t *);
static void _bfc_handle_lshift (bfc_t *);
static void _bfc_handle_rshift (bfc_t *);
static void _bfc_handle_dot (bfc_t *);
static void _bfc_handle_comma (bfc_t *);
static void _bfc_handle_lparen (bfc_t *);
static void _bfc_handle_rparen (bfc_t *);
static void _bfc_compile_char (bfc_t *, char);
static void _bfc_compile_string (bfc_t *, char *);
static void _bfc_compile_file (bfc_t *, char *);

bfc_t *
bfc_new (size_t memory_size, size_t max_paren, size_t opt_level)
{
  bfc_t *self = malloc (sizeof (bfc_t));
  self->ctxt = gcc_jit_context_acquire ();
  gcc_jit_context_set_str_option (
    self->ctxt, GCC_JIT_STR_OPTION_PROGNAME, "bfc");
  gcc_jit_context_set_int_option (
    self->ctxt, GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL, opt_level);

  self->byte_type =
    gcc_jit_context_get_type (self->ctxt, GCC_JIT_TYPE_CHAR);
  self->ptr_type =
    gcc_jit_context_get_type (self->ctxt, GCC_JIT_TYPE_UNSIGNED_INT);
  self->mem_type = 
    gcc_jit_context_new_array_type (
      self->ctxt, NULL, self->byte_type, memory_size);
  gcc_jit_type *int_type = 
    gcc_jit_context_get_type (self->ctxt, GCC_JIT_TYPE_INT);

  gcc_jit_param *argc_param =
    gcc_jit_context_new_param (self->ctxt, NULL, int_type, "argc");

  gcc_jit_type *argv_type = 
    gcc_jit_type_get_pointer (
      gcc_jit_type_get_pointer (
        gcc_jit_context_get_type (self->ctxt, GCC_JIT_TYPE_CHAR)));
  gcc_jit_param *argv_param =
    gcc_jit_context_new_param (self->ctxt, NULL, argv_type, "argv");
  gcc_jit_param *params[2] = {argc_param, argv_param};

  self->main_func = gcc_jit_context_new_function (
    self->ctxt, NULL, GCC_JIT_FUNCTION_EXPORTED,
    int_type, "main", 2, params, 0);

  self->num_paren = 0;
  self->max_paren = max_paren;

  self->block = gcc_jit_function_new_block (self->main_func, "main");

  self->memory =
    gcc_jit_context_new_global (
      self->ctxt, NULL, GCC_JIT_GLOBAL_EXPORTED,
      self->mem_type, "memory");
  self->pointer =
    gcc_jit_context_new_global (
      self->ctxt, NULL, GCC_JIT_GLOBAL_EXPORTED,
      self->ptr_type, "pointer");

  self->byte_zero =
    gcc_jit_context_zero (self->ctxt, self->byte_type);
  self->byte_one =
    gcc_jit_context_one (self->ctxt, self->byte_type);
  self->int_one =
    gcc_jit_context_one (self->ctxt, self->ptr_type);

  gcc_jit_param *putchar_param[1] = 
    {gcc_jit_context_new_param (self->ctxt, NULL, self->byte_type, "c")};
  self->putchar_func =
    gcc_jit_context_new_function (
      self->ctxt, NULL, GCC_JIT_FUNCTION_IMPORTED,
      int_type, "putchar", 1, putchar_param, 0);

  self->getchar_func =
    gcc_jit_context_new_function (
      self->ctxt, NULL, GCC_JIT_FUNCTION_IMPORTED,
      self->byte_type, "getchar", 0, NULL, 0);
  
  self->loop_test = malloc (sizeof (gcc_jit_block *) * max_paren);
  self->loop_body = malloc (sizeof (gcc_jit_block *) * max_paren);
  self->loop_after = malloc (sizeof (gcc_jit_block *) * max_paren);
  
  return self;
}

void
bfc_release (bfc_t *self)
{
  gcc_jit_context_release (self->ctxt);
  if (self->result != NULL)
    gcc_jit_result_release (self->result);
  free (self->loop_test);
  free (self->loop_body);
  free (self->loop_after);
  free (self);
}

int
bfc_exec_string (bfc_t *self, char *s)
{
  _bfc_compile_string (self, s);
  return _bfc_compile_and_run (self);
}

void
bfc_compile_string (bfc_t *self, char *s, char *dst)
{
  _bfc_compile_string (self, s);
  _bfc_compile_to_file (self, dst);
}

int
bfc_exec_file (bfc_t *self, char *src)
{
  _bfc_compile_file (self, src);
  return _bfc_compile_and_run (self);
}

void
bfc_compile_file (bfc_t *self, char *src, char *dst)
{
  _bfc_compile_file (self, src);
  _bfc_compile_to_file (self, dst);
}

static void
_bfc_fatal_error (bfc_t *self, const char *msg)
{
  printf ("bfc: error: %s\n", msg);
  exit (1);
}

static gcc_jit_lvalue *
_bfc_get_value (bfc_t *self)
{
  return gcc_jit_context_new_array_access (
    self->ctxt, NULL,
    gcc_jit_lvalue_as_rvalue (self->memory),
    gcc_jit_lvalue_as_rvalue (self->pointer));
}

static gcc_jit_lvalue *
_bfc_get_pointer (bfc_t *self)
{
  return self->pointer;
}

static void
_bfc_handle_plus (bfc_t *self)
{
  gcc_jit_lvalue *lvalue = _bfc_get_value (self);
  gcc_jit_block_add_assignment_op (
    self->block, NULL, lvalue, GCC_JIT_BINARY_OP_PLUS, self->byte_one);   
}

static void
_bfc_handle_minus (bfc_t *self)
{
  gcc_jit_lvalue *value = _bfc_get_value (self);
  gcc_jit_block_add_assignment_op (
    self->block, NULL, value, GCC_JIT_BINARY_OP_MINUS, self->byte_one);   
}

static void
_bfc_handle_lshift (bfc_t *self)
{
  gcc_jit_lvalue *ptr = _bfc_get_pointer (self);
  gcc_jit_block_add_assignment_op (
    self->block, NULL, ptr, GCC_JIT_BINARY_OP_MINUS, self->int_one);   
}

static void
_bfc_handle_rshift (bfc_t *self)
{
  gcc_jit_lvalue *ptr = _bfc_get_pointer (self);
  gcc_jit_block_add_assignment_op (
    self->block, NULL, ptr, GCC_JIT_BINARY_OP_PLUS, self->int_one);   
}

static void
_bfc_handle_dot (bfc_t *self)
{
  gcc_jit_lvalue *value = _bfc_get_value (self);
  gcc_jit_rvalue *args[1] = {gcc_jit_lvalue_as_rvalue (value)};
  gcc_jit_block_add_eval (
    self->block, NULL, 
    gcc_jit_context_new_call (
      self->ctxt, NULL, self->putchar_func, 1, args));
}

static void
_bfc_handle_comma (bfc_t *self)
{
  gcc_jit_lvalue *value = _bfc_get_value (self);
  gcc_jit_block_add_assignment (
    self->block, NULL, value,
    gcc_jit_context_new_call (
      self->ctxt, NULL, self->getchar_func, 0, NULL));
}

static void
_bfc_handle_lparen (bfc_t *self)
{
  if (self->num_paren == self->max_paren)
    _bfc_fatal_error (self, "max depth of parentheses exceeded");
  gcc_jit_block *loop_test =
    gcc_jit_function_new_block (self->main_func, NULL);
  gcc_jit_block *loop_after =
    gcc_jit_function_new_block (self->main_func, NULL);
  gcc_jit_block *loop_body =
    gcc_jit_function_new_block (self->main_func, NULL);
  
  gcc_jit_block_end_with_jump (
    self->block, NULL, loop_test);
  gcc_jit_block_end_with_conditional (
    loop_test, NULL,
    gcc_jit_context_new_comparison (
      self->ctxt, NULL, GCC_JIT_COMPARISON_EQ,
      gcc_jit_lvalue_as_rvalue (_bfc_get_value (self)),
      self->byte_zero),
    loop_after,
    loop_body);
  self->loop_test[self->num_paren] = loop_test;
  self->loop_body[self->num_paren] = loop_body;
  self->loop_after[self->num_paren] = loop_after;
  self->num_paren++;
  self->block = loop_body;
}

static void
_bfc_handle_rparen (bfc_t *self)
{
  if (self->num_paren == 0)
    _bfc_fatal_error (self, "unbalanced parentheses");
  self->num_paren--;
  gcc_jit_block_end_with_jump (
    self->block, NULL,
    self->loop_test[self->num_paren]);
  self->block = self->loop_after[self->num_paren];
}

static void
_bfc_compile_char (bfc_t *self, char c)
{
  switch (c) {
    case '+':
      _bfc_handle_plus (self);
      break;
    case '-':
      _bfc_handle_minus (self);
      break;
    case '<':
      _bfc_handle_lshift (self);
      break;
    case '>':
      _bfc_handle_rshift (self);
      break;
    case '.':
      _bfc_handle_dot (self);
      break;
    case ',':
      _bfc_handle_comma (self);
      break;
    case '[':
      _bfc_handle_lparen (self);
      break;
    case ']':
      _bfc_handle_rparen (self);
      break;
    default:
      break;
  }
}

static void
_bfc_compile_string (bfc_t *self, char *s)
{
  while (*s)
    _bfc_compile_char (self, *s++);
}

static void
_bfc_compile_file (bfc_t *self, char *src)
{
  FILE *fp = fopen (src, "r");
  if (!fp)
    _bfc_fatal_error (self, "failed to open file.");
  
  while (!feof (fp))
    _bfc_compile_char (self, fgetc (fp));

  fclose (fp);
}


static void
_bfc_compile (bfc_t *self)
{
  if (self->num_paren > 0)
    _bfc_fatal_error (self, "unbalanced parentheses");
  gcc_jit_block_end_with_return (
    self->block, NULL, gcc_jit_context_zero (
      self->ctxt, gcc_jit_context_get_type (self->ctxt, GCC_JIT_TYPE_INT))); 
  self->result = gcc_jit_context_compile (self->ctxt);
}

static int
_bfc_compile_and_run (bfc_t *self)
{
  _bfc_compile (self);
  if (self->result)
  {
    typedef int (*func_t) (int, char **);
    func_t main_func = gcc_jit_result_get_code (self->result, "main");
    return main_func (0, NULL);
  }
  return 1;
}

static void
_bfc_compile_to_file (bfc_t *self, char *fname)
{
  if (self->num_paren > 0)
    _bfc_fatal_error (self, "unbalanced parentheses");
  gcc_jit_block_end_with_return (
    self->block, NULL, gcc_jit_context_zero (
      self->ctxt, gcc_jit_context_get_type (self->ctxt, GCC_JIT_TYPE_INT))); 
  gcc_jit_context_compile_to_file (
    self->ctxt, GCC_JIT_OUTPUT_KIND_EXECUTABLE, fname);
}


