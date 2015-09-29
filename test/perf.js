var fs = require("fs");
var p = require("../protobuf");
var desc = fs.readFileSync(__dirname + "/points.desc");
var pb = new p(desc);

var points = [];
for (var i = 0; i < 100000; i++) {
	points.push({
		i: 1388534400000,
		j: 1388534400000
	});
}

/*var t1 = process.hrtime()

for (var i = 0; i < 1000; i++) {
	var serJSON = JSON.stringify({
		points: points
	});
	var deserJSON = JSON.parse(serJSON);
}

var t2 = process.hrtime()

var diff = t2[0] * 1e9 + t2[1] - (t1[0] * 1e9 + t1[1])
console.log("Time JSON:", diff / 1000);*/

var t1 = process.hrtime()
var serPB = pb.serialize({
	points: points
}, 'Points');
var deserPB = pb.parse(serPB, 'Points');
var t2 = process.hrtime()

var diff = t2[0] * 1e9 + t2[1] - (t1[0] * 1e9 + t1[1])
console.log("  Time PB:", diff);
