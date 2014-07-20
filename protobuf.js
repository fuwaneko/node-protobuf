/**

This is the main wrapper for protobuf implementation

**/

var assert = require("assert")
var protobuf = require("bindings")("protobuf.node")

function pb_wrapper() {
	if (!(this instanceof pb_wrapper))
		return new pb_wrapper.apply(this, arguments)

	var descriptor = arguments[0] || null
	var int64 = arguments[1] || true

	assert(descriptor, "Descriptor must be provided")

	this.native = new protobuf.native(descriptor)
}

pb_wrapper.prototype.parse = function() {
	if (arguments.length < 2)
		throw new Error("Invalid arguments")

	var buffer = arguments[0]
	var schema = arguments[1]
	var callback = arguments[2] || null
	var native = this.native

	if (!Buffer.isBuffer(buffer))
		throw new Error("First argument must be a Buffer")

	if (callback === null) {
		var result = native.parse(buffer, schema)
		if (result === null)
			throw new Error("Unexpected error while parsing " + schema)
		else
			return result
	}
	else
		process.nextTick(function() {
			try {
				var result = native.parse(buffer, schema)
				callback(null, result)
			} catch (e) {
				callback(e, null)
			}
		})
}

pb_wrapper.prototype.serialize = function() {
	if (arguments.length < 2)
		throw new Error("Invalid arguments")

	var object = arguments[0]
	var schema = arguments[1]
	var callback = arguments[2] || null
	var native = this.native

	if (callback === null) {
		var result = native.serialize(object, schema)
		if (result === null)
			throw new Error("Missing required fields while serializing " + schema)
		else
			return result
	}
	else
		process.nextTick(function() {
			try {
				var result = native.serialize(object, schema)
				if (result === null)
					callback(Error("Missing required fields during serializing " + schema), null)
				else
					callback(null, result)
			} catch (e) {
				callback(e, null)
			}
		})
}

pb_wrapper.prototype.info = function() {
	return this.native.info();
}

// backward compatibility
pb_wrapper.prototype.Parse = pb_wrapper.prototype.parse
pb_wrapper.prototype.Serialize = pb_wrapper.prototype.serialize

module.exports = pb_wrapper