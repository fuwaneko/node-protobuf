var assert = require("assert")
var fs = require("fs")

var p = require("../protobuf")
var pb = new p(fs.readFileSync(__dirname + "/test.desc"))

describe("Basic", function() {
	var obj = {
		"name": "test",
		"n64": 123,
	}

	describe("Serialize", function() {
		it("Should serialize object according to schema", function() {
			pb.serialize(obj, "Test")
		})

		it("Should throw an error on invalid schema", function() {
			assert.throws(function () {
				pb.serialize(obj, "I don't exist")
			}, Error)
		})
	})

	describe("Parse", function() {
		it("Should parse a buffer and return exactly the same object", function() {
			var buffer = pb.serialize(obj, "Test")
			var parsed = pb.parse(buffer, "Test")
			assert.deepEqual(obj, parsed)
		})

		it("Should throw an error on invalid argument", function() {
			assert.throws(function() {
				pb.parse("Invalid", "Test")
			}, Error)
		})

		it("Should throw an error on invalid buffer", function() {
			assert.throws(function() {
				pb.parse(new Buffer("Invalid"), "Test")
			}, Error)
		})

		it("Should throw an error on invalid schema", function() {
			assert.throws(function() {
				var buffer = pb.serialize(obj, "Test")
				pb.parse(buffer, "I don't exist")
			}, Error)
		})

		it("Should ignore optional null fields", function() {
			var objWithNull = {
				"name": "test",
				"n64": 123,
				"value": null
			}

			var buffer = pb.serialize(objWithNull, "Test")
			var parsed = pb.parse(buffer, "Test")

			delete objWithNull.value;
			assert.deepEqual(objWithNull, parsed)
		})

		it("Should ignore optional undefined fields", function() {
			var objWithNull = {
				"name": "test",
				"n64": 123,
				"value": undefined
			}
			
			var buffer = pb.serialize(objWithNull, "Test")
			var parsed = pb.parse(buffer, "Test")

			delete objWithNull.value;
			assert.deepEqual(objWithNull, parsed)
		})
	})
})

/*
// test optional
var obj = {
	"name": "test",
	"n64": 123,
	"type": "DEFAULT"
}

var buf = pb.Serialize(obj, "Test")
var newObj = pb.Parse(buf, "Test")

console.log("Optional fields test 1:\n", require("util").inspect(newObj, {colors: true, showHidden: true, depth: null}))

obj = {
	"name": "test",
	"n64": 123,
	"value": "messages",
	"data": {
		"sender": "me",
		"recipient": "other",
		"message": "hello"
	},
	"type": 2
}

buf = pb.Serialize(obj, "Test")
newObj = pb.Parse(buf, "Test")

console.log("Optional fields test 2:\n", require("util").inspect(newObj, {colors: true, showHidden: true, depth: null}))

try {
	newObj = pb.Parse(new Buffer("Bad buffer"), "Test")
} catch (e) {
	console.log(e)
	if (e.trace)
		console.log(e.trace)
}

try {
	newObj = pb.Parse("Bad argument", "Test")
} catch (e) {
	console.log(e)
	if (e.trace)
		console.log(e.trace)
}

try {
	buf = pb.Serialize(obj, "Idonotexist")
} catch (e) {
	console.log(e)
	if (e.trace)
		console.log(e.trace)
}

console.log("Testing memory leaks")

function leak() {
	try {
		var buf = pb.Serialize(obj, "Test")
		buf = null
	} catch (e) {
		console.log(e)
		if (e.trace)
			console.log(e.trace)
	}
}

var i = 0
while (i++ < 10000) {
	setTimeout(leak, 100)
}
*/
