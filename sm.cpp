/*
 * Made in 2010 by Christian Stigen Larsen
 * http://csl.sublevel3.org
 *
 * Placed in the public domain by the author.
 *
 * This is a very simple stack machine.
 *
 * It's Turing complete, and you can do anything you want
 * with it, as long as you do it within 1 Mb of memory.
 *
 * Currently, the only program it accepts is hardcoded.
 * Program and memory size will be parameters later on.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <vector>

enum Op {
  NOP,  // do nothing
  ADD,  // pop a, pop b, push a + b
  SUB,  // pop a, pop b, push a - b
  AND,  // pop a, pop b, push a & b
  OR,   // pop a, pop b, push a | b
  XOR,  // pop a, pop b, push a ^ b
  NOT,  // pop a, push !a
  IN,   // push one byte read from stream
  OUT,  // pop one byte and write to stream
  LOAD, // pop a, push byte read from address a
  STOR, // pop a, pop b, write b to address a
  JMP,  // pop a, goto a
  JZ,   // pop a, if a == 0 goto a
  PUSH, // push next word
  DUP   // duplicate word on stack
};

const char* to_s(Op op)
{
  switch ( op ) {
  default:
  case NOP: return "NOP"; break;
  case ADD: return "ADD"; break;
  case SUB: return "SUB"; break;
  case AND: return "AND"; break;
  case OR:  return "OR"; break;
  case XOR: return "XOR"; break;
  case NOT: return "NOT"; break;
  case IN:  return "IN"; break;
  case OUT: return "OUT"; break;
  case LOAD: return "LOAD"; break;
  case STOR: return "STOR"; break;
  case JMP: return "JMP"; break;
  case JZ:  return "JZ"; break;
  }
}

// machine
std::vector<int32_t> stack;
int32_t memory[1024*1000];
int32_t ip = 0;
FILE* fin = stdin;
FILE* fout = stdout;

static void stop(const char* s, int code=1)
{
  fprintf(stderr, "%s\n", s);
  exit(code);
}

static void push(const int32_t& n)
{
  stack.push_back(n);
}

static int32_t pop()
{
  int32_t n = stack.back();
  stack.pop_back();
  return n;
}

static void check_bounds(int32_t n, const char* msg)
{
  if ( n>=0 && n<=sizeof(memory)/sizeof(int32_t) )
    return;

  fprintf(stderr, "%s out of bounds\n", msg);
  exit(1);
}

static void help()
{
  Op op=NOP; 
  do {
    printf("0x%x = %s\n", op, to_s(op));
    op = static_cast<Op>(op+1);
  } while ( op != (1+DUP) );

  printf("To halt program, jump to current position:\n\n");
  printf("0x0 PUSH 0x%x\n", sizeof(int32_t));
  printf("0x%x JMP\n\n", sizeof(int32_t));
  printf("Word size is %d bytes\n", sizeof(int32_t));

  exit(0);
}

static void next()
{
  ip += sizeof(int32_t);
  if ( ip > sizeof(memory)/sizeof(int32_t) )
    ip = 0;
}

static void reset()
{
  memset(memory, NOP, sizeof(memory));
  ip = 0;
  stack.clear();
}

static void load(Op op)
{
  memory[ip] = op;
  next();
}

static void load(int32_t n)
{
  memory[ip] = n;
  next();
}

static void init()
{
  ip = 0;
}

int main(int argc, char** argv)
{
  for ( int n=1; n<argc; ++n )
    if ( argv[n][0] == '-' )
      help();

  reset();

  // print message
  load(PUSH); load('H'); load(OUT);
  load(PUSH); load('e'); load(OUT);
  load(PUSH); load('l'); load(DUP); load(OUT); load(OUT);
  load(PUSH); load('o'); load(OUT);
  load(PUSH); load(' '); load(OUT);
  load(PUSH); load('w'); load(OUT);
  load(PUSH); load('o'); load(OUT);
  load(PUSH); load('r'); load(OUT);
  load(PUSH); load('l'); load(OUT);
  load(PUSH); load('d'); load(OUT);
  load(PUSH); load('!'); load(OUT);
  load(PUSH); load('\n'); load(OUT);

  // halt program
  load(PUSH); load(ip+sizeof(int32_t)); load(JMP);

  init();

  for (;;) {
    Op op = static_cast<Op>(memory[ip]);

    int32_t a=NOP, b=NOP;

    if ( stack.size() > 1 )
      a = stack[stack.size()-1];
    if ( stack.size() > 2 )
      b = stack[stack.size()-1];

    fprintf(stderr, "ip=%d op=%s stack(%d) = %d, %d\n",
      ip, to_s(op), stack.size(), a, b);

    a=b=NOP;

    switch(op) {
    case NOP:
      next();
      break;

    case ADD:
      push(pop() + pop());
      next();
      break;

    case SUB:
      a = pop();
      b = pop();
      push(a-b);
      next();
      break;

    case AND:
      push(pop() & pop());
      next();
      break;

    case OR:
      push(pop() | pop());
      next();
      break;

    case XOR:
      push(pop() ^ pop());
      next();
      break;

    case NOT:
      push(!pop());
      next();
      break;

    case IN:
      push(getc(fin));
      next();
      break;

    case OUT:
      putc(pop(), fout);
      next();
      break;

    case LOAD:
      a = pop();
      check_bounds(a, "LOAD");
      push(memory[a]);
      next();
      break;

    case STOR:
      a = pop();
      b = pop();
      check_bounds(a, "STOR");
      memory[a] = b;
      next();
      break;

    case JMP:
      a = pop();
      check_bounds(a, "JMP");  

      // check if we are halting, i.e. jumping to current
      // address -- if so, quit
      if ( a == ip )
        stop("HALT", 0);

      ip = a;
      break;

    case JZ:
      a = pop();

      if ( a != 0 )
        next();
      else {
        check_bounds(a, "JZ");
        ip = a; // jump
      }
      break;

    case PUSH:
      next();
      a = memory[ip];
      push(a);
      next();
      break;

    case DUP:
      a = pop();
      push(a);
      push(a);
      next();
      break;
    } 
  }
}
