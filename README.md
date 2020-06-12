# ExternalSort
Simple implementation of external sorting algorithm.

The assumption is made that the elements in the file are stored with the same endianness as the host has.  
exsort allow to choose sorting direction as well as element size. See usage details below.  

```shell
Usage:
exsort { -h | <-i inputFilePath> [-o outputFilePath] [-s ramSize] [-e elSize] [-r]}
  -h     print this usage and exit
  -i     input file path
  -o     output file path, default is: /tmp/exsort.out
  -s     available ram size to use for sorting in bytes, defult is: 1024
  -e     element size in bytes, range [1, 8], should be pow of 2, defult is: 4
  -r     reverse sort, default is increasing order.
```

There are 2 helper utility called randomfile and checkfile.  

randomfile may be used to generate rando mfile.  
This utility is not developed with optimization in mind, but  
it's still much more faster then /dev/random. See usage details below.  
```shell
Usage: 
randomfile { -h | <-o outputFilePath> <-s fileSize>}
  -h     print this usage and exit
  -o     file path
  -s     file size in bytes
```

checkfile may be used to check if the given file is sorted or not,  
also it may be used to calculate the power of the given file.  
Please note that the power of the file before and after sorting should be equal.See usage below.  
```shell
Usage: 
checkfile {-h | <-i inputFilePath> <-e elementSize> [-r]}
  -h     print this usage and exit
  -i     file path
  -e     element size in bytes, range [1, 8], should be pow of 2
  -r     reverse check, default is increasing order
```

Example usage  

Create big random file, ~11GB  
```shell
out/bin/randomfile -o /tmp/random -s 10737418240
```

Sort given file with element size 8, buffer size 10KB  
You may increase buffer size to increase sorting speed

```shel
time out/bin/exsort  -i /tmp/random -o /tmp/random_8_sorted -s 10485760 -e 8
```

Check input and output files.
```
out/bin/checkfile -i /tmp/random -e 8
Magic: -2283801748155845678
Array is not sorted.
```
```shell
out/bin/checkfile -i /tmp/random_8_sorted -e 8
Magic: -2283801748155845678
Array is sorted.
```

Sort given file with element size 4, buffer size 10KB, in reverse order
```shell
time out/bin/exsort  -i /tmp/random -o /tmp/random_4_sorted -s 10485760 -e 4 -r
```

Used C++17.  
Tested under gcc-v7.5.0, ubuntu 18.04.1.
