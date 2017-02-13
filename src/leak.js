var fs = require("fs")

var p = require("../protobuf")

var obj = {
	"name": "testtesttesttesttest",
	"n64": 123,
	"r": []
}

var heapUsed;
var last = 0;
var desc = fs.readFileSync(__dirname + "/test.desc");

function run() {
	var rss = process.memoryUsage().rss;
	console.error('RSS: ' + rss + ', Delta: ' + (rss-last));
	last = rss;

	for (let i = 0; i < 2000; i++) {
		var pb = new p(desc);
	}
	setTimeout(run, 500);
}
run();
