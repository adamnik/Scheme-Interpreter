#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"
#include "tokenizer.h"
#include "parser.h"

Frame *global_frame;

void displayVal(Value *val);

void evaluationError() {
  printf("Evaluation Error\n");
  texit(1);
}

//makes top/global frame
Frame *makeTopFrame() {
  Frame *top = talloc(sizeof(Frame));
  top->parent = NULL;
  top->bindings = makeNull();
  return top;
}


//makes new frame; returns new frame
Frame *newFrame(Frame *parent) {
  Frame *frame = talloc(sizeof(Frame));
  frame->parent = parent;
  frame->bindings = makeNull();
  return frame;
}

Value *eval(Value *expr, Frame *frame);

Frame *bind(Value *bindings, Frame *frame) {
  //null binding in let
  if (bindings->c.car->c.cdr->type == NULL_TYPE) {
    evaluationError();
  }
  frame->bindings = bindings;
  return frame;
}

Frame *makeBinds(Value *symbol, Value *val, Frame *frame) {
  Value *bind = makeNull();
  bind = cons(val, bind);
  bind = cons(symbol, bind);
  Value *null_val = makeNull();
  if (frame->bindings->type != NULL_TYPE) {
    frame->bindings = reverse(frame->bindings);
    frame->bindings = cons(bind, frame->bindings);
    frame->bindings = reverse(frame->bindings);
  } else {
    frame->bindings = cons(bind, null_val);
  }
  return frame;
}

//returns SYMBOL_TYPE Value from frame bindings
Value *lookUpSymbol(Value *symbol, Frame frame) {
  Value *result = makeNull(); 
  while (frame.parent != NULL) {
    while (frame.bindings->type == CONS_TYPE) {
      if (!strcmp(frame.bindings->c.car->c.car->s, symbol->s)) {
        return frame.bindings->c.car->c.cdr->c.car;
      }
      frame.bindings = cdr(frame.bindings);
    }
    frame = *frame.parent;
  }
  while (frame.bindings->type == CONS_TYPE) {
    if (!strcmp(frame.bindings->c.car->c.car->s, symbol->s)) {
      return frame.bindings->c.car->c.cdr->c.car;
    }
    frame.bindings = cdr(frame.bindings);
  }
  evaluationError();
  return result;
}

Value *createClosure(Value *params, Value *body, Frame *frame) {
  Value *closure = makeNull();
  closure->type = CLOSURE_TYPE;
  closure->cl.paramNames = params;
  closure->cl.functionCode = body;
  closure->cl.frame = frame;

  return closure;
}

void bind_fn(char *name, Value *(*function)(Value *), Frame *frame) {
  Value *primitiveFn_name = makeNull();
  primitiveFn_name->type = SYMBOL_TYPE;
  primitiveFn_name->s = talloc(sizeof(char) * 301);
  strcpy(primitiveFn_name->s, name);
  Value *primitiveFn_fn = makeNull();
  primitiveFn_fn->type = PRIMITIVE_TYPE;
  primitiveFn_fn->pf = function;
  frame = makeBinds(primitiveFn_name, primitiveFn_fn, frame);
}

Value *primitiveAdd(Value *args) {
  Value *result = makeNull();
  double count = 0;
  bool is_int = true;
  while (args->type != NULL_TYPE) {
    if (args->c.car->type == INT_TYPE) {
      count = count + args->c.car->i;
    }
    else if (args->c.car->type == DOUBLE_TYPE) {
      count = count + args->c.car->d;
      is_int = false;
    }
    else {
      evaluationError();
    }
    args = args->c.cdr;
  }
  if (is_int) {
    int count_int = (int) count;
    result->type = INT_TYPE;
    result->i = count_int;
    return result;
  } else {
    result->type = DOUBLE_TYPE;
    result->d = count;
    return result;
  }
}

Value *primitiveSubtract(Value *args) {
  Value *result = makeNull();
  double count = 0;
  bool is_int = true;
  if (args->c.car->type == INT_TYPE) {
      count = count + args->c.car->i;
    }
    else if (args->c.car->type == DOUBLE_TYPE) {
      count = count + args->c.car->d;
      is_int = false;
    }
    else {
      evaluationError();
    }
    args = args->c.cdr;
  while (args->type != NULL_TYPE) {
    if (args->c.car->type == INT_TYPE) {
      count = count - args->c.car->i;
    }
    else if (args->c.car->type == DOUBLE_TYPE) {
      count = count - args->c.car->d;
      is_int = false;
    }
    else {
      evaluationError();
    }
    args = args->c.cdr;
  }
  if (is_int) {
    int count_int = (int) count;
    result->type = INT_TYPE;
    result->i = count_int;
    return result;
  } else {
    result->type = DOUBLE_TYPE;
    result->d = count;
    return result;
  }
}

Value *primitiveEquals(Value *args) {
  Value *result = makeNull();
  result->type = BOOL_TYPE;
  result->s = talloc(sizeof(char) * 301);
  Value *check = args->c.car;
  if (check->type != INT_TYPE && check->type != DOUBLE_TYPE) {
    evaluationError();
  }
  args = args->c.cdr;
  while (args->type == CONS_TYPE) {
    if (args->c.car->type == INT_TYPE) {
      if (check->i != args->c.car->i) {
        char *text = "#f";
        strcpy(result->s,text);
        return result;
      }
    } else if (args->c.car->type == DOUBLE_TYPE) {
      if (check->d != args->c.car->d) {
        char *text = "#f";
        strcpy(result->s,text);
        return result;
      }
    } else {
      evaluationError();
    }
    args = args->c.cdr;
  }
  char *text = "#t";
  strcpy(result->s,text);
  return result;
}

Value *primitiveGreaterThan(Value *args) {
  Value *result = makeNull();
  result->type = BOOL_TYPE;
  result->s = talloc(sizeof(char) * 301);
  Value *check = args->c.car;
  if (check->type != INT_TYPE && check->type != DOUBLE_TYPE) {
    evaluationError();
  }
  args = args->c.cdr;
  while (args->type == CONS_TYPE) {
    if (args->c.car->type == INT_TYPE) {
      if (check->i <= args->c.car->i) {
        char *text = "#f";
        strcpy(result->s,text);
        return result;
      }
    } else if (args->c.car->type == DOUBLE_TYPE) {
      if (check->d <= args->c.car->d) {
        char *text = "#f";
        strcpy(result->s,text);
        return result;
      }
    } else {
      evaluationError();
    }
    check = args->c.car;
    args = args->c.cdr;
  }
  char *text = "#t";
  strcpy(result->s,text);
  return result;
}

Value *primitiveLessThan(Value *args) {
  Value *result = makeNull();
  result->type = BOOL_TYPE;
  result->s = talloc(sizeof(char) * 301);
  Value *check = args->c.car;
  if (check->type != INT_TYPE && check->type != DOUBLE_TYPE) {
    evaluationError();
  }
  args = args->c.cdr;
  while (args->type == CONS_TYPE) {
    if (args->c.car->type == INT_TYPE) {
      if (check->i >= args->c.car->i) {
        char *text = "#f";
        strcpy(result->s,text);
        return result;
      }
    } else if (args->c.car->type == DOUBLE_TYPE) {
      if (check->d >= args->c.car->d) {
        char *text = "#f";
        strcpy(result->s,text);
        return result;
      }
    } else {
      evaluationError();
    }
    check = args->c.car;
    args = args->c.cdr;
  }
  char *text = "#t";
  strcpy(result->s,text);
  return result;
}

Value *primitiveCar(Value *args) {
  if (args->type == NULL_TYPE) {
    evaluationError();
  } else if (args->c.car->type != CONS_TYPE) {
    evaluationError();
  }
  else {
    return car(args->c.car);
  }
  return 0;
}

Value *primitiveCdr(Value *args) {
  if (args->type == NULL_TYPE) {
    evaluationError();
  } else if (args->c.car->type != CONS_TYPE) {
    evaluationError();
  }
  else {
    return cdr(args->c.car);
  }
  return 0;
}

Value *primitiveNull(Value *args) {
  Value *result = makeNull();
  Value check_args = (*args);
  result->type = BOOL_TYPE;
  result->s = talloc(sizeof(char) * 301);
  if (check_args.type == CONS_TYPE) {
    while(check_args.type == CONS_TYPE) {
      check_args = *check_args.c.car;
    }
  }
  if (check_args.type == NULL_TYPE) {
    char *text = "#t";
    strcpy(result->s,text);
  } else {
    char *text = "#f";
    strcpy(result->s,text);
  }
  return result;
}

