# c2numpy

Write Numpy (.npy) files from C or C++ for analysis in [Numpy](http://www.numpy.org/), [Scipy](https://www.scipy.org/), [Scikit-Learn](http://scikit-learn.org/stable/), [Pandas](http://pandas.pydata.org/), etc.

Fills a collection of .npy files with a maximum size, a common prefix and a rotating number (like rotating log files). Each file contains one [structured array](http://docs.scipy.org/doc/numpy/user/basics.rec.html), consisting of named, typed columns (numbers and fixed-size strings) and many rows. In Python, you access rows and columns with string and integer indexing:

    myarray["column1"]   # one column, all rows
    myarray[3:5]         # all columns, slice of rows
                         # etc.

This project does not support _reading_ of Numpy files.

## Installation

Put `c2numpy.h` in your C or C++ project and compile. No libraries are required. Adheres to strict [ISO C99](http://www.iso-9899.info/wiki/The_Standard).

For an example and testing, `test.c` is provided. Compile and run it with

    gcc -std=c99 test.c -o testme && ./testme

and view the results with

    python -c "import numpy; print numpy.load(open('testout0.npy')); print numpy.load(open('testout1.npy'));"

## C API

### Enumeration constants for Numpy types: `c2numpy_type`

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

Not currently supported:

   * `C2NUMPY_FLOAT16`
   * `C2NUMPY_COMPLEX`
   * `C2NUMPY_COMPLEX64`
   * `C2NUMPY_COMPLEX128`
   * strings 155 characters or longer

### Writer object: `c2numpy_writer`

A writer contains the following fields. Some of them are internal, and all of them should be treated as read-only. Use the associated functions to manipulate.

    typedef struct {
        char buffer[16];              // used for temporary copies in c2numpy_row

        FILE *file;                   // output file handle
        char *outputFilePrefix;       // output file name, not including the rotating number and .npy
        int64_t sizeSeekPosition;     // (internal) keep track of Numpy shape for possible modification before closing
        int64_t sizeSeekSize;         // (internal)

        int32_t numColumns;           // number of columns in the record array
        char **columnNames;           // column names
        c2numpy_type *columnTypes;    // column types

        int32_t numRowsPerFile;       // maximum number of rows per file
        int32_t currentColumn;        // current column number
        int32_t currentRowInFile;     // current row number in the current file
        int32_t currentFileNumber;    // current file number
    } c2numpy_writer;

### Numpy description string from type: `c2numpy_descr`

    const char *c2numpy_descr(c2numpy_type type);

Rarely needed by typical users; converts a `c2numpy_type` to the corresponding Numpy "descr" string. Returns `NULL` if the `type` is invalid.

### Initialize a writer object: `c2numpy_init`

    int c2numpy_init(c2numpy_writer *writer, const char *outputFilePrefix, int32_t numRowsPerFile);

This is the first function you should call on a new writer. After this, call `c2numpy_addcolumn` to add column descriptions.

   * `writer`: a new writer object.
   * `outputFilePrefix`: name of the output files, not including the rotating file number or the `.npy` suffix. This can include directories.
   * `numRowsPerFile`: number of rows to write before starting a new file.
   * **returns:** 0 if successful, -1 otherwise

### Add a column to the writer: `c2numpy_addcolumn`

    int c2numpy_addcolumn(c2numpy_writer *writer, const char *name, c2numpy_type type);

This is the second function you should call on a new writer. Call it once for each column you wish to add.

   * `writer`: the writer object, already initialized.
   * `name`: the name of the column to add.
   * `type`: the type of the column to add (see enumeration constants above).
   * **returns:** 0 if successful, -1 otherwise

### Optional open file: `c2numpy_open`

    int c2numpy_open(c2numpy_writer *writer);

Open a file and write its header to disk. If not called, adding data will open the file, but you might want to call it immediately to learn about I/O errors early.

### Write a row of data in one call: `c2numpy_row`

    int c2numpy_row(c2numpy_writer *writer, ...)

Varadic function writes a whole row at a time. If `writer->currentColumn` is out of sync, this will raise an error, so it is safe against column-misalignment. The varadic arguments are not type-safe, however: this has the same features and issues as `printf` in the standard library.

### Write an item of data: `c2numpy_*`

The following suite of functions support writing one item (row and column) at a time. 






## C++ API

Not written yet (will wrap around the C functions with macros).

## To do

   * Add convenience function to calculate number of rows for a target file size.
   * Compressed (.npz) files.
   * System independence (currently assumes little endian with 32-bit `int` and 64-bit `size_t`).
   * Faster guessing of header size and column types.
   * Float16 and complex numbers.
   * Optional C++ API.
