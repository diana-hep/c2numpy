#include <stdio.h>
#include <stdlib.h>

#include "c2numpy.h"

int main(int argc, char **argv) {
  printf("hello\n");

  c2numpy_writer writer;

  c2numpy_init(&writer, "testout", -1);
  c2numpy_addcolumn(&writer, "one", C2NUMPY_INTC);

  printf("open\n");
  c2numpy_open(&writer);


  printf("row\n");
  c2numpy_fill(&writer, 1);

  printf("row\n");
  c2numpy_fill(&writer, 2);

  printf("row\n");
  c2numpy_fill(&writer, 3);

  printf("row\n");
  c2numpy_fill(&writer, 4);

  printf("row\n");
  c2numpy_fill(&writer, 5);


  printf("close\n");
  c2numpy_close(&writer);


}
