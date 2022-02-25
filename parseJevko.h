#ifndef PARSEJEVKO_H
#define PARSEJEVKO_H

#include <stdio.h>
#include <stdbool.h>
#include "lib.h"

typedef struct Jevko Jevko;
typedef struct Subjevko Subjevko;

struct Jevko {
  Vector* subjevkos;
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
  jevko->subjevkos = Vector_make();
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
  Vector_free(&jevko->subjevkos, (void (*)(void**))&delete_Subjevko);
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

String* Jevko_toString(Jevko* jevko);

String* escape(String* str);
inline String* escape(String* str) {
  String* ret = String_make();
  for (int i = 0; i < String_length(str); ++i) {
    char c = String_cstr(str)[i];
    if (c == opener || c == closer || c == escaper) {
      String_append_c(ret, escaper);
    }
    String_append_c(ret, c);
  }
  return ret;
}

String* Subjevko_toString(Subjevko* subjevko);
inline String* Subjevko_toString(Subjevko* subjevko) {
  String* ret = String_make();
  String_append_d(ret, escape(subjevko->prefix));
  String_append_c(ret, opener);
  String_append_d(ret, Jevko_toString(subjevko->jevko));
  String_append_c(ret, closer);
  return ret;
}

inline String* Jevko_toString(Jevko* jevko) {
  String* ret = String_make();
  Vector* subjevkos = jevko->subjevkos;
  Subjevko** data = (Subjevko**)Vector_data(subjevkos);
  for (int i = 0; i < Vector_length(subjevkos); ++i) {
    String_append_d(ret, Subjevko_toString(data[i]));
  }
  String_append_d(ret, escape(jevko->suffix));
  return ret;
}

Jevko* parseJevko(String* str);
inline Jevko* parseJevko(String* str) {
  Vector* ancestors = Vector_make();

  Jevko* parent = new_Jevko();
  String* text = String_make();
  bool isEscaped = false;

  unsigned int line = 1;
  unsigned int column = 1;

  for (int i = 0; i < String_length(str); ++i) {
    char chr = String_cstr(str)[i];
    if (isEscaped) {
      if (chr == escaper || chr == opener || chr == closer) {
        String_append_c(text, chr);
        isEscaped = false;
      } else {
        printf(
          "Invalid digraph (%c%c) at %d:%d!\n", escaper, chr, line, column
        );
        exit(1);
      }
    } else if (chr == escaper) {
      isEscaped = true;
    } else if (chr == opener) {
      Jevko* jevko = new_Jevko();
      Subjevko* sub = new_Subjevko(text, jevko);
      Vector_push(parent->subjevkos, sub);
      Vector_push(ancestors, parent);
      parent = jevko;
      text = String_make();
    } else if (chr == closer) {
      String_append_d(parent->suffix, text);
      text = String_make();
      if (Vector_length(ancestors) < 1) {
        printf("Unexpected closer (%c) at %d:%d!\n", closer, line, column);
        exit(1);
      }
      parent = (Jevko*)Vector_pop(ancestors);
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
    printf("Unexpected end after escaper (%c)!\n", escaper);
    exit(1);
  }
  if (Vector_length(ancestors) > 0) {
    printf(
      "Unexpected end: missing %d closer(s) (%c)!\n", 
      Vector_length(ancestors), 
      closer
    );
    exit(1);
  }
  String_append_d(parent->suffix, text);
  Vector_free(&ancestors, (void (*)(void**))&delete_Jevko);
  return parent;
}

#include <stdarg.h>

#define J(...) 1l, __VA_ARGS__, -1l

Jevko* argsToJevko(int one, ...);
inline Jevko* argsToJevko(int one, ...) {
  va_list args;
  va_start(args, one);

  if (one != 1l) {
    printf("oops: todo %d", one);
    exit(1);
  }
  long level = one;

  Vector* ancestors = Vector_make();

  Jevko* parent = new_Jevko();
  String* text = String_make();
  bool isEscaped = false;

  void* arg;
  // note: might cause a segfault if unbalanced
  while (true) {
    arg = va_arg(args, void*);

    if ((long)arg == 1l) {
      level += 1;
      Jevko* jevko = new_Jevko();
      Subjevko* sub = new_Subjevko(text, jevko);
      Vector_push(parent->subjevkos, sub);
      Vector_push(ancestors, parent);
      parent = jevko;
      text = String_make();
    } else if ((long)arg == -1l) {
      level -= 1;
      if (level == 0) break;
      String_append_d(parent->suffix, text);
      text = String_make();
      if (Vector_length(ancestors) < 1) {
        printf("Unexpected closer (%c)!\n", closer);
        exit(1);
      }
      parent = (Jevko*)Vector_pop(ancestors);
    } else if ((long)arg == 0) {
      // nothing
    } else {
      // printf("LEVEL %ld, %ld, %s", level, (long)arg, (char*)arg);
      String_append_cstr(text, (char*)arg);
    }
  }
  if (Vector_length(ancestors) > 0) {
    printf(
      "Unexpected end: missing %d closer(s) (%c)!\n", 
      Vector_length(ancestors), 
      closer
    );
    exit(1);
  }
  String_append_d(parent->suffix, text);
  Vector_free(&ancestors, (void (*)(void**))&delete_Jevko);
  return parent;
}

#endif