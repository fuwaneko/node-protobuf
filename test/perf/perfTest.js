var fs = require("fs");
var p = require("../../protobuf");
var desc = fs.readFileSync(__dirname + "/perfTest.desc");
var pb = new p(desc);

var message = {
    uint64Field: Date.now(),
    uint32Field: Date.now() / 1000,
    stringField: 'sample string value',
    floatField: 1.23456789012345,
    enumField: 'THIRD',
    boolField: true,
    messageField: {
        field1: 'value1',
        field2: 'value2',
        field3: 'value3'
    }
};

console.log('serializing and parsing a message 1,000,000 times...');

console.time('serialize');
for (var i = 0; i < 1000000; i++) {
    var serPB = pb.serialize(message, 'PerfTest');
}
console.timeEnd('serialize');

var buffer = pb.serialize(message, 'PerfTest');
console.time('parse');
for (var i = 0; i < 1000000; i++) {
    var deserPB = pb.parse(buffer, 'PerfTest');
}
console.timeEnd('parse');
