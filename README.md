# c2numpy

Write Numpy (.npy) files from C or C++ for analysis in [Numpy](http://www.numpy.org/), [Scipy](https://www.scipy.org/), [Scikit-Learn](http://scikit-learn.org/stable/), [Pandas](http://pandas.pydata.org/), etc.

## Installation

Put `c2numpy.h` in your C or C++ project and compile. No libraries are required. Adheres to strict [ISO C99](http://www.iso-9899.info/wiki/The_Standard).

For an example and testing, `test.c` is provided. Compile and run it with

    gcc -std=c99 test.c -o testme && ./testme

## API


