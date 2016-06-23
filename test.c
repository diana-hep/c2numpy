#include <stdio.h>
#include <stdlib.h>

#include "c2numpy.h"

int main(int argc, char **argv) {
  printf("hello\n");

  c2numpy_writer writer;

  c2numpy_init(&writer, "testout", 5);
  c2numpy_addcolumn(&writer, "one", C2NUMPY_INTC);
  c2numpy_addcolumn(&writer, "two", C2NUMPY_FLOAT64);
  c2numpy_addcolumn(&writer, "three", C2NUMPY_INTC);

  printf("row %d\n", writer.currentRowInFile);
  c2numpy_row(&writer, 1, 1.1, 1);

  printf("row %d\n", writer.currentRowInFile);
  c2numpy_row(&writer, 2, 2.2, 2);

  printf("row %d by separate calls\n", writer.currentRowInFile);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_intc(&writer, 3);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_float64(&writer, 3.3);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_intc(&writer, 3);

  printf("row %d by separate calls\n", writer.currentRowInFile);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_intc(&writer, 4);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_float64(&writer, 4.4);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_intc(&writer, 4);

  printf("row %d\n", writer.currentRowInFile);
  c2numpy_row(&writer, 5, 5.5, 5);

  printf("row %d\n", writer.currentRowInFile);
  c2numpy_row(&writer, 6, 6.6, 6);

  printf("row %d\n", writer.currentRowInFile);
  c2numpy_row(&writer, 7, 7.7, 7);

  printf("row %d by separate calls\n", writer.currentRowInFile);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_intc(&writer, 8);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_float64(&writer, 8.8);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_intc(&writer, 8);

  printf("close\n");
  c2numpy_close(&writer);
}
