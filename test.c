// Copyright 2016 Jim Pivarski
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdlib.h>

#include "c2numpy.h"

int main(int argc, char **argv) {
  printf("start\n");

  c2numpy_writer writer;

  c2numpy_init(&writer, "testout", 5);
  c2numpy_addcolumn(&writer, "one", C2NUMPY_INTC);
  c2numpy_addcolumn(&writer, "two", C2NUMPY_FLOAT64);
  c2numpy_addcolumn(&writer, "three", C2NUMPY_STRING + 5);

  // completely optional: writing will open a file if not explicitly called
  c2numpy_open(&writer);

  printf("row %d\n", writer.currentRowInFile);
  c2numpy_row(&writer, 1, 1.1, "ONE");

  printf("row %d\n", writer.currentRowInFile);
  c2numpy_row(&writer, 2, 2.2, "TWO");

  printf("row %d by separate calls\n", writer.currentRowInFile);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_intc(&writer, 3);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_float64(&writer, 3.3);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_string(&writer, "THREE");

  printf("row %d by separate calls\n", writer.currentRowInFile);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_intc(&writer, 4);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_float64(&writer, 4.4);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_string(&writer, "FOUR");

  printf("row %d\n", writer.currentRowInFile);
  c2numpy_row(&writer, 5, 5.5, "FIVE");

  printf("row %d\n", writer.currentRowInFile);
  c2numpy_row(&writer, 6, 6.6, "SIX");

  printf("row %d\n", writer.currentRowInFile);
  c2numpy_row(&writer, 7, 7.7, "SEVEN");

  printf("row %d by separate calls\n", writer.currentRowInFile);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_intc(&writer, 8);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_float64(&writer, 8.8);
  printf("  col %d\n", writer.currentColumn);
  c2numpy_string(&writer, "EIGHT");

  printf("close\n");
  c2numpy_close(&writer);

  printf("end\n");
}
