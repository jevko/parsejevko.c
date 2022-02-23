#include <stdio.h>
#include <assert.h>

#include "parseJevko.h"

int main() {
  String* instr = String_from_cstr("a`[`]`` [b [x [y]] z] c [d] eaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  Jevko* jevko = parseJevko(instr);
  String* outstr = Jevko_toString(jevko);

  assert(strcmp(String_cstr(instr), String_cstr(outstr)) == 0);

  printf("%s\n", String_cstr(outstr));

  delete_Jevko(&jevko);
  String_free(&instr);
  String_free(&outstr);

  return 0;
}