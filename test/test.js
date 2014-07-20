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
		
		it("Should throw an error on missing required fields", function() {
			assert.throws(function () {
				pb.serialize({}, "Test")
			})
		})

		it("Should throw an error on null required fields", function() {
			assert.throws(function () {
				pb.serialize({
					name: null
				}, "Test")
			})
		})
		
		it("Should serialize asynchronously", function(done) {
			pb.serialize(obj, "Test", function (error, buf) {
				if (!Buffer.isBuffer(buf)) throw new Error("Invalid result")
				done()
			})
		})
	})

	describe("Parse", function() {
		it("Should parse a buffer and return exactly the same object", function() {
			var buffer = pb.serialize(obj, "Test")
			var parsed = pb.parse(buffer, "Test")
			assert.deepEqual(obj, parsed)
		})
		
		it("Should do previous step asynchronously", function(done) {
			var buffer = pb.serialize(obj, "Test")
			
			pb.parse(buffer, "Test", function (error, parsed) {
				assert.deepEqual(obj, parsed)
				done()
			})
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

	describe("Info", function() {
		it("Should return correct info about descriptor", function() {
			var info = pb.info()
			var compare = [ "Test", "Data" ]

			assert.deepEqual(info, compare)
		})
	})
})