Value *primitiveCons(Value *args) {
  return cons(args->c.car, args->c.cdr->c.car);
}

Value *apply(Value *function, Value *args, Frame *frame) {
  Value *result;
  if (function->type == CLOSURE_TYPE) {
    Value *functionCode = function->cl.functionCode;
    Frame *new_frame = newFrame(function->cl.frame);
    while (function->cl.paramNames->c.cdr->type != NULL_TYPE) {
      new_frame = makeBinds(function->cl.paramNames->c.car, eval(args->c.car, frame), new_frame);
      function->cl.paramNames = function->cl.paramNames->c.cdr;
      args = args->c.cdr;
    }
    if (function->cl.paramNames->c.car->type == SYMBOL_TYPE) {
      new_frame = makeBinds(function->cl.paramNames->c.car, eval(args->c.car, frame), new_frame);
    }
    result = eval(functionCode->c.car, new_frame);
  } else if (function->type == PRIMITIVE_TYPE) {
    Value *new_args = makeNull();
    while (args->type != NULL_TYPE) {
      Value *new_car = eval(args->c.car, frame);
      new_args = cons(new_car, new_args);
      args = args->c.cdr;
    }
    new_args = reverse(new_args);
    result = function->pf(new_args);
  }
  return result;
}

//evaluates special form IF statement
Value *evalIf(Value *args,Frame *frame) {
  Value *condition;
  Value *second_arg;
  Value *third_arg;

  //if statements check if we have proper args
  if (args->type != NULL_TYPE) {
    condition = eval(car(args), frame);
  } else {
    evaluationError();
  }
  if (args->c.cdr->type != NULL_TYPE) {
    second_arg = car(cdr(args));
  } else {
    evaluationError();
  }
  if (args->c.cdr->c.cdr->type != NULL_TYPE) {
    third_arg = car(cdr(cdr(args)));
  } else {
    evaluationError();
  }
  if (args->c.cdr->c.cdr->c.cdr->type != NULL_TYPE) {
    evaluationError();
  }
  Value *result;

  if (!strcmp(condition->s,"#t")) {
    result = eval(second_arg, frame);
  } else {
    result = eval(third_arg, frame);
  }

  return result;
}

//evaluates special form LET statement
Value *evalLet(Value *args, Frame *frame) {
  frame = newFrame(frame);
  Value *bindingList;
  Value *symbol;

  //if statements check if we have proper args
  if (args->type != NULL_TYPE) {
    if (args->c.car->c.car->type != NULL_TYPE) {
      bindingList = car(args);
      frame = bind(bindingList, frame);
    }
  } else {
    evaluationError();
  }
  if (args->c.cdr->type != NULL_TYPE) {
    symbol = cdr(args);
  } else {
    evaluationError();
  }
  Value *result;
  while (symbol->type != NULL_TYPE) {
    result = eval(symbol->c.car, frame);
    symbol = symbol->c.cdr;
  }
  return result;
}

//evaluates special form LET* statement
Value *evalLetStar(Value *args, Frame *frame) {
  Value *symbol;

  //if statements check if we have proper args
  if (args->type != NULL_TYPE) {
    if (args->c.car->c.car->type != NULL_TYPE) {
      Value *bindings = car(args);
      while (bindings->type != NULL_TYPE) {
        frame = newFrame(frame);
        frame = makeBinds(bindings->c.car->c.car, eval(bindings->c.car->c.cdr->c.car, frame), frame);
        bindings = bindings->c.cdr;
      }
    }
  } else {
    evaluationError();
  }
  if (args->c.cdr->type != NULL_TYPE) {
    symbol = car(cdr(args));
  } else {
    evaluationError();
  }
  return eval(symbol, frame);
}

