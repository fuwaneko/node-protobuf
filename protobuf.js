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

	this.pool = protobuf.init(descriptor, int64)
}

pb_wrapper.prototype.parse = function() {
	if (arguments.length < 2)
		throw new Error("Invalid arguments")

	var buffer = arguments[0]
	var schema = arguments[1]
	var callback = arguments[2] || null

	if (!Buffer.isBuffer(buffer))
		throw new Error("First argument must be a Buffer")

	if (callback === null)
		return protobuf.parse(this.pool, buffer, schema)
	else
		protobuf.parseAsync(this.pool, buffer, schema, callback)
}

pb_wrapper.prototype.serialize = function() {
	if (arguments.length < 2)
		throw new Error("Invalid arguments")

	var object = arguments[0]
	var schema = arguments[1]
	var callback = arguments[2] || null

	if (callback === null)
		return protobuf.serialize(this.pool, object, schema)
	else
		protobuf.serializeAsync(this.pool, object, schema, callback)
}

// backward compatibility
pb_wrapper.prototype.Parse = pb_wrapper.prototype.parse
pb_wrapper.prototype.Serialize = pb_wrapper.prototype.serialize

module.exports = pb_wrapper