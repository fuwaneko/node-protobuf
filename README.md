# node-protobuf

It's a very simple wrapper around protocol buffers library by Google.
It uses dynamic message building and parsing based on protocol schema you can get by compiling protocol description.
I know that wrappers do exist, but I didn't like them.

## Issues

* There is no check for required fields present while serializing

## Changelog

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
var buf = p.Serialize(obj, "MySchema") // you get Buffer here, send it via socket.write, etc.
var newObj = p.Parse(buf, "MySchema") // you get plain object here, it should be exactly the same as obj
```

### Construct

**Protobuf(buffer[, preserve_int64])**

Parses binary buffer holding schema description. You can get schema by calling ```protoc protocol.proto -o protocol.desc```.

Optional preserve_int64 argument allows to pass int64/uint64 numbers to/from JavaScript in a form of array consisting of high and low bits of such numbers: [hi, lo]. Note, that this is highly experimental and you must work with such numbers only if you really need such whole presicion. In any other case I recommend to use int32/uint32 or double if you need more precision. I added this functionality so JS programs working with existing protocol buffers enabled software could get same amount of presicion when transferring int64/uint64.

Please note, that both high and low bits must be unsigned if you want to Parse. Serialize will return unsigned only too. You have to work with bit parts to create full number using some other library, e.g. [Int64](https://github.com/broofa/node-int64).

Also, if you don't care about int64/uint64 presicion in JS, you can forget about preserve_int64 and all 64-bit integers that come to Serialize will become just Numbers, but if they are bigger than 2^53 you'll lose some data.

### Serialize

**Protobuf.Serialize(object, schema)**

Serializes plain object with accordance to protocol schema (i.e. message described in you protocol description file). Returns Node.js Buffer.

### Parse

**Protobuf.Parse(buffer, schema)**

Parses Buffer (or UInt8Array for example, just anything that is binary data array) according to schema and returns plain object.