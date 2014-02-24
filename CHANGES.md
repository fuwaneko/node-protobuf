## Changelog

### 1.0.12

+ Fixed incorrect compiler behaviour on Mac OS X on some systems

### 1.0.11

+ Cleaned up gypsy file for Mac OS X

### 1.0.10

+ Temporarily fixed memory leaks in long-running processes

### 1.0.9

+ Fixed incorrect handling of repeated strings

### 1.0.8

+ Fixed build on Mac OS X 10.9
+ Updated README with better example

### 1.0.7

+ Fixed segfault on incorrect schema name.

### 1.0.6

+ Added check for correct argument type and handling for malformed protocol buffers.

### 1.0.5

+ Fixed incorrect parsing of byte datatype. Now it's parsed as Node.js Buffer and Buffer can be serialized into byte field. Big thanks to [xanm](https://github.com/AlexMarlo) for contributing.

### 1.0.4

+ Fixed incorrect parsing of optional nested messages

### 1.0.3

+ Fixed incorrect parsing of optional values (now they are omitted from result object if not present)

### 1.0.2

+ Added experimental support for 64-bit precision integers (look below for constructor arguments)