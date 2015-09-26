var assert = require("assert")
var fs = require("fs")

var p = require("../protobuf")
var pb = new p(fs.readFileSync(__dirname + "/test.desc"))

describe("Basic", function() {
	var obj = {
		"name": "test",
		"n64": 123,
		"r": []
	}

	describe("Serialize", function() {
		it("Should serialize object according to schema", function() {
			pb.serialize(obj, "tk.tewi.Test")
		})

		it("Should throw an error on invalid schema", function() {
			assert.throws(function () {
				pb.serialize(obj, "I don't exist")
			}, Error)
		})

		it("Should throw an error on missing required fields", function() {
			assert.throws(function () {
				pb.serialize({}, "tk.tewi.Test")
			})
		})

		it("Should throw an error on null required fields", function() {
			assert.throws(function () {
				pb.serialize({
					name: null
				}, "tk.tewi.Test")
			})
		})

		it("Should serialize asynchronously", function(done) {
			pb.serialize(obj, "tk.tewi.Test", function (error, buf) {
				if (!Buffer.isBuffer(buf)) throw new Error("Invalid result")
				done()
			})
		})
	})

	describe("Parse", function() {
		it("Should parse a buffer and return exactly the same object", function() {
			var buffer = pb.serialize(obj, "tk.tewi.Test")
			var parsed = pb.parse(buffer, "tk.tewi.Test")
			assert.deepEqual(obj, parsed)
		})

		it("Should do previous step asynchronously", function(done) {
			var buffer = pb.serialize(obj, "tk.tewi.Test")

			pb.parse(buffer, "tk.tewi.Test", function (error, parsed) {
				assert.deepEqual(obj, parsed)
				done()
			})
		})

		it("Should throw an error on invalid argument", function() {
			assert.throws(function() {
				pb.parse("Invalid", "tk.tewi.Test")
			}, Error)
		})

		it("Should throw an error on invalid buffer", function() {
			assert.throws(function() {
				pb.parse(new Buffer("Invalid"), "tk.tewi.Test")
			}, Error)
		})

		it("Should throw an error on invalid schema", function() {
			assert.throws(function() {
				var buffer = pb.serialize(obj, "tk.tewi.Test")
				pb.parse(buffer, "I don't exist")
			}, Error)
		})

		it("Should ignore optional null fields", function() {
			var objWithNull = {
				"name": "test",
				"n64": 123,
				"value": null,
				"r": []
			}

			var buffer = pb.serialize(objWithNull, "tk.tewi.Test")
			var parsed = pb.parse(buffer, "tk.tewi.Test")

			delete objWithNull.value;
			assert.deepEqual(objWithNull, parsed)
		})

		it("Should ignore optional undefined fields", function() {
			var objWithNull = {
				"name": "test",
				"n64": 123,
				"value": undefined,
				"r": []
			}

			var buffer = pb.serialize(objWithNull, "tk.tewi.Test")
			var parsed = pb.parse(buffer, "tk.tewi.Test")

			delete objWithNull.value;
			assert.deepEqual(objWithNull, parsed)
		})
	})

	describe("Info", function() {
		it("Should return correct info about descriptor", function() {
			var info = pb.info()
			var compare = [ "tk.tewi.Test", "tk.tewi.Data" ]

			assert.deepEqual(info, compare)
		})
	})

	describe("Behaviour", function() {
		it("Should return empty repeated as empty arrays", function() {
			var obj = {
				"name": "test",
				"n64": 123,
				"r": []
			}

			var buffer = pb.serialize(obj, "tk.tewi.Test")
			var parsed = pb.parse(buffer, "tk.tewi.Test")

			delete obj.value;
			assert.deepEqual(obj, parsed)
		})
	})
})
