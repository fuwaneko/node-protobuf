# node-protobuf

[![Join the chat at https://gitter.im/fuwaneko/node-protobuf](https://badges.gitter.im/fuwaneko/node-protobuf.svg)](https://gitter.im/fuwaneko/node-protobuf?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

[![Build Status](https://travis-ci.org/fuwaneko/node-protobuf.png?branch=master)](https://travis-ci.org/fuwaneko/node-protobuf)

It's a very simple wrapper around protocol buffers library by Google.
It uses dynamic message building and parsing based on protocol schema you can get by compiling protocol description.
I know that wrappers do exist, but I didn't like them.

## Alternatives

[ProtoBuf.js](https://github.com/dcodeIO/ProtoBuf.js)

## Current status

Works with Node.js 0.10.x, 0.12.x and 4.x (thanks to [bgdavidx](https://github.com/bgdavidx)).

## Custom buffers

Any string/byte field you pass to Serialize can have optional toProtobuf method which must return Buffer.
Thanks to [cleverca22](https://github.com/cleverca22) for this nice suggestion and pull.
TODO: example and test

## Roadmap

+ Add better support for serializing and parsing int64 from/to JS String

## Requirements

* Protocol Buffers >= 2.1.0
* Node.js >= 0.12.0

## Installation

### Linux

Make sure you have node, node-gyp, pkg-config, a compiler and libprotobuf binary and development files. Then install: ``` npm install node-protobuf ```
Simple, huh?

### Windows

First you have to obtain Microsoft Visual C++ 2010. Express is fine, but if you install SP1 then you'll need to reinstall x64 compilers from [here](http://www.microsoft.com/en-us/download/details.aspx?id=4422). Node.js distribution for Windows already includes node-gyp tool, so you don't need to worry about it. VC++ 2012/2013 should work as long as you have compiler version compatible with your Node.js installation (i.e. 64 for 64 and 32 for 32). For any problems compiling native modules on Windows consult [node-gyp](https://github.com/TooTallNate/node-gyp) repository.

Next, compile libprotobuf. Get it from Google, open vsprojects/protobuf.sln and compile it according to your OS version. For Windows 64 you *must* compile 64-bit library as Node.js is 64-bit on your system. By default there is only 32-bit target, so you have to add 64-bit manually. Open libprotobuf project properties and set CRT to Multi-Threaded (*not* DLL). Compile libprotobuf Release.

Next, run vsprojects/extract-includes.bat, it will copy required headers in vsprojects/include.

Create LIBPROTOBUF environment variable pointing to some folder where you'll put libprotobuf files. You can use SET LIBPROTOBUF=path from command line prompt, or use tool like RapidEE. Put libprotobuf.lib to $(LIBPROTOBUF)/lib and include files to $(LIBPROTOBUF)/include. You're done.

Now just ``` npm install node-protobuf ``` and it should be fine.

### Mac OS X

Looks like it's as simple as ``` brew install protobuf ``` before installing npm.

## Usage

You are *not* required to generate any JS code from your protocol buffer description. Everything happens dynamically.

```JavaScript
var fs = require("fs")
var p = require("node-protobuf") // note there is no .Protobuf part anymore
// WARNING: next call will throw if desc file is invalid
var pb = new p(fs.readFileSync("protocol.desc")) // obviously you can use async methods, it's for simplicity reasons
var obj = {
	"name": "value"
}
try {
	var buf = pb.serialize(obj, "MySchema") // you get Buffer here, send it via socket.write, etc.
} catch (e) {
	// will throw if MySchema does not exist
}
try {
	var newObj = pb.parse(buf, "MySchema") // you get plain object here, it should be exactly the same as obj
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

**Protobuf.serialize(object, schema, [callback])**

Serializes plain object with accordance to protocol schema (i.e. message described in you protocol description file).

Accepts optional callback parameter which is essentially a function in Node.js callback style, i.e. function(error, result) {}. In case of exceptions (see below) they are passed as first parameter to callback.

- Returns Node.js Buffer
- Throws if schema does not exist
- Throws if required fields are not present

### Parse

**Protobuf.parse(buffer, schema, [callback, limit, warn])**

Parses Buffer (or UInt8Array for example, just anything that is binary data array) according to schema.

Accepts optional callback parameter which is essentially a function in Node.js callback style, i.e. function(error, result) {}. In case of exceptions (see below) they are passed as first parameter to callback.

Optional limit argument allows to set the maximum message limit. If the optional warn argument is not set, the warning threshold is set to limit/2.

- Returns plain object
- Throws if first argument isn't a Buffer
- Throws if Buffer is malformed (i.e. not a Protobuf)

### Info

**Protobuf.info()**

In case you want to get list of all available schemas in a descriptor, just call this method.

Thanks to [@colprog](https://github.com/colprog) for a suggestion.

```JavaScript
var schemas = pb.info()
// returns an array, e.g. [ "Test", "Data" ] for test suite (see test/test.proto and compare)
```

### Enums handling

While serializing you can supply either strings or integers as enum values. E.g. you can pass
```JavaScript
obj = {
  enumValue: "first"
}

// also valid
obj = {
  enumValue: 0
}
```

While parsing you *always* get string. E.g. after parsing buffers serialized from both objects from above you'll get this:
```JavaScript
parsedObj = {
  enumValue: "first"
}
```

## License
```
The MIT License (MIT)

Copyright (c) 2015 Dmitry Gorbunov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
