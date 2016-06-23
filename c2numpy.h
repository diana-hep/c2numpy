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

    C2NUMPY_STRING       = 100,  // strings are C2NUMPY_STRING + their fixed size (up to 155)
    C2NUMPY_END          = 255   // ensure that c2numpy_type is at least a byte
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
      // FIXME: implement fixed-width strings
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

    return 0;
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

    return 0;
}

int c2numpy_open(c2numpy_writer *writer) {
    char *fileName = (char*)malloc(strlen(writer->outputFilePrefix) + 15);
    sprintf(fileName, "%s%d.npy", writer->outputFilePrefix, writer->currentFileNumber);
    writer->file = fopen(fileName, "wb");

    // FIXME: better initial guess about header size before going in 128 byte increments
    char *header = NULL;
    for (uint16_t headerSize = 128;  headerSize <= 4294967295;  headerSize += 128) {
        if (header != NULL) free(header);
        header = (char*)malloc(headerSize + 1);

        char version1 = headerSize <= 65535;
        uint16_t descrSize;
        if (version1)
            descrSize = headerSize - 10;
        else
            descrSize = headerSize - 12;

        header[0] = 147;                            // magic
        header[1] = 'N';
        header[2] = 'U';
        header[3] = 'M';
        header[4] = 'P';
        header[5] = 'Y';
        if (version1) {
            header[6] = 1;                          // format version 1.0
            header[7] = 0;
            *(uint16_t*)(header + 8) = descrSize;   // version 1.0 has a 16-byte descrSize
        }
        else {
            header[6] = 2;                          // format version 2.0
            header[7] = 0;
            *(uint32_t*)(header + 8) = descrSize;   // version 2.0 has a 32-byte descrSize
        }

        int offset = headerSize - descrSize;
        offset += snprintf((header + offset), headerSize - offset + 1, "{'descr': [");
        if (offset >= headerSize) continue;

        for (int column = 0;  column < writer->numColumns;  ++column) {
            offset += snprintf((header + offset), headerSize - offset + 1, "('%s', '%s')",
                              writer->columnNames[column],
                              c2numpy_descr(writer->columnTypes[column]));
            if (offset >= headerSize) continue;

            if (column < writer->numColumns - 1)
                offset += snprintf((header + offset), headerSize - offset + 1, ", ");
            if (offset >= headerSize) continue;
        }

        offset += snprintf((header + offset), headerSize - offset + 1, "], 'fortran_order': False, 'shape': (%d,), }", writer->numRowsPerFile);
        if (offset >= headerSize) continue;

        while (offset < headerSize) {
            if (offset < headerSize - 1)
                header[offset] = ' ';
            else
                header[offset] = '\n';
            offset += 1;
        }
        header[headerSize] = 0;

        fwrite(header, 1, headerSize, writer->file);

        return 0;
    }

    return -1;
}

int c2numpy_fill(c2numpy_writer *writer, ...) {
  va_list argp;
  va_start(argp, writer);

  for (int column = 0;  column < writer->numColumns;  ++column) {
    switch (writer->columnTypes[column]) {
      case C2NUMPY_BOOL:   // "bool" is just a byte
          *(int8_t*)writer->buffer = (int8_t)va_arg(argp, int);
          fwrite((void*)writer->buffer, sizeof(int8_t), 1, writer->file);
          break;
      case C2NUMPY_INT:    // Numpy's default int is 64-bit
          *(int64_t*)writer->buffer = va_arg(argp, int64_t);
          fwrite((void*)writer->buffer, sizeof(int64_t), 1, writer->file);
          break;
      case C2NUMPY_INTC:   // the built-in C int
          *(int*)writer->buffer = va_arg(argp, int);
          fwrite((void*)writer->buffer, sizeof(int), 1, writer->file);
          break;
      case C2NUMPY_INTP:   // intp is Numpy's way of saying size_t
          *(size_t*)writer->buffer = va_arg(argp, size_t);
          fwrite((void*)writer->buffer, sizeof(size_t), 1, writer->file);
          break;
      case C2NUMPY_INT8:
          *(int8_t*)writer->buffer = (int8_t)va_arg(argp, int);
          fwrite((void*)writer->buffer, sizeof(int8_t), 1, writer->file);
          break;
      case C2NUMPY_INT16:
          *(int16_t*)writer->buffer = (int16_t)va_arg(argp, int);
          fwrite((void*)writer->buffer, sizeof(int16_t), 1, writer->file);
          break;
      case C2NUMPY_INT32:
          *(int32_t*)writer->buffer = va_arg(argp, int32_t);
          fwrite((void*)writer->buffer, sizeof(int32_t), 1, writer->file);
          break;
      case C2NUMPY_INT64:
          *(int64_t*)writer->buffer = va_arg(argp, int64_t);
          fwrite((void*)writer->buffer, sizeof(int64_t), 1, writer->file);
          break;
      case C2NUMPY_UINT8:
          *(uint8_t*)writer->buffer = (uint8_t)va_arg(argp, int);
          fwrite((void*)writer->buffer, sizeof(uint8_t), 1, writer->file);
          break;
      case C2NUMPY_UINT16:
          *(uint16_t*)writer->buffer = (uint16_t)va_arg(argp, int);
          fwrite((void*)writer->buffer, sizeof(uint16_t), 1, writer->file);
          break;
      case C2NUMPY_UINT32:
          *(uint32_t*)writer->buffer = va_arg(argp, uint32_t);
          fwrite((void*)writer->buffer, sizeof(uint32_t), 1, writer->file);
          break;
      case C2NUMPY_UINT64:
          *(uint64_t*)writer->buffer = va_arg(argp, uint64_t);
          fwrite((void*)writer->buffer, sizeof(uint64_t), 1, writer->file);
          break;
      case C2NUMPY_FLOAT:   // Numpy's "float" is a double
          *(double*)writer->buffer = va_arg(argp, double);
          fwrite((void*)writer->buffer, sizeof(double), 1, writer->file);
          break;
      // case C2NUMPY_FLOAT16:   // how to do float16 in C?
      //     break;
      case C2NUMPY_FLOAT32:
          *(float*)writer->buffer = (float)va_arg(argp, double);
          fwrite((void*)writer->buffer, sizeof(float), 1, writer->file);
          break;
      case C2NUMPY_FLOAT64:
          *(double*)writer->buffer = va_arg(argp, double);
          fwrite((void*)writer->buffer, sizeof(double), 1, writer->file);
          break;
      // case C2NUMPY_COMPLEX:    // how to do complex in C?
      //     break;
      // case C2NUMPY_COMPLEX64:
      //     break;
      // case C2NUMPY_COMPLEX128:
      //     break;
      default:
          return -1;
    }
  }

  va_end(argp);
  return 0;
}

int c2numpy_close(c2numpy_writer *writer) {
    fclose(writer->file);
    // FIXME: free all the memory that was allocated in c2numpy_writer.
}

#endif /* C2NUMPY */
