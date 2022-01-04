#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"

// Read all of the input from stdin, and return a linked list consisting of the
// tokens.
Value *tokenize(){
  char charRead;
  Value *list = makeNull();
  charRead = (char)fgetc(stdin);
  bool tokenized = false;

  while (charRead != EOF) {
    // place holder to copy token
    Value *val = talloc(sizeof(Value));
    char text[300] = "";
    val->s = talloc(sizeof(char) * 301);

    //SYMBOL checking
    char initial[] = "!$%&*/:<=>?~_^";
    char subsequent[] = "!$%&*/:<=>?~_^.+-";
    char *check;
    check = strchr(initial, charRead);

    // comment
    if (charRead == ';') {
      while (charRead != '\n') {
        charRead = (char)fgetc(stdin);
      }
    }

    // BOOL
    else if (charRead == '#') {
      charRead = (char)fgetc(stdin);
      if (charRead == 't') {
        //true
        val->type = BOOL_TYPE;
        char *text = "#t";
        strcpy(val->s,text);
        list = cons(val,list);
        tokenized = true;

      } else if (charRead == 'f') {
        //false
        val->type = BOOL_TYPE;
        char *text = "#f";
        strcpy(val->s,text);
        list = cons(val,list);
        tokenized = true;
        
      } else {
        printf("Syntax error (readBoolean): boolean was not #t or #f\n");
        texit(1);
      }
    }

    // STRING
    else if (charRead == '"') {
      val->type = STR_TYPE;
      strncat(text, &charRead, 1);
      charRead = (char)fgetc(stdin);
      while (charRead != '"') {
        strncat(text, &charRead, 1);
        charRead = (char)fgetc(stdin);
      }
      strncat(text, &charRead, 1);
      strcpy(val->s,text);
      list = cons(val,list);
      tokenized = true;
    }

    //SYMBOL
    else if (check != NULL || isalpha(charRead)) {
      strncat(text, &charRead, 1);
      charRead = (char)fgetc(stdin);
      check = strchr(subsequent, charRead);
      while (isdigit(charRead) || check != NULL || isalpha(charRead)) {
        strncat(text, &charRead, 1);
        charRead = (char)fgetc(stdin);
        check = strchr(subsequent, charRead);
      }
      val->type = SYMBOL_TYPE;
      strcpy(val->s,text);
      list = cons(val,list);
      tokenized = true;
      ungetc(charRead, stdin);
    }

    // OPEN
    else if (charRead == '(') {
    val->type = OPEN_TYPE;
    char *text = "(";
    strcpy(val->s,text);
    list = cons(val,list);
    tokenized = true;
    }

    // CLOSE
    else if (charRead == ')') {
    val->type = CLOSE_TYPE;
    char *text = ")";
    strcpy(val->s,text);
    list = cons(val,list);
    tokenized = true;
    } 

    // sign 
    if (charRead == '+' || charRead == '-') {
      char sign;
      if (charRead == '+') {
        sign = '+';
      } else {
        sign = '-';
      }
      charRead = (char)fgetc(stdin);
      if (isdigit(charRead) || charRead == '.') {
        ungetc(charRead, stdin);
        strncat(text, &sign, 1);
        charRead = (char)fgetc(stdin);
      }
      else {
        ungetc(charRead, stdin);
        val->type = SYMBOL_TYPE;
        strncat(text, &sign, 1);
        strcpy(val->s,text);
        list = cons(val,list);
        tokenized = true;
      }
  
    }

    // INT or DOUBLE
    if (isdigit(charRead)) {
      strncat(text, &charRead, 1);
      charRead = (char)fgetc(stdin);
      while (isdigit(charRead)) {
        strncat(text, &charRead, 1);
        charRead = (char)fgetc(stdin);
      }

      // DOUBLE
      if (charRead == '.') {
        strncat(text, &charRead, 1);
        char *eptr;
        charRead = (char)fgetc(stdin);
        while (isdigit(charRead)) {
          strncat(text, &charRead, 1);
          charRead = (char)fgetc(stdin);
        }
        strcpy(val->s,text);
        val->type = DOUBLE_TYPE;
        val->d = strtod(val->s, &eptr);
        list = cons(val,list);
        tokenized = true;
      } 
      // INT
      else {
        strcpy(val->s,text);
        val->type = INT_TYPE;
        val->i = atoi(val->s);
        list = cons(val,list);
        tokenized = true;
      }
      ungetc(charRead, stdin);
    }

    // .<digit>* format DOUBLE
    if (charRead == '.') {
      charRead = (char)fgetc(stdin);
      if (isdigit(charRead)) {
        char *eptr;
        char period = '.';
        strncat(text, &period, 1);
        while (isdigit(charRead)) {
          strncat(text, &charRead, 1);
          charRead = (char)fgetc(stdin);
        }
        strcpy(val->s,text);
        val->type = DOUBLE_TYPE;
        val->d = strtod(val->s, &eptr);
        list = cons(val,list);
        tokenized = true;
        ungetc(charRead, stdin);
      }
    }

    // error checking
    if (tokenized == false && charRead != ' ' && charRead != '\n' && charRead != '\t' && charRead != EOF) {
      printf("Syntax Error: unable to tokenize.\n");
      texit(1);
    }

    charRead = (char)fgetc(stdin);

  }

  Value *revList = reverse(list);
  return revList;

}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list){
  if (list->type == CONS_TYPE) {
    switch (list->c.car->type) {
      case NULL_TYPE:
        break;
      case PTR_TYPE:
        break;
      case CONS_TYPE:
        break;
      case INT_TYPE:
        printf("%i:integer\n", list->c.car->i);
        displayTokens(list->c.cdr);
        break;
      case DOUBLE_TYPE:
        printf("%f:double\n", list->c.car->d);
        displayTokens(list->c.cdr);
        break;
      case STR_TYPE:
        printf("%s:string\n", list->c.car->s);
        displayTokens(list->c.cdr);
        break;
      case BOOL_TYPE:
        printf("%s:boolean\n", list->c.car->s);
        displayTokens(list->c.cdr);
        break;
      case OPEN_TYPE:
        printf("%s:open\n", list->c.car->s);
        displayTokens(list->c.cdr);
        break;
      case CLOSE_TYPE:
        printf("%s:close\n", list->c.car->s);
        displayTokens(list->c.cdr);
        break;
      case SYMBOL_TYPE:
        printf("%s:symbol\n", list->c.car->s);
        displayTokens(list->c.cdr);
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