//evaluates special form letrec statement
Value *evalLetRec(Value *args, Frame *frame) {
  Value *symbol;
  frame = newFrame(frame);

  //if statements check if we have proper args
  if (args->type != NULL_TYPE) {
    if (args->c.car->c.car->type != NULL_TYPE) {
      Value *bindings = car(args);
      Value *new_bindings = makeNull();
      //evaluate e1...eN
      while (isNull(bindings) == false) {
        bindings->c.car->c.cdr->c.car = eval(bindings->c.car->c.cdr->c.car, frame);
        new_bindings = cons(bindings->c.car, new_bindings);
        bindings = bindings->c.cdr;
      }
      new_bindings = reverse(new_bindings);
      //bind each xI to eI
      while (new_bindings->type != NULL_TYPE) {
        frame = makeBinds(new_bindings->c.car->c.car, new_bindings->c.car->c.cdr->c.car, frame);
        new_bindings = new_bindings->c.cdr;
      }
    }
  } else {
    evaluationError();
  }
  if (args->c.cdr->type != NULL_TYPE) {
    symbol = car(cdr(args));
  } else {
    evaluationError();
  }
  return eval(symbol, frame);
}

//evaluates DEFINE statement
Value *evalDefine(Value *args, Frame *frame) {
  Value *symbol;
  Value *val;
  if (args->type != NULL_TYPE) {
    symbol = car(args);
  } else {
    evaluationError();
  }
  if (args->c.cdr->type != NULL_TYPE) {
    val = eval(car(cdr(args)), global_frame);
  } else {
    evaluationError();
  }

  frame = makeBinds(symbol, val, frame);
  Value *void_val = makeNull();
  void_val->type = VOID_TYPE;
  return void_val;
}

// evaluates SET statement
Value *evalSet(Value *args, Frame *frame) {
  Value *symbol;
  Value *val;
  if (args->type != NULL_TYPE) {
    symbol = car(args);
  } else {
    evaluationError();
  }
  if (args->c.cdr->type != NULL_TYPE) {
    val = eval(car(cdr(args)), frame);
  } else {
    evaluationError();
  }
  bool not_found = true;
  Frame *tempFrame = frame;
  Value *new_bindings;
  while (tempFrame != NULL) {
    Value *tempBinding = tempFrame->bindings;
    new_bindings = makeNull();
    while (tempBinding->type == CONS_TYPE) {
      Value *new_bind = makeNull();
      if (!strcmp(tempBinding->c.car->c.car->s, symbol->s) && not_found) {
        new_bind = cons(val, new_bind);
        not_found = false;
      } else {
        new_bind = cons(tempBinding->c.car->c.cdr->c.car, new_bind);
      }
      new_bind = cons(tempBinding->c.car->c.car, new_bind);
      new_bindings = cons(new_bind, new_bindings);
      tempBinding = cdr(tempBinding);
    }
    new_bindings = reverse(new_bindings);
    tempFrame->bindings = new_bindings;
    tempFrame = tempFrame->parent;
  }
  if (not_found) {
    evaluationError();
  }
  Value *void_val = makeNull();
  void_val->type = VOID_TYPE;
  return void_val;
}

Value *evalBegin(Value *args, Frame *frame) {
  Value *result;
  while (args->type != NULL_TYPE) {
    result = eval(args->c.car, frame);
    args = args->c.cdr;
  }
  return result;
}

Value *evalAnd(Value *args, Frame *frame) {
  Value *result = makeNull();
  result->type = BOOL_TYPE;
  result->s = talloc(sizeof(char) * 301);
  while (args->type == CONS_TYPE) {
    Value *check_arg = eval(args->c.car, frame);
    if (check_arg->type != BOOL_TYPE) {
      evaluationError();
    } else {
      if (strcmp(check_arg->s,"#t")) {
        char *text = "#f";
        strcpy(result->s,text);
        return result;
      }
    }
    args = args->c.cdr;
  }
  char *text = "#t";
  strcpy(result->s,text);
  return result;
}

Value *evalOr(Value *args, Frame *frame) {
  Value *result = makeNull();
  result->type = BOOL_TYPE;
  result->s = talloc(sizeof(char) * 301);
  while (args->type == CONS_TYPE) {
    Value *check_arg = eval(args->c.car, frame);
    if (check_arg->type != BOOL_TYPE) {
      evaluationError();
    } else {
      if (strcmp(check_arg->s,"#f")) {
        char *text = "#t";
        strcpy(result->s,text);
        return result;
      }
    }
    args = args->c.cdr;
  }
  char *text = "#f";
  strcpy(result->s,text);
  return result;
}

