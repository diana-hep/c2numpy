# c2numpy

Write Numpy (.npy) files from C or C++ for analysis in [Numpy](http://www.numpy.org/), [Scipy](https://www.scipy.org/), [Scikit-Learn](http://scikit-learn.org/stable/), [Pandas](http://pandas.pydata.org/), etc.

Fills a collection of .npy files with a maximum size, a common prefix and a rotating number (like rotating log files). Each file contains one [structured array](http://docs.scipy.org/doc/numpy/user/basics.rec.html), consisting of named, typed columns (numbers and fixed-size strings) and many rows. In Python, you access rows and columns with string and integer indexing:

```python
myarray["column1"]   # one column, all rows
myarray[3:5]         # all columns, slice of rows
                     # etc.
```

This project does not support _reading_ of Numpy files in C.

## Installation

Put `c2numpy.h` in your C or C++ project and compile. No libraries are required. Adheres to strict [ISO C99](http://www.iso-9899.info/wiki/The_Standard).

For an example and testing, `test.c` is provided. Compile and run it with

```bash
gcc -std=c99 test.c -o testme && ./testme
```

and view the results with

```bash
python -c "import numpy; print numpy.load(open('testout0.npy'));"
python -c "import numpy; print numpy.load(open('testout1.npy'));"
```

## C example

```c
// declare writer
c2numpy_writer writer;

// initialize it and add columns
c2numpy_init(&writer, "testout", 5);
c2numpy_addcolumn(&writer, "one", C2NUMPY_INTC);
c2numpy_addcolumn(&writer, "two", C2NUMPY_FLOAT64);
c2numpy_addcolumn(&writer, "three", C2NUMPY_STRING + 5);

// first row
c2numpy_row(&writer, 1, 1.1, "ONE");

// second row
c2numpy_row(&writer, 2, 2.2, "TWO");

// third row
c2numpy_intc(&writer, 3);
c2numpy_float64(&writer, 3.3);
c2numpy_string(&writer, "THREE");

// close file
c2numpy_close(&writer);
```

## C API

### Enumeration constants for Numpy types: `c2numpy_type`

See [number type definitions](http://docs.scipy.org/doc/numpy/user/basics.types.html) in the Numpy documentation.

```c
C2NUMPY_BOOL        // Boolean (True or False) stored as a byte
C2NUMPY_INT         // Default integer type (same as C long; normally either int64 or int32)
C2NUMPY_INTC        // Identical to C int (normally int32 or int64)
C2NUMPY_INTP        // Integer used for indexing (same as C ssize_t; normally either int32 or int64)
C2NUMPY_INT8        // Byte (-128 to 127)
C2NUMPY_INT16       // Integer (-32768 to 32767)
C2NUMPY_INT32       // Integer (-2147483648 to 2147483647)
C2NUMPY_INT64       // Integer (-9223372036854775808 to 9223372036854775807)
C2NUMPY_UINT8       // Unsigned integer (0 to 255)
C2NUMPY_UINT16      // Unsigned integer (0 to 65535)
C2NUMPY_UINT32      // Unsigned integer (0 to 4294967295)
C2NUMPY_UINT64      // Unsigned integer (0 to 18446744073709551615)
C2NUMPY_FLOAT       // Shorthand for float64.
C2NUMPY_FLOAT16     // Half precision float: sign bit, 5 bits exponent, 10 bits mantissa
C2NUMPY_FLOAT32     // Single precision float: sign bit, 8 bits exponent, 23 bits mantissa
C2NUMPY_FLOAT64     // Double precision float: sign bit, 11 bits exponent, 52 bits mantissa
C2NUMPY_COMPLEX     // Shorthand for complex128.
C2NUMPY_COMPLEX64   // Complex number, represented by two 32-bit floats (real and imaginary components)
C2NUMPY_COMPLEX128  // Complex number, represented by two 64-bit floats (real and imaginary components)
C2NUMPY_STRING      = 100  // strings are C2NUMPY_STRING + their fixed size (up to 155)
```

Not currently supported:

   * `C2NUMPY_FLOAT16`
   * `C2NUMPY_COMPLEX`
   * `C2NUMPY_COMPLEX64`
   * `C2NUMPY_COMPLEX128`
   * strings 155 characters or longer

### Writer object: `c2numpy_writer`

A writer contains the following fields. Some of them are internal, and all of them should be treated as read-only. Use the associated functions to manipulate.

```c
typedef struct {
    char buffer[16];              // (internal) used for temporary copies in c2numpy_row

    FILE *file;                   // output file handle
    char *outputFilePrefix;       // output file name, not including the rotating number and .npy
    int64_t sizeSeekPosition;     // (internal) keep track of number of rows to modify before closing
    int64_t sizeSeekSize;         // (internal)

    int32_t numColumns;           // number of columns in the record array
    char **columnNames;           // column names
    c2numpy_type *columnTypes;    // column types

    int32_t numRowsPerFile;       // maximum number of rows per file
    int32_t currentColumn;        // current column number
    int32_t currentRowInFile;     // current row number in the current file
    int32_t currentFileNumber;    // current file number
} c2numpy_writer;
```

### Numpy description string from type: `c2numpy_descr`

```c
const char *c2numpy_descr(c2numpy_type type);
```

Rarely needed by typical users; converts a `c2numpy_type` to the corresponding Numpy "descr" string. Returns `NULL` if the `type` is invalid.

### Initialize a writer object: `c2numpy_init`

```c
int c2numpy_init(c2numpy_writer *writer, const char *outputFilePrefix, int32_t numRowsPerFile);
```

This is the first function you should call on a new writer. After this, call `c2numpy_addcolumn` to add column descriptions.

   * `writer`: a new writer object.
   * `outputFilePrefix`: name of the output files, not including the rotating file number or the `.npy` suffix. This can include directories.
   * `numRowsPerFile`: number of rows to write before starting a new file.
   * **returns:** 0 if successful, -1 otherwise

**Copies** the `outputFilePrefix`, so you are responsible for deleting the original if necessary.

### Add a column to the writer: `c2numpy_addcolumn`

```c
int c2numpy_addcolumn(c2numpy_writer *writer, const char *name, c2numpy_type type);
```

This is the second function you should call on a new writer. Call it once for each column you wish to add.

   * `writer`: the writer object, already initialized.
   * `name`: the name of the column to add.
   * `type`: the type of the column to add (see enumeration constants above).
   * **returns:** 0 if successful, -1 otherwise

**Copies** the `name`, so you are responsible for deleting the original if necessary.

### Optional open file: `c2numpy_open`

```c
int c2numpy_open(c2numpy_writer *writer);
```

Open a file and write its header to disk. If not called, adding data will open the file, but you might want to call it immediately to learn about I/O errors early.

**Returns:** 0 if successful and -1 otherwise.

### Write a row of data in one call: `c2numpy_row`

```c
int c2numpy_row(c2numpy_writer *writer, ...)
```

Varadic function writes a whole row at a time. If `writer->currentColumn` is out of sync, this will raise an error, so it is safe against column-misalignment. The varadic arguments are not type-safe, however: this has the same features and issues as `printf` in the standard library.

**Returns:** 0 if successful, -1 otherwise.

### Write an item of data: `c2numpy_*`

The following suite of functions support writing one item (row and column) at a time. They check the requested data type against the expected data type for the current column, but cannot prevent column-misalignment if all data types are the same.

```c
int c2numpy_bool(c2numpy_writer *writer, int8_t data);        // "bool" is just a byte
int c2numpy_int(c2numpy_writer *writer, int64_t data);        // Numpy's default int is 64-bit
int c2numpy_intc(c2numpy_writer *writer, int data);           // the built-in C int
int c2numpy_intp(c2numpy_writer *writer, size_t data);        // intp is Numpy's way of saying size_t
int c2numpy_int8(c2numpy_writer *writer, int8_t data);
int c2numpy_int16(c2numpy_writer *writer, int16_t data);
int c2numpy_int32(c2numpy_writer *writer, int32_t data);
int c2numpy_int64(c2numpy_writer *writer, int64_t data);
int c2numpy_uint8(c2numpy_writer *writer, uint8_t data);
int c2numpy_uint16(c2numpy_writer *writer, uint16_t data);
int c2numpy_uint32(c2numpy_writer *writer, uint32_t data);
int c2numpy_uint64(c2numpy_writer *writer, uint64_t data);
int c2numpy_float(c2numpy_writer *writer, double data);       // Numpy's "float" is a double
// int c2numpy_float16(c2numpy_writer *writer, ??? data);     // how to do float16 in C?
int c2numpy_float32(c2numpy_writer *writer, float data);
int c2numpy_float64(c2numpy_writer *writer, double data);
// int c2numpy_complex(c2numpy_writer *writer, ??? data);     // how to do complex in C?
// int c2numpy_complex64(c2numpy_writer *writer, ??? data);
// int c2numpy_complex128(c2numpy_writer *writer, ??? data);
int c2numpy_string(c2numpy_writer *writer, const char *data);
```

**Returns:** 0 if successful and -1 otherwise.

`c2numpy_string` **only writes** the string `data`, so you are responsible for deleting the original if necessary.

### Required close file: `c2numpy_close`

```c
int c2numpy_close(c2numpy_writer *writer);
```

If you do not explicitly close the writer, your last file may be corrupted. Be sure to do this after your loop over data.

**Returns:** 0 if successful and -1 otherwise.

## C++ example and C++ API

Not written yet (will wrap the C functions with C++ class structure using [__cplusplus](http://stackoverflow.com/a/6779715/1623645)).

## To do

   * Add convenience function to calculate number of rows for a target file size.
   * Compressed (.npz) files.
   * System independence (currently assumes little endian with 32-bit `int` and 64-bit `size_t`).
   * Faster guessing of header size and column types.
   * Float16 and complex numbers.
   * Distinct return values for different errors and documentation of those errors.
   * Optional C++ API.
