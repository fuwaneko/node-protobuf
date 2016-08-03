var assert = require("assert")
var fs = require("fs")

var p = require("../protobuf")
var pb = new p(fs.readFileSync(__dirname + "/test.desc"))
var pbUnknown = new p(fs.readFileSync(__dirname + "/testUnknown.desc"))

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
			assert.throws(function() {
				pb.serialize(obj, "I don't exist")
			}, Error)
		})

		it("Should throw an error on missing required fields", function() {
			assert.throws(function() {
				pb.serialize({}, "tk.tewi.Test")
			})
		})

		it("Should throw an error on null required fields", function() {
			assert.throws(function() {
				pb.serialize({
					name: null
				}, "tk.tewi.Test")
			})
		})

		it("Should serialize asynchronously", function(done) {
			pb.serialize(obj, "tk.tewi.Test", function(error, buf) {
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

			pb.parse(buffer, "tk.tewi.Test", function(error, parsed) {
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

		// Typed arrays are hilariously broken before 0.12
		if (Number(process.version.match(/^v(\d+\.\d+)/)[1]) > 0.11) {
		it("Should return repeated int32 fields as typed array", function() {
			var obj = {
				"name": "test",
				"n64": 123,
			        "r": new Int32Array([1,2,3,4])
			}

			var buffer = pb.serialize(obj, "tk.tewi.Test")
			var parsed = pb.parse(buffer, "tk.tewi.Test")

			assert.ok(parsed.r instanceof Int32Array)
			assert.deepEqual(obj.r, parsed.r)
		})

		it("Should return repeated int32 fields as non-typed array", function() {
			var obj = {
				"name": "test",
				"n64": 123,
			        "r": new Int32Array([1,2,3,4])
			}

			var buffer = pb.serialize(obj, "tk.tewi.Test")
			var parsed = pb.parse(buffer, "tk.tewi.Test", null, null, null, false);

			assert.deepEqual(obj.r, parsed.r)
			assert.notEqual(parsed.r instanceof Int32Array, true,
				'Should not be an Int32Array');
		})
		}
	})

	describe("Parse With Unknown", function() {
		it("Should parse a buffer and return exactly the same object", function() {
			var buffer = pb.serialize(obj, "tk.tewi.Test")
			var parsed = pb.parseWithUnknown(buffer, "tk.tewi.Test")

			// We add an extra key here for unknownFields
			obj['$unknownFields'] = [];
			assert.deepEqual(obj, parsed)
		})

		it("Should do previous step asynchronously", function(done) {
			var buffer = pb.serialize(obj, "tk.tewi.Test")
			var isSync = true;
			pb.parseWithUnknown(buffer, "tk.tewi.Test", function(error, parsed) {
				assert.deepEqual(obj, parsed)
				assert.equal(isSync, false, 'Did not run async')
				done()
			})
			isSync = false;
		})

		it("Should throw an error on invalid argument", function() {
			assert.throws(function() {
				pb.parseWithUnknown("Invalid", "tk.tewi.Test")
			}, Error)
		})

		it("Should throw an error on invalid buffer", function() {
			assert.throws(function() {
				pb.parseWithUnknown(new Buffer("Invalid"), "tk.tewi.Test")
			}, Error)
		})

		it("Should throw an error on invalid schema", function() {
			assert.throws(function() {
				var buffer = pb.serialize(obj, "tk.tewi.Test")
				pb.parseWithUnknown(buffer, "I don't exist")
			}, Error)
		})

		it("Should ignore optional null fields", function() {
			var objWithNull = {
				"name": "test",
				"n64": 123,
				"value": null,
				"r": [],
				'$unknownFields': []
			}

			var buffer = pb.serialize(objWithNull, "tk.tewi.Test")
			var parsed = pb.parseWithUnknown(buffer, "tk.tewi.Test")

			delete objWithNull.value;
			assert.deepEqual(objWithNull, parsed)
		})

		it("Should ignore optional undefined fields", function() {
			var objWithNull = {
				"name": "test",
				"n64": 123,
				"value": undefined,
				"r": [],
				'$unknownFields': []
			}

			var buffer = pb.serialize(objWithNull, "tk.tewi.Test")
			var parsed = pb.parseWithUnknown(buffer, "tk.tewi.Test")

			delete objWithNull.value;
			assert.deepEqual(objWithNull, parsed)
		})

		it("Should tell us about unknownFields", function() {
			var objWithNull = {
				"name": "test",
				"n64": 123,
				"value": undefined,
				"r": [],
				'iDontBelong': 'hereatall',
			}

			var buffer = pbUnknown.serialize(objWithNull, "tk.tewi.Test")
			var parsed = pb.parseWithUnknown(buffer, "tk.tewi.Test")

			delete objWithNull.value;
			assert.deepEqual({
				'$unknownFields': [{ number: 7, type: 'TYPE_LENGTH_DELIMITED'}],
				"n64": 123,
				"r": [],
				"name": "test",
			}, parsed)
		})

		// Typed arrays are hilariously broken before 0.12
		if (Number(process.version.match(/^v(\d+\.\d+)/)[1]) > 0.11) {
		it("Should return repeated int32 fields as typed array", function() {
			var obj = {
				"name": "test",
				"n64": 123,
			        "r": new Int32Array([1,2,3,4])
			}

			var buffer = pb.serialize(obj, "tk.tewi.Test")
			var parsed = pb.parseWithUnknown(buffer, "tk.tewi.Test")

			assert.deepEqual(obj.r, parsed.r)
			assert.ok(parsed.r instanceof Int32Array)
		})

		it("Should return repeated int32 fields as non-typed array", function() {
			var obj = {
				"name": "test",
				"n64": 123,
			        "r": new Int32Array([1,2,3,4])
			}

			var buffer = pb.serialize(obj, "tk.tewi.Test")
			var parsed = pb.parseWithUnknown(buffer, "tk.tewi.Test", null, null, null, false);

			assert.deepEqual(obj.r, parsed.r)
			assert.notEqual(parsed.r instanceof Int32Array, true,
				'Should not be an Int32Array');
		})
		}
	})

	describe("Info", function() {
		it("Should return correct info about descriptor", function() {
			var info = pb.info()
			var compare = ["tk.tewi.Test", "tk.tewi.Data"]

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
