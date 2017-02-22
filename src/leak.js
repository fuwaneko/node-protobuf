var fs = require("fs");

var p = require("../protobuf");
var t = require('assert');

var obj = {
	"name": "testtesttesttesttest",
	"n64": 123,
	"r": []
}

var heapUsed;
var last = 0;
var desc = fs.readFileSync(__dirname + "/../test/test.desc");
var otherdesc = fs.readFileSync(__dirname + '/../test/testUnknown.desc');

function run() {
	var rss = process.memoryUsage().rss;
	console.error('RSS: ' + rss + ', Delta: ' + (rss-last));
	last = rss;
	let x;
	let xdecoded;

	for (let i = 0; i < 2000; i++) {
		{
			// Put this in an isolated scope so we can gc it properly.
			var pb = new p(desc);
			x = pb.serialize({
				lies: 'hey',
				n64: 121,
				name: 'yeah'
			}, 'tk.tewi.Test');
			xdecoded = pb.parse(x, 'tk.tewi.Test');
			try {
				pb.parse(null, 'tk.tewi.Test');
			} catch (e) {}
		}
		{
			var pb = new p(otherdesc);
			x = pb.serialize({
				lies: 'hey',
				n64: 121,
				name: 'yeah'
			}, 'tk.tewi.Test');
			xdecoded = pb.parse(x, 'tk.tewi.Test');
		}
		global.gc();
	}

	setImmediate(run);
}
run();
