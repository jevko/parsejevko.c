#include <stdio.h>

#include "parseJevko.h"

int main() {
  String* str = String_make();
  String_append_ccstr(str, "a`[`]`` [b [x [y]] z] c [d] `e");
  Jevko* jevko = parseJevko(str);
  String* outstr = jevkoToString(jevko);
  printf("%s", String_cstr(outstr)) ;
  delete_Jevko(&jevko);

  String_free(&outstr);
  String_free(&str);

  return 0;
}