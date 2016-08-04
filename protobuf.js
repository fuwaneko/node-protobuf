/**

This is the main wrapper for protobuf implementation

**/
"use strict";

var assert = require("assert");
var protobuf = require("bindings")("protobuf.node");

function pb_wrapper() {
  if (!(this instanceof pb_wrapper)) {
    // prevent arguments leaking
    var args = [];
    var i = 0;
    for (i = 0; i < arguments.length; i++) {
      args.push(arguments[i]);
    }
    return new pb_wrapper.apply(this, args);
  }

  var descriptor = null;
  var int64 = true;

  if (arguments[0] !== void 0) {
    descriptor = arguments[0];
  }

  if (arguments[1] !== void 0) {
    int64 = arguments[1];
  }

  assert(descriptor, "Descriptor must be provided");

  this.native = new protobuf.native(descriptor, int64);
}

pb_wrapper.prototype.parse = function(buffer, schema, callback, limit, warn, used_typed_array) {
  if (!buffer || !schema)
    throw new Error("Invalid arguments");

  if (!Buffer.isBuffer(buffer))
    throw new Error("First argument must be a Buffer");

  limit = limit | 0;
  warn = warn | 0;
  var native = this.native;

  if (used_typed_array === undefined) {
    used_typed_array = true;
  }

  if (!callback) {
    var result = native.parse(buffer, schema, limit, warn, used_typed_array);
    if (result === null)
      throw new Error("Unexpected error while parsing " + schema);
    else
      return result;
  } else
    process.nextTick(function() {
      try {
        var result = native.parse(buffer, schema, limit, warn, used_typed_array);
        callback(null, result);
      } catch (e) {
        callback(e, null);
      }
    })
};

pb_wrapper.prototype.parseWithUnknown = function(buffer, schema, callback, limit, warn, used_typed_array) {
  if (!buffer || !schema)
    throw new Error("Invalid arguments");

  if (!Buffer.isBuffer(buffer))
    throw new Error("First argument must be a Buffer");

  limit = limit | 0;
  warn = warn | 0;
  var native = this.native;

  if (used_typed_array === undefined) {
    used_typed_array = true;
  }

  if (!callback) {
    var result = native.parseWithUnknown(buffer, schema, limit, warn, used_typed_array);
    if (result === null)
      throw new Error("Unexpected error while parsing " + schema);
    else
      return result;
  } else
    process.nextTick(function() {
      try {
        var result = native.parseWithUnknown(buffer, schema, limit, warn, used_typed_array);
        callback(null, result);
      } catch (e) {
        callback(e, null);
      }
    })
};

pb_wrapper.prototype.serialize = function(object, schema, callback) {
  if (!object || !schema)
    throw new Error("Invalid arguments");

  var native = this.native;

  if (!callback) {
    var result = native.serialize(object, schema);
    if (result === null)
      throw new Error("Missing required fields while serializing " + schema);
    else
      return result;
  } else
    process.nextTick(function() {
      try {
        var result = native.serialize(object, schema);
        if (result === null)
          callback(Error("Missing required fields during serializing " + schema), null);
        else
          callback(null, result);
      } catch (e) {
        callback(e, null);
      }
    })
};

pb_wrapper.prototype.info = function() { return this.native.info(); };

// backward compatibility
pb_wrapper.prototype.Parse = pb_wrapper.prototype.parse;
pb_wrapper.prototype.Serialize = pb_wrapper.prototype.serialize;

module.exports = pb_wrapper;
