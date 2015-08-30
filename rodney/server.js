var net = require('net');
var md5 = require('md5');
require('longjohn');

var github = require('./github.js');
var gitlab = require('./gitlab.js');

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
      checksum = data.toString(ENCODING, SIZE_WIDTH, SIZE_WIDTH + MD5_WIDTH);
      buffer = new Buffer(size);
      cursor = 0;
      pos = SIZE_WIDTH + MD5_WIDTH;
      console.info("New message: size " + size);
    }
    while (pos < data.length) {
      buffer[cursor++] = data[pos++];
    }
    if (cursor >= size) {
      if (md5(buffer.toString(ENCODING)) != checksum) {
        console.warn(
          "Checksums differ:\n"
          + checksum + "\n"
          + md5(buffer.toString(ENCODING)) + "\n"
        );
      } else {
        console.info("Got message: size " + buffer.length);
        var valid = 1;
        var incoming;

        try {
          incoming = JSON.parse(buffer);
        } catch(err) {
          console.error("Illegal request: JSON parse failed");
          valid = 0;
        }
        if (valid && !("transactionId" in incoming)) {
          console.error("Illegal request: missing transactionId");
          valid = 0;
        }

        if (valid) {
          var transactionId = incoming.transactionId;
          var query = incoming.query;
          console.info("Got query: " + query +
                        ", transactionId: " + transactionId);
          var send = function(data) {
            sendResponse(socket, transactionId, data);
          };

          if (query.substring(0, 7) == "github.") {
            github.query(query.substring(7), send, console.error);
          } else if (query.substring(0, 7) == "gitlab.") {
            gitlab.query(query.substring(7), send, console.error);
          } else {
            console.error("Unsupported operation: " + query);
          }
        }

        buffer = null;
        cursor = -1;
        size = 0;
        checksum = 0;
      }
    }

  })

});

server.listen(2080, '127.0.0.1');
//server.listen(2080, '66.220.23.27');

function sendResponse(socket, transactionId, data) {
  body = {
    transactionId: transactionId,
    body: data
  };
  body = JSON.stringify(body);
  body = new Buffer(body); // XXX this sucks
  var header = new Buffer(SIZE_WIDTH + MD5_WIDTH);
  var size = body.length;
  var checksum = md5(body.toString(ENCODING));
  header[0] = (size & 0xFFFFFFFF) / 0x1000000;
  header[1] = (size & 0xFFFFFF) / 0x10000;
  header[2] = (size & 0xFFFF) / 0x100;
  header[3] = (size & 0xFF);
  header.write(checksum, SIZE_WIDTH, MD5_WIDTH, ENCODING);
  console.log("Sending response: size " + size);
  socket.write(Buffer.concat([header, body]), ENCODING);
}
