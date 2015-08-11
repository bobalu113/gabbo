var net = require('net');
var md5 = require('md5');
var https = require('https');
require('longjohn');

var SIZE_WIDTH = 4;
var MD5_WIDTH = 32;
var ENCODING = 'utf8'

var server = net.createServer(function(socket) {
    var buffer = null;
    var cursor = -1;
    var size = 0;
    var checksum = 0;

    socket.on('data', function(data) {
        var pos = 0;
        if (buffer == null) {
            size += (data[0] & 0xFF) * 0x1000000;
            size += (data[1] & 0xFF) * 0x10000;
            size += (data[2] & 0xFF) * 0x100;
            size += (data[3] & 0xFF);
            checksum = data.toString(ENCODING,
                                     SIZE_WIDTH, SIZE_WIDTH + MD5_WIDTH);
            buffer = new Buffer(size);
            cursor = 0;
            pos = SIZE_WIDTH + MD5_WIDTH;
            console.log("New message: size " + size);
        }
        while (pos < data.length) {
            buffer[cursor++] = data[pos++];
        }
        if (cursor >= size) {
            if (md5(buffer.toString(ENCODING)) != checksum) {
                console.log("Checksums differ:\n"
                  + checksum + "\n"
                  + md5(buffer.toString(ENCODING)) + "\n");
            } else {
                console.log("Got message:\n" + buffer.toString(ENCODING));

                https.get({
                    hostname: 'api.github.com',
                    path: '/repos/bobalu113/gabbo/pulls',
                    headers: {'User-agent': 'Rodney for Gabbo'}
                },
                function(res) {
                    console.log("PR req status code: " + res.statusCode);
                    var data = [];
                    res.on('data', function(d) {
                        data.push(d);
                    }).on('error', function(e) {
                        console.error(e);
                    }).on('end', function() {
                        var pullRequests = Buffer.concat(data);
                        var header = new Buffer(SIZE_WIDTH + MD5_WIDTH);
                        var size = pullRequests.length;
                        var checksum = md5(pullRequests.toString(ENCODING));
                        header[0] = (size & 0xFFFFFFFF) / 0x1000000;
                        header[1] = (size & 0xFFFFFF) / 0x10000;
                        header[2] = (size & 0xFFFF) / 0x100;
                        header[3] = (size & 0xFF);
                        header.write(checksum, SIZE_WIDTH, MD5_WIDTH,
                                     ENCODING);
                        console.log("Got pull requests: size " + size);

                        if (res.statusCode == 200) {
                            socket.write(Buffer.concat([header,
                                                        pullRequests]),
                                         ENCODING);
                        }
                    });
                }).on('error', function(e) {
                    console.error(e);
                });

                buffer = null;
                cursor = -1;
                size = 0;
                checksum = 0;
            }
        }

    })

});

//server.listen(80, '127.0.0.1');
server.listen(2080, '66.220.23.27');
