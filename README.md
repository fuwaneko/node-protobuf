# node-protobuf

[![Build Status](https://travis-ci.org/fuwaneko/node-protobuf.png?branch=master)](https://travis-ci.org/fuwaneko/node-protobuf)

It's a very simple wrapper around protocol buffers library by Google.
It uses dynamic message building and parsing based on protocol schema you can get by compiling protocol description.
I know that wrappers do exist, but I didn't like them.

## Issues

* There is no check for required fields present when serializing

## Roadmap

+ Add support for serializing and parsing int64 from/to JS String
+ Add asynchronous serialize and parse
+ Refactor code to make it more readable
+ Add check for required fields present when serializing

## Changelog

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

## Requirements

* Protocol Buffers >= 2.1.0
* Node.js >= 0.10.0

## Installation

### Linux

Make sure you have node, node-gyp, compiler and libprotobuf binary and development files. Then install: ``` npm install node-protobuf ```
Simple, huh?

### Windows

First you have to obtain Microsoft Visual C++ 2010. Express is fine, but if you install SP1 then you'll need to reinstall x64 compilers from [here](http://www.microsoft.com/en-us/download/details.aspx?id=4422). Node.js distribution for Windows already includes node-gyp tool, so you don't need to worry about it.

Next, compile libprotobuf. Get it from Google, open vsprojects/protobuf.sln and compile it according to your OS version. For Windows 64 you *must* compile 64-bit library as Node.js is 64-bit on your system. By default there is only 32-bit target, so you have to add 64-bit manually. Open libprotobuf project properties and set CRT to Multi-Threaded (*not* DLL). Compile libprotobuf Release.

Next, run vsprojects/extract-includes.bat, it will copy required headers in vsprojects/include.

Create LIBPROTOBUF environment variable pointing to some folder where you'll put libprotobuf files. You can use SET LIBPROTOBUF=path from command line prompt, or use tool like RapidEE. Put libprotobuf.lib to $(LIBPROTOBUF)/lib and include files to $(LIBPROTOBUF)/include. You're done.

Now just ``` npm install node-protobuf ``` and it should be fine.

### Mac OS X

Looks like it's as simple as ``` brew install protobuf ``` before installing npm.
And then ``` LIBPROTOBUF=/usr/local/Cellar/protobuf/2.5.0 npm install node-protobuf ```.

## Usage

You are *not* required to generate any JS code from your protocol buffer description. Everything happens dynamically.

```
var fs = require("fs")
var p = require("node-protobuf").Protobuf
// WARNING: next call will throw if desc file is invalid
var pb = new p(fs.readFileSync("protocol.desc")) // obviously you can use async methods, it's for simplicity reasons
var obj = {
	"name": "value"
}
try {
	var buf = pb.Serialize(obj, "MySchema") // you get Buffer here, send it via socket.write, etc.
} catch (e) {
	// will throw if MySchema does not exist
}
try {
	var newObj = pb.Parse(buf, "MySchema") // you get plain object here, it should be exactly the same as obj
} catch (e) {
	// will throw on invalid buffer or if MySchema does not exist
}
```

### Construct

**Protobuf(buffer[, preserve_int64])**

Parses binary buffer holding schema description. You can get schema by calling ```protoc protocol.proto -o protocol.desc```.

Optional preserve_int64 argument allows to pass int64/uint64 numbers to/from JavaScript in a form of array consisting of high and low bits of such numbers: [hi, lo]. Note, that this is highly experimental and you must work with such numbers only if you really need such whole presicion. In any other case I recommend to use int32/uint32 or double if you need more precision. I added this functionality so JS programs working with existing protocol buffers enabled software could get same amount of presicion when transferring int64/uint64.

Please note, that both high and low bits must be unsigned if you want to Parse. Serialize will return unsigned only too. You have to work with bit parts to create full number using some other library, e.g. [Int64](https://github.com/broofa/node-int64).

Also, if you don't care about int64/uint64 presicion in JS, you can forget about preserve_int64 and all 64-bit integers that come to Serialize will become just Numbers, but if they are bigger than 2^53 you'll lose some data.

### Serialize

**Protobuf.Serialize(object, schema)**

Serializes plain object with accordance to protocol schema (i.e. message described in you protocol description file). Returns Node.js Buffer. Throws an exception if schema does not exist.

### Parse

**Protobuf.Parse(buffer, schema)**

Parses Buffer (or UInt8Array for example, just anything that is binary data array) according to schema and returns plain object. If first argument isn't a Buffer, throws an exception. If Buffer is malformed (i.e. not a Protobuf), throws an exception.