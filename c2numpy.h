#ifndef C2NUMPY
#define C2NUMPY

#include <inttypes.h>
#include <stdarg.h>
#include <string.h>

// http://docs.scipy.org/doc/numpy/user/basics.types.html
typedef enum {
    C2NUMPY_BOOL,        // Boolean (True or False) stored as a byte
    C2NUMPY_INT,         // Default integer type (same as C long; normally either int64 or int32)
    C2NUMPY_INTC,        // Identical to C int (normally int32 or int64)
    C2NUMPY_INTP,        // Integer used for indexing (same as C ssize_t; normally either int32 or int64)
    C2NUMPY_INT8,        // Byte (-128 to 127)
    C2NUMPY_INT16,       // Integer (-32768 to 32767)
    C2NUMPY_INT32,       // Integer (-2147483648 to 2147483647)
    C2NUMPY_INT64,       // Integer (-9223372036854775808 to 9223372036854775807)
    C2NUMPY_UINT8,       // Unsigned integer (0 to 255)
    C2NUMPY_UINT16,      // Unsigned integer (0 to 65535)
    C2NUMPY_UINT32,      // Unsigned integer (0 to 4294967295)
    C2NUMPY_UINT64,      // Unsigned integer (0 to 18446744073709551615)
    C2NUMPY_FLOAT,       // Shorthand for float64.
    C2NUMPY_FLOAT16,     // Half precision float: sign bit, 5 bits exponent, 10 bits mantissa
    C2NUMPY_FLOAT32,     // Single precision float: sign bit, 8 bits exponent, 23 bits mantissa
    C2NUMPY_FLOAT64,     // Double precision float: sign bit, 11 bits exponent, 52 bits mantissa
    C2NUMPY_COMPLEX,     // Shorthand for complex128.
    C2NUMPY_COMPLEX64,   // Complex number, represented by two 32-bit floats (real and imaginary components)
    C2NUMPY_COMPLEX128,  // Complex number, represented by two 64-bit floats (real and imaginary components)
} c2numpy_type;

typedef struct {
    char buffer[16];
    FILE *file;
    int32_t numColumns;
    char **columnNames;
    c2numpy_type *columnTypes;
    char *outputFilePrefix;
    int32_t numRowsPerFile;
    int32_t currentFileNumber;
} c2numpy_writer;

int c2numpy_init(c2numpy_writer *writer, const char *outputFilePrefix, int32_t numRowsPerFile) {
    writer->file = NULL;
    writer->numColumns = 0;
    writer->columnNames = NULL;
    writer->columnTypes = NULL;
    writer->outputFilePrefix = (char*)malloc(strlen(outputFilePrefix) + 1);
    strcpy(writer->outputFilePrefix, outputFilePrefix);
    writer->numRowsPerFile = numRowsPerFile;
    writer->currentFileNumber = 0;
}

int c2numpy_addcolumn(c2numpy_writer *writer, const char *name, c2numpy_type type) {
    writer->numColumns += 1;

    char *newColumnName = (char*)malloc(strlen(name) + 1);
    strcpy(newColumnName, name);

    char **oldColumnNames = writer->columnNames;
    writer->columnNames = (char**)malloc(writer->numColumns * sizeof(char*));
    for (int column = 0;  column < writer->numColumns - 1;  ++column)
        writer->columnNames[column] = oldColumnNames[column];
    writer->columnNames[writer->numColumns - 1] = newColumnName;
    if (oldColumnNames != NULL)
        free(oldColumnNames);

    c2numpy_type *oldColumnTypes = writer->columnTypes;
    writer->columnTypes = (c2numpy_type*)malloc(writer->numColumns * sizeof(c2numpy_type));
    for (int column = 0;  column < writer->numColumns - 1;  ++column)
        writer->columnTypes[column] = oldColumnTypes[column];
    writer->columnTypes[writer->numColumns - 1] = type;
    if (oldColumnTypes != NULL)
        free(oldColumnTypes);
}


int c2numpy_open(c2numpy_writer *writer) {
    char *fileName = (char*)malloc(strlen(writer->outputFilePrefix) + 15);
    sprintf(fileName, "%s%d.npy", writer->outputFilePrefix, writer->currentFileNumber);
    writer->file = fopen(fileName, "wb");
}

int c2numpy_fill(c2numpy_writer *writer, ...) {
  va_list argp;
  va_start(argp, writer);

  for (int column = 0;  column < writer->numColumns;  ++column) {
    switch (writer->columnTypes[column]) {
      // case C2NUMPY_BOOL:
      //     break;
      // case C2NUMPY_INT:
      //     break;
      case C2NUMPY_INTC:
          *(int*)writer->buffer = va_arg(argp, int);
          fwrite((void*)writer->buffer, sizeof(int), 1, writer->file);
          break;
      // case C2NUMPY_INTP:
      //     break;
      // case C2NUMPY_INT8:
      //     break;
      // case C2NUMPY_INT16:
      //     break;
      // case C2NUMPY_INT32:
      //     break;
      // case C2NUMPY_INT64:
      //     break;
      // case C2NUMPY_UINT8:
      //     break;
      // case C2NUMPY_UINT16:
      //     break;
      // case C2NUMPY_UINT32:
      //     break;
      // case C2NUMPY_UINT64:
      //     break;
      // case C2NUMPY_FLOAT:
      //     break;
      // case C2NUMPY_FLOAT16:
      //     break;
      // case C2NUMPY_FLOAT32:
      //     break;
      case C2NUMPY_FLOAT64:
          *(double*)writer->buffer = va_arg(argp, double);
          fwrite((void*)writer->buffer, sizeof(double), 1, writer->file);
          break;
      // case C2NUMPY_COMPLEX:
      //     break;
      // case C2NUMPY_COMPLEX64:
      //     break;
      // case C2NUMPY_COMPLEX128:
      //     break;
    }
  }

  va_end(argp);
}

int c2numpy_close(c2numpy_writer *writer) {
    fclose(writer->file);
}

#endif /* C2NUMPY */
