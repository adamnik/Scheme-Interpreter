#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"
#include "tokenizer.h"

// Takes a list of tokens from a Scheme program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens){
  Value *tree = makeNull();
  int paren_count = 0;

  while (tokens->type == CONS_TYPE) {

    //pop off stack until open parenthesis
    if (tokens->c.car->type == CLOSE_TYPE) {
      paren_count--;
      Value *subtree = makeNull();

      if (tree->c.car->type != OPEN_TYPE) {
        while (tree->c.car->type != OPEN_TYPE) {
          subtree = cons(tree->c.car, subtree);
          //pop off items from stack as we add them to new subtree
          tree = tree->c.cdr;
          //we can't find matching open paren
          if (tree->type == NULL_TYPE) {
            printf("Syntax error: too many close parentheses\n");
            texit(1);
          }
        }
      }
      //empty parentheses
      else {
        Value *null_node = makeNull();
        subtree = cons(null_node, subtree);
      }
      //pop off final open paren from stack
      tree = tree->c.cdr;
      //push subtree to stack
      tree = cons(subtree, tree);
    }

    //push tokens to stack
    else {
      if (tokens->c.car->type == OPEN_TYPE) {
        paren_count++;
      }
      tree = cons(tokens->c.car, tree);
    }

    //pop off now read token
    tokens = tokens->c.cdr;
  }

  //too many open paren
  if (paren_count != 0) {
    printf("Syntax error: not enough close parentheses\n");
    texit(1);
  }
  Value *revTree = reverse(tree);
  return revTree;
}


// Prints the tree to the screen in a readable fashion. It should look just like
// Scheme code; use parentheses to indicate subtrees.
void printTree(Value *tree){

  if (tree->type != CONS_TYPE) {
      printf(". ");
      switch (tree->type) {
        case NULL_TYPE:
          printf("NULL");
          break;
        case PTR_TYPE:
          break;
        case CONS_TYPE:
          break;
        case INT_TYPE:
          printf("%i", tree->i);
          break;
        case DOUBLE_TYPE:
          printf("%f", tree->d);
          break;
        case STR_TYPE:
          printf("%s", tree->s);
          break;
        case BOOL_TYPE:
          printf("%s", tree->s);
          break;
        case OPEN_TYPE:
          printf("%s", tree->s);
          break;
        case CLOSE_TYPE:
          printf("%s", tree->s);
          break;
        case SYMBOL_TYPE:
          printf("%s", tree->s);
          break;
        case CLOSURE_TYPE:
          printf("CLOSURE");
          break;
        case VOID_TYPE:
          break;
        case PRIMITIVE_TYPE:
          printf("PRIMITIVE");
          break;
    }
  }
  else if (tree->type == CONS_TYPE) {
    if (tree->c.car->type == CONS_TYPE) {
      printf("(");
      printTree(tree->c.car);
      printf(")");
    }
    switch (tree->c.car->type) {
      case NULL_TYPE:
        printf("NULL");
        break;
      case PTR_TYPE:
        break;
      case CONS_TYPE:
        break;
      case INT_TYPE:
        printf("%i", tree->c.car->i);
        break;
      case DOUBLE_TYPE:
        printf("%f", tree->c.car->d);
        break;
      case STR_TYPE:
        printf("%s", tree->c.car->s);
        break;
      case BOOL_TYPE:
        printf("%s", tree->c.car->s);
        break;
      case OPEN_TYPE:
        printf("%s", tree->c.car->s);
        break;
      case CLOSE_TYPE:
        printf("%s", tree->c.car->s);
        break;
      case SYMBOL_TYPE:
        printf("%s", tree->c.car->s);
        break;
      case CLOSURE_TYPE:
        printf("CLOSURE");
        break;
      case VOID_TYPE:
        break;
      case PRIMITIVE_TYPE:
        printf("PRIMITIVE");
        break;
      }
    }
  if (tree->type == CONS_TYPE) {
    if (tree->c.cdr->type != NULL_TYPE) {
      printf(" ");
      printTree(tree->c.cdr);
    }
  }
}
