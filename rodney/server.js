/**
 * The Rodney for Gabbo external request server.
 *
 * @module server
 * @requires  module:net
 * @requires  module:md5
 * @requires  module:github
 * @requires  module:gitlab
 * @requires  module:bitbucket
 */

var net = require('net');
var md5 = require('md5');

var github = require('./github.js');
var gitlab = require('./gitlab.js');
var bitbucket = require('./bitbucket.js');

const PORT = 2080;
const HOST = 'localhost';
//var HOST = 'mud.panterasbox.com';

const SIZE_WIDTH = 4;
const MD5_WIDTH = 32;
const ENCODING = 'utf8'

startServer();

/**
 * Start up the Rodney for Gabbo server.
 *
 * @function
 * @return {object} the newly started server instance
 */
function startServer() {
  var server = net.createServer(function(socket) {
    var state = { };
    resetState(state);
    socket.on('data', function(data) {
      if (handleRequest(data, state, socket)) {
        resetState(state);
      }
    });
  });
  server.listen(PORT, HOST);
  return server;
}

/**
 * Reset the current request state to its starting values. This state object
 * will read the headers in the first request object, then append subsequent
 * requests to a message buffer until the entire request has been received.
 * At that time the message will be passed off to handleRequest and the
 * request state should be reset.
 *
 * @function
 * @param {object}  state          - processing state of current request
 * @param {object}  state.buffer   - the Buffer containing partial message
 *                                   received so far
 * @param {number}  state.cursor   - the current position in the message
 * @param {number}  state.size     - the total size of the message
 * @param {string}  state.checksum - the md5 checksum of the message
 */
function resetState(state) {
  state.buffer = null;
  state.cursor = -1;
  state.size = 0;
  state.checksum = 0;
}

/**
 * Handle an incoming request. A single message may span multiple requests,
 * and the first request for a message will include size/checksum headers
 * in addition to the message being sent. The existing message state is
 * maintained by the calling scope and must be passed as a paramter.
 *
 * @function
 * @param  {object}   data        - the Buffer containing incoming data
 * @param  {object}   st          - processing state of current request
 * @param  {object}   socket      - the server socket for sending responses
 * @return {boolean}  true if message was completed, false to keep going
 */
function handleRequest(data, st, socket) {
  var pos = 0;
  if (st.buffer == null) {
    st.size += (data[0] & 0xFF) * 0x1000000;
    st.size += (data[1] & 0xFF) * 0x10000;
    st.size += (data[2] & 0xFF) * 0x100;
    st.size += (data[3] & 0xFF);
    st.checksum = data.toString(ENCODING, SIZE_WIDTH,
                                SIZE_WIDTH + MD5_WIDTH);
    st.buffer = new Buffer(st.size);
    st.cursor = 0;
    pos = SIZE_WIDTH + MD5_WIDTH;
    console.info("New message: size " + st.size);
  }
  while (pos < data.length) {
    st.buffer[st.cursor++] = data[pos++];
  }
  if (st.cursor >= st.size) {
    if (md5(st.buffer.toString(ENCODING)) != st.checksum) {
      console.warn(
        "Checksums differ:\n"
        + st.checksum + "\n"
        + md5(st.buffer.toString(ENCODING)) + "\n"
      );
    } else {
      console.info("Got message: size " + st.buffer.length);
      var valid = 1;
      var incoming;

      try {
        incoming = JSON.parse(st.buffer);
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

        if (!runQuery(query, send, console.error)) {
          console.error("Unsupported operation: " + query);
        }
      }

      return true;
    }
  }
  return false;
}

/**
 * Serialize data to JSON and send to client over socket.
 *
 * @function
 * @param  {object} socket        the socket to send the response over
 * @param  {string} transactionId the transactionId for this response
 * @param  {object} data          the data to serialize and send
 */
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

/**
 * Execute the specified query with onFulfilled and onRejected callbacks.
 *
 * @param  {string} query          the query to run
 * @param  {function} onFulfilled  called with result if query is successful
 * @param  {function} onRjected    called with error if query fails
 * @return {boolean}               true for valid query string, false
 *                                 otherwise
 */
function runQuery(query, onFulfilled, onRejected) {
  if (query.substring(0, 7) == "github.") {
    return github.query(query.substring(7), onFulfilled, onRejected);
  } else if (query.substring(0, 7) == "gitlab.") {
    return gitlab.query(query.substring(7), onFulfilled, onRejected);
  } else if (query.substring(0, 10) == "bitbucket.") {
    return bitbucket.query(query.substring(10), onFulfilled, onRejected);
  }
  return false;
}