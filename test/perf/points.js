var fs = require("fs");
var p = require("../../protobuf");
var pjs = require("protobufjs")
var desc = fs.readFileSync(__dirname + "/points.desc");
var pbjsBuilder = pjs.loadProtoFile(__dirname + "/points.proto")

var pb = new p(desc);
var pointsBuilder = pbjsBuilder.build("Points")
var point = pbjsBuilder.build("Point")

var points = [];
for (var i = 0; i < 100000; i++) {
	points.push({
		i: 1388534400000,
		j: 1388534400000
	});
}

var par = []
for (var i = 0; i < 100000; i++) {
	par.push(new point(1388534400000, 1388534400000))
}
var pointsjs = new pointsBuilder(par)

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
for (var i = 0; i < 10; i++) {
	var serPB = pb.serialize({
		points: points
	}, 'Points');
	var deserPB = pb.parse(serPB, 'Points');
}
var t2 = process.hrtime()

var diff = t2[0] * 1e9 + t2[1] - (t1[0] * 1e9 + t1[1])
console.log("  Time PB:", diff / 10);

var t1 = process.hrtime()
for (var i = 0; i < 10; i++) {
	var serPB = pointsjs.encode()
	var deserPB = pointsBuilder.decode(serPB)
}
var t2 = process.hrtime()

var diff = t2[0] * 1e9 + t2[1] - (t1[0] * 1e9 + t1[1])
console.log("Time PBJS:", diff / 10);
