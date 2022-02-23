#ifndef PARSEJEVKO_LIB_H
#define PARSEJEVKO_LIB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef int Bool;

extern inline void freeptr(void** ptr) {
  void* value = *ptr;
  free(value);
  ptr = NULL;
}

typedef struct {
  unsigned int capacity;
  unsigned int length;
  void** data;
} Vector;
Vector* Vector_make();
void Vector_push(Vector* vector, void* value);
void* Vector_pop(Vector* vector);
void** Vector_data(Vector* vector);
unsigned int Vector_length(Vector* vector);
void Vector_free(Vector** vectorptr, void (*value_free)(void**));

typedef struct {
  unsigned int capacity;
  unsigned int length;
  char* str;
} String;
String* String_make();
String* String_from_cstr(char* cstr);
void String_free(String** stringptr);
String* String_make_c(char c);
void String_append(String* string, String* s);
void String_append_cstr(String* string, char* s);
void String_append_ccstr(String* string, const char* s);
void String_append_d(String* string, String* s);
void String_append_c(String* string, char c);
char* String_cstr(String* string);
unsigned int String_length(String* string);

inline Vector* Vector_make() {
  Vector* vector = (Vector*)malloc(sizeof *vector);
  unsigned int capacity = 64;
  vector->capacity = capacity;
  vector->length = 0;
  vector->data = (void**)malloc(capacity * sizeof(void*));
  return vector;
}
inline void Vector_push(Vector* vector, void* value) {
  int length = vector->length;
  if (length >= vector->capacity) {
    int capacity = vector->capacity * 2;
    vector->data = (void**)realloc(vector->data, capacity);
    vector->capacity = capacity;
  }
  vector->data[length] = value;
  vector->length = length + 1;
}
inline void* Vector_pop(Vector* vector) {
  vector->length -= 1;
  return vector->data[vector->length];
}
inline void** Vector_data(Vector* vector) {
  return vector->data;
}
inline unsigned int Vector_length(Vector* vector) {
  return vector->length;
}
inline void Vector_free(Vector** vectorptr, void (*value_free)(void**)) {
  Vector* vector = *vectorptr;
  void** data = vector->data;
  for (unsigned int i = 0; i < vector->length; ++i) {
    value_free(&data[i]);
  }
  free(data);
  free(vector);
  vectorptr = NULL;
}

inline String* String_make() {
  String* ret = (String*)malloc(sizeof *ret);
  int capacity = 64;
  ret->str = (char*)calloc(capacity, sizeof(char));
  ret->capacity = capacity;
  ret->length = 0;
  return ret;
}
inline String* String_from_cstr(char* cstr) {
  String* ret = String_make();
  String_append_cstr(ret, cstr);
  return ret;
}
inline void String_free(String** stringptr) {
  String* string = *stringptr;
  free(string->str);
  free(string);
  stringptr = NULL;
}
inline String* String_make_c(char c) {
  String* ret = String_make();
  String_append_c(ret, c);
  return ret;
}
inline void String_append_c(String* string, char c) {
  int length = string->length;
  if (length >= string->capacity - 1) {
    int capacity = string->capacity * 2;
    string->str = (char*)realloc(string->str, capacity);
    string->capacity = capacity;
  }
  string->str[length] = c;
  string->length = length + 1;
  string->str[length + 1] = 0;
}
inline void String_append(String* string, String* s) {
  String_append_cstr(string, String_cstr(s));
}
inline void String_append_cstr(String* string, char* s) {
  for (int i = 0; i < strlen(s); ++i) {
    String_append_c(string, s[i]);
  }
}
inline void String_append_ccstr(String* string, const char* s) {
  for (int i = 0; i < strlen(s); ++i) {
    String_append_c(string, s[i]);
  }
}
inline void String_append_d(String* string, String* s) {
  String_append(string, s);
  String_free(&s);
}
extern inline char* String_to_cstr(String* string) {
  char* ret = (char*)calloc(string->length + 1, sizeof(char));
  strcpy(ret, string->str);
  return ret;
}
inline char* String_cstr(String* string) {
  return string->str;
}
inline unsigned int String_length(String* string) {
  return string->length;
}

#endif