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

// FIXME: all of the "<" signs should be system-dependent (little endian)
const char *c2numpy_bool = "|b1";
const char *c2numpy_int = "<i8";
const char *c2numpy_intc = "<i4";   // FIXME: should be system-dependent
const char *c2numpy_intp = "<i8";   // FIXME: should be system-dependent
const char *c2numpy_int8 = "|i1";
const char *c2numpy_int16 = "<i2";
const char *c2numpy_int32 = "<i4";
const char *c2numpy_int64 = "<i8";
const char *c2numpy_uint8 = "|u1";
const char *c2numpy_uint16 = "<u2";
const char *c2numpy_uint32 = "<u4";
const char *c2numpy_uint64 = "<u8";
const char *c2numpy_float = "<f8";
const char *c2numpy_float16 = "<f2";
const char *c2numpy_float32 = "<f4";
const char *c2numpy_float64 = "<f8";
const char *c2numpy_complex = "<c16";
const char *c2numpy_complex64 = "<c8";
const char *c2numpy_complex128 = "<c16";

// FIXME: This is where you can put in system dependence.
const char *c2numpy_descr(c2numpy_type type) {
    switch (type) {
      case C2NUMPY_BOOL:
          return c2numpy_bool;
      case C2NUMPY_INT:
          return c2numpy_int;
      case C2NUMPY_INTC:
          return c2numpy_intc;
      case C2NUMPY_INTP:
          return c2numpy_intp;
      case C2NUMPY_INT8:
          return c2numpy_int8;
      case C2NUMPY_INT16:
          return c2numpy_int16;
      case C2NUMPY_INT32:
          return c2numpy_int32;
      case C2NUMPY_INT64:
          return c2numpy_int64;
      case C2NUMPY_UINT8:
          return c2numpy_uint8;
      case C2NUMPY_UINT16:
          return c2numpy_uint16;
      case C2NUMPY_UINT32:
          return c2numpy_uint32;
      case C2NUMPY_UINT64:
          return c2numpy_uint64;
      case C2NUMPY_FLOAT:
          return c2numpy_float;
      case C2NUMPY_FLOAT16:
          return c2numpy_float16;
      case C2NUMPY_FLOAT32:
          return c2numpy_float32;
      case C2NUMPY_FLOAT64:
          return c2numpy_float64;
      case C2NUMPY_COMPLEX:
          return c2numpy_complex;
      case C2NUMPY_COMPLEX64:
          return c2numpy_complex64;
      case C2NUMPY_COMPLEX128:
          return c2numpy_complex128;
    }
}

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

    // FIXME: loop over multiples of 128, attempting to fit the header in each, break when it works.
    // FIXME: if descrSize must be greater than 65535, switch to version 2.0
    uint16_t headerSize = 128;
    char *header = (char*)malloc(headerSize + 1);
    uint16_t descrSize = headerSize - 10;

    header[0] = 147;                    // magic
    header[1] = 'N';
    header[2] = 'U';
    header[3] = 'M';
    header[4] = 'P';
    header[5] = 'Y';
    header[6] = 1;                      // format version 1.0
    header[7] = 0;
    *(uint16_t*)(header + 8) = descrSize;   // version 1.0 has a 16-byte descrSize

    int offset = headerSize - descrSize;
    offset += sprintf((header + offset), "{'descr': [");

    for (int column = 0;  column < writer->numColumns;  ++column) {
        offset += sprintf((header + offset), "('%s', '%s')",
                          writer->columnNames[column],
                          c2numpy_descr(writer->columnTypes[column]));
        if (column < writer->numColumns - 1)
            offset += sprintf((header + offset), ", ");
    }

    offset += sprintf((header + offset), "], 'fortran_order': False, 'shape': (%d,), }", writer->numRowsPerFile);

    while (offset < headerSize) {
        if (offset < headerSize - 1)
            header[offset] = ' ';
        else
            header[offset] = '\n';
        offset += 1;
    }
    header[headerSize] = 0;

    fwrite(header, 1, headerSize, writer->file);
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