Value *eval(Value *expr, Frame *frame) {

  switch (expr->type) {
    case NULL_TYPE:
      break;

    case PTR_TYPE:
      break;

    case CONS_TYPE: {
      Value *first = car(expr);
      Value *args = cdr(expr);
      Value *result;

      if (!strcmp(first->s,"if")) {
        Value *result = evalIf(args, frame);
        return result;
      }

      else if (!strcmp(first->s,"let")) {
        Value *result = evalLet(args, frame);
        return result;
      }

      else if (!strcmp(first->s,"let*")) {
        Value *result = evalLetStar(args, frame);
        return result;
      }

      else if (!strcmp(first->s,"letrec")) {
        Value *result = evalLetRec(args, frame);
        return result;
      }

      else if (!strcmp(first->s,"define")) {
        Value *result = evalDefine(args, frame);
        return result;
      }

      else if (!strcmp(first->s,"set!")) {
        Value *result = evalSet(args, frame);
        return result;
      }

      else if (!strcmp(first->s,"quote")) {
        if (args->type == NULL_TYPE) {
          evaluationError();
        }
        if (args->c.cdr->type == NULL_TYPE) {
          return args->c.car;
        }
        else {
          evaluationError();
        }
      }

      else if (!strcmp(first->s,"begin")) {
        Value *result = evalBegin(args, frame);
        return result;
      }

      else if (!strcmp(first->s,"lambda")) {
        return createClosure(car(args), cdr(args), frame);
      }

      else if (!strcmp(first->s,"and")) {
        Value *result = evalAnd(args, frame);
        return result;
      }

      else if (!strcmp(first->s,"or")) {
        Value *result = evalOr(args, frame);
        return result;
      }

      else {
        Value *applied = eval(first, frame);
        return apply(applied, args, frame);
      }

      break;
    }

    case INT_TYPE:
      return expr;
      break;

    case DOUBLE_TYPE:
      return expr;
      break;

    case STR_TYPE:
      return expr;
      break;

    case BOOL_TYPE:
      return expr;
      break;

    case OPEN_TYPE:
      break;

    case CLOSE_TYPE:
      break;

    case SYMBOL_TYPE: {
      return lookUpSymbol(expr, (*frame));
      break;
    }

    case CLOSURE_TYPE:
      return expr;
      break;

    case VOID_TYPE:
      break;

    case PRIMITIVE_TYPE:
      return expr;
      break;

  }

    return 0;
}

void displayVal(Value *val) {
  switch (val->type) {
    case NULL_TYPE:
      break;
    case PTR_TYPE:
      break;
    case CONS_TYPE:
      printf("(");
      printTree(val);
      printf(")");
      break;
    case INT_TYPE:
      printf("%i ", val->i);
      break;
    case DOUBLE_TYPE:
      printf("%f ", val->d);
      break;
    case STR_TYPE:
      printf("%s ", val->s);
      break;
    case BOOL_TYPE:
      printf("%s ", val->s);
      break;
    case OPEN_TYPE:
      break;
    case CLOSE_TYPE:
      break;
    case SYMBOL_TYPE:
      printf("%s ", val->s);
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

void interpret(Value *tree) {
  global_frame = makeTopFrame();

  bind_fn("+", primitiveAdd, global_frame);
  bind_fn("car", primitiveCar, global_frame);
  bind_fn("cdr", primitiveCdr, global_frame);
  bind_fn("null?", primitiveNull, global_frame);
  bind_fn("cons", primitiveCons, global_frame);
  bind_fn("-", primitiveSubtract, global_frame);
  bind_fn("=", primitiveEquals, global_frame);
  bind_fn(">", primitiveGreaterThan, global_frame);
  bind_fn("<", primitiveLessThan, global_frame);

  while (tree->type == CONS_TYPE) {
    Value *val = eval(car(tree), global_frame);
    if (val->type != VOID_TYPE) {
      displayVal(val);
      printf("\n");
    }
    tree = cdr(tree);
  }
}

int main() {

    Value *list = tokenize();
    Value *tree = parse(list);
    interpret(tree);

    tfree();
    return 0;
}