#ifndef PARSEJEVKO_H
#define PARSEJEVKO_H

#include <stdio.h>
#include <stdbool.h>
#include "lib.h"

#define runtime_error(x) printf("%s\n", x); exit(1);

typedef struct Jevko Jevko;
typedef struct Subjevko Subjevko;

struct Jevko {
  List* subjevkos;
  String* suffix;
};

struct Subjevko {
  String* prefix;
  Jevko* jevko;
};

const char opener = '[';
const char closer = ']';
const char escaper = '`';

Jevko* new_Jevko();
inline Jevko* new_Jevko() {
  Jevko* jevko = (Jevko*)malloc(sizeof *jevko);
  jevko->subjevkos = List_make();
  jevko->suffix = String_make();
  return jevko;
}

Subjevko* new_Subjevko(String* prefix, Jevko* jevko);
inline Subjevko* new_Subjevko(String* prefix, Jevko* jevko) {
  Subjevko* subjevko = (Subjevko*)malloc(sizeof *subjevko);
  subjevko->prefix = prefix;
  subjevko->jevko = jevko;
  return subjevko;
}

void delete_Subjevko(Subjevko** subjevko);
void delete_Jevko(Jevko** jevko);

inline void delete_Jevko(Jevko** jevkoptr) {
  Jevko* jevko = *jevkoptr;
  List_free(&jevko->subjevkos, (void (*)(void**))&delete_Subjevko);
  String_free(&jevko->suffix);
  free(jevko);
  jevkoptr = NULL;
}

inline void delete_Subjevko(Subjevko** subjevkoptr) {
  Subjevko* subjevko = *subjevkoptr;
  String_free(&subjevko->prefix);
  delete_Jevko(&subjevko->jevko);
  free(subjevko);
  subjevkoptr = NULL;
}

String* jevkoToString(Jevko* jevko);

String* escape(String* str);
inline String* escape(String* str) {
  String* ret = String_make();
  for (int i = 0; i < str->length; ++i) {
    char c = str->str[i];
    if (c == opener || c == closer || c == escaper) {
      String_append_c(ret, escaper);
    }
    String_append_c(ret, c);
  }
  return ret;
}

String* subjevkoToString(Subjevko* subjevko);
inline String* subjevkoToString(Subjevko* subjevko) {
  String* ret = String_make();
  String_append_d(ret, escape(subjevko->prefix));
  String_append_c(ret, opener);
  String_append_d(ret, jevkoToString(subjevko->jevko));
  String_append_c(ret, closer);
  return ret;
}

inline String* jevkoToString(Jevko* jevko) {
  String* ret = String_make();
  List* subjevkos = jevko->subjevkos;
  Node* node = List_head(subjevkos);
  while (Node_isValid(node)) {
    String_append_d(ret, subjevkoToString((Subjevko*)node->value));
    node = Node_next(node);
  }
  String_append_d(ret, escape(jevko->suffix));
  return ret;
}

Jevko* parseJevko(String* str);
inline Jevko* parseJevko(String* str) {
  Stack* ancestors = Stack_make();

  Jevko* parent = new_Jevko();
  String* text = String_make();
  bool isEscaped = false;

  unsigned int line = 1;
  unsigned int column = 1;

  for (int i = 0; i < str->length; ++i) {
    char chr = str->str[i];
    if (isEscaped) {
      if (chr == escaper || chr == opener || chr == closer) {
        String_append_c(text, chr);
        isEscaped = false;
      } else {
        String* str = String_make();
        String_append_ccstr(str, "Invalid digraph (");
        String_append_c(str, escaper);
        String_append_c(str, chr);
        String_append_ccstr(str, ") at ");
        String_append_i(str, line);
        String_append_ccstr(str, ":");
        String_append_i(str, column);
        String_append_ccstr(str, "!");
        runtime_error(String_cstr(str));
      }
    } else if (chr == escaper) {
      isEscaped = true;
    } else if (chr == opener) {
      Jevko* jevko = new_Jevko();
      Subjevko* sub = new_Subjevko(text, jevko);
      List_add(parent->subjevkos, sub);
      Stack_push(ancestors, parent);
      parent = jevko;
      text = String_make();
    } else if (chr == closer) {
      String_append_d(parent->suffix, text);
      text = String_make();
      if (Stack_size(ancestors) < 1) {
        String* str = String_make();
        String_append_ccstr(str, "Unexpected closer (");
        String_append_c(str, closer);
        String_append_ccstr(str, ") at ");
        String_append_i(str, line);
        String_append_ccstr(str, ":");
        String_append_i(str, column);
        String_append_ccstr(str, "!");
        runtime_error(String_cstr(str));
      }
      parent = (Jevko*)Stack_peek(ancestors);
      Stack_pop(ancestors);
    } else {
      String_append_c(text, chr);
    }

    if (chr == '\n') {
      line += 1;
      column = 1;
    } else {
      column += 1;
    }
  }
  if (isEscaped) {
    String* str = String_make();
    String_append_ccstr(str, "Unexpected end after escaper (");
    String_append_c(str, escaper);
    String_append_ccstr(str, ")!");
    runtime_error(String_cstr(str));
  }
  if (Stack_size(ancestors) > 0) {
    String* str = String_make();
    String_append_ccstr(str, "Unexpected end: missing ");
    String_append_i(str, Stack_size(ancestors));
    String_append_ccstr(str, " closer(s) (");
    String_append_c(str, closer);
    String_append_ccstr(str, ")!");
    runtime_error(String_cstr(str));
  }
  String_append_d(parent->suffix, text);
  Stack_free(&ancestors);
  return parent;
}

#endif