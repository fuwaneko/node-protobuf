# node-protobuf

It's a very simple wrapper around protocol buffers library by Google.
It uses dynamic message building and parsing based on protocol schema you can get by compiling protocol description.
I know that wrappers do exist, but I didn't like them.

## Requirements

Protocol Buffers >= 2.1.0
Node.js >= 0.10.0

## Installation

### Linux

Make sure you have node, node-gyp, compiler and libprotobuf binary and development files. Then install: ``` npm install node-protobuf ```
Simple, huh?

### Windows

First you have to obtain Microsoft Visual C++ 2010. Express is fine, but if you install SP1 then you'll need to reinstall x64 compilers from [http://www.microsoft.com/en-us/download/details.aspx?id=4422](here). Node.js distribution for Windows already includes node-gyp tool, so you don't need to worry about it.

Next, compile libprotobuf. Get it from Google, open vsprojects/protobuf.sln and compile it according to your OS version. For Windows 64 you *must* compile 64-bit library as Node.js is 64-bit on your system. By default there is only 32-bit target, so you have to add 64-bit manually. Open libprotobuf project properties and set CRT to Multi-Threaded (*not* DLL). Compile libprotobuf Release.

Next, run vsprojects/extract-includes.bat, it will copy required headers in vsprojects/include.

Create LIBPROTOBUF environment variable pointing to some folder where you'll put libprotobuf files. You can use SET LIBPROTOBUF=path from command line prompt, or use tool like RapidEE. Put libprotobuf.lib to $(LIBPROTOBUF)/lib and include files to $(LIBPROTOBUF)/include. You're done.

Now just ``` npm install node-protobuf ``` and it should be fine.

### Mac OS X

I don't have Mac, so if someone can describe procedure I'll be grateful.

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

```Protobuf(buffer)```

Parses binary buffer holding schema description. You can get schema by calling ```protoc protocol.proto -o protocol.desc```.

### Serialize

```Protobuf.Serialize(object, schema)```

Serializes plain object with accordance to protocol schema (i.e. message described in you protocol description file). Returns Node.js Buffer.

### Parse

```Protobuf.Parse(buffer, schema)```

Parses Buffer (or UInt8Array for example, just anything that is binary data array) according to schema and returns plain object.