#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"

// Create a new NULL_TYPE value node.
Value *makeNull(){
  Value *value = talloc(sizeof(Value));
  value->type = NULL_TYPE;
  return value;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *newCar, Value *newCdr){
  Value *value = talloc(sizeof(Value));
  value->type = CONS_TYPE;
  value->c.car = newCar;
  value->c.cdr = newCdr;
  return value;
}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list){
  if (list->type == CONS_TYPE) {
    switch (list->c.car->type) {
      case NULL_TYPE:
        break;
      case PTR_TYPE:
        break;
      case CONS_TYPE:
        printf("(");
        display(list->c.car);
        display(list->c.cdr);
        printf(")");
        break;
      case INT_TYPE:
        printf("%i ", list->c.car->i);
        display(list->c.cdr);
        break;
      case DOUBLE_TYPE:
        printf("%f ", list->c.car->d);
        display(list->c.cdr);
        break;
      case STR_TYPE:
        printf("%s ", list->c.car->s);
        display(list->c.cdr);
        break;
      case BOOL_TYPE:
        printf("%s ", list->c.car->s);
        display(list->c.cdr);
        break;
      case OPEN_TYPE:
        printf("%s ", list->c.car->s);
        display(list->c.cdr);
        break;
      case CLOSE_TYPE:
        printf("%s ", list->c.car->s);
        display(list->c.cdr);
        break;
      case SYMBOL_TYPE:
        printf("%s ", list->c.car->s);
        display(list->c.cdr);
        break;
      case CLOSURE_TYPE:
        break;
      case VOID_TYPE:
        break;
      case PRIMITIVE_TYPE:
        break;
    }
  }
}

// Return a new list that is the reverse of the one that is passed in. No stored
// data within the linked list should be duplicated; rather, a new linked list
// of CONS_TYPE nodes should be created, that point to items in the original
// list.
Value *reverse(Value *list){
  Value *head = makeNull();
  while(list->type != NULL_TYPE) {
    Value *val = list->c.car;
    head = cons(val,head);
    if (list->type == CONS_TYPE) {
      list = list->c.cdr;
    }
  }
  return head;
}

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list){
  assert(list != NULL);
  return list->c.car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list){
  assert(list != NULL);
  return list->c.cdr;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value){
  assert(value != NULL);
  if (value->type == NULL_TYPE){
    return true;
  } 
  else {
    return false;
  }
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value){
  assert(value != NULL);
  int length = 0;
  while (value->type != NULL_TYPE){
    length++;
    value = value->c.cdr;
  }
  return length;
}