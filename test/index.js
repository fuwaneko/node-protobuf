var expect = require('chai').expect;
var fs = require('fs');
var path = require('path');
var exec = require('child_process').exec;
var Protobuf = require('../build/Release/protobuf.node').Protobuf;

describe('node-protobuf', function () {

    var protobuf, desc, serialized, parsed;
    var obj = {
        "name": 'Ada Lovelace',
        "id": 2332,
        "email": "ada@test.com"
    };

    var fixturesDir = path.join(__dirname, './fixtures');
    var addressbookPathBase = fixturesDir + '/addressbook';

    before(function (done) {
        var cmd = 'protoc ' + addressbookPathBase + '.proto --proto_path=' + fixturesDir + ' --descriptor_set_out=' + addressbookPathBase + '.desc';

        exec(cmd, function (err, stdout, stderr) {

            expect(err).to.not.exist;
            expect(stderr).to.equal('');
            done();
        });
    });

    before(function (done) {

        fs.readFile(addressbookPathBase + '.desc', function (err, data) {

            expect(err).to.not.exist;
            desc = data;
            done();
        });
    });

    it('creates a new protobuf instance from desc', function (done) {

        protobuf = new Protobuf(desc);
        done();
    });

    it('creates a new serialized protobuf object', function (done) {

        serialized = protobuf.Serialize(obj, 'Person');
        done();
    });

    it('parses a serialized protobuf object', function (done) {

        parsed = protobuf.Parse(serialized, 'Person');
        done();
    });

    it('compares the original obj to the parsed protobuf', function (done) {

        expect(obj.name).to.equal(parsed.name);
        expect(obj.id).to.equal(parsed.id);
        expect(obj.email).to.equal(parsed.email);
        done();
    });
});
