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

        var incoming = JSON.parse(buffer);
        var transactionId = incoming.transactionId;
        var query = incoming.query;
        var send = function(data) {
          sendResponse(socket, transactionId, data);
        };

        if (query == "github.pullRequests") {
          getPullRequests(send, console.error);
        } else if (query.substring(0, 19) == "github.pullRequest.") {
          getPullRequest(send, console.error, query.substring(19));
        } else {
          throw new Error("Unsupported operation: " + query);
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

function apiRequest(onFulfilled, onRejected, path, marshall) {
  https.get({
    hostname: 'api.github.com',
    path: path,
    headers: {'User-agent': 'Rodney for Gabbo'}
  }, function(res) {
    console.log("PR req status code: " + res.statusCode);
    var data = [];
    res.on('data', function(d) {
      data.push(d);
    }).on('error', function(e) {
      onRejected(e);
    }).on('end', function() {
      if (res.statusCode == 200) {
        onFulfilled(marshall(Buffer.concat(data)));
      }
    });
  }).on('error', function(e) {
    onRejected(e);
  });
}

function getPullRequests(onFulfilled, onRejected) {
  apiRequest(onFulfilled,
             onRejected,
             '/repos/bobalu113/gabbo/pulls',
             marshallPullRequests);
}

function getPullRequest(onFulfilled, onRejected, number) {
  var commentsRequest = function(data) {
    var commentsFulfilled = function(commentsData) {
      data.comments = commentsData;
      onFulfilled(data);
    };
    apiRequest(commentsFulfilled,
               onRejected,
               '/repos/bobalu113/gabbo/issues/' + number + '/comments',
               marshallComments);
  }
  var issueRequest = function(data) {
    var issueFulfilled = function(issueData) {
      for (var attr in issueData) { data[attr] = issueData[attr]; }
      commentsRequest(data);
    };
    apiRequest(issueFulfilled,
               onRejected,
               '/repos/bobalu113/gabbo/issues/' + number,
               marshallIssue);
  }
  apiRequest(issueRequest,
             onRejected,
             '/repos/bobalu113/gabbo/pulls/' + number,
             marshallPullRequest);
}

function marshallPullRequests(buffer) {
  var data = JSON.parse(buffer);
  var out = data.map(function(x) {
    return {
          "number": x.number,
           "state": x.state,
           "title": x.title,
            "body": x.body,
      "created_at": x.created_at,
      "updated_at": x.updated_at,
       "closed_at": x.closed_at,
       "merged_at": x.merged_at,
            "user": x.user.login,
    }
  });
  return out;
}

function marshallPullRequest(buffer) {
  var data = JSON.parse(buffer);
  var out = {
           "number": data.number,
            "state": data.state,
            "title": data.title,
             "body": data.body,
       "created_at": data.created_at,
       "updated_at": data.updated_at,
        "closed_at": data.closed_at,
        "merged_at": data.merged_at,
             "user": data.user.login,
           "merged": data.merged,
        "mergeable": data.mergeable,
//         "comments": data.comments,
          "commits": data.commits,
        "additions": data.additions,
        "deletions": data.deletions,
    "changed_files": data.changed_files
  };
  return out;
}

function marshallIssue(buffer) {
  var data = JSON.parse(buffer);
  var out = {

  };
  return out;
}

function marshallComments(buffer) {
  var data = JSON.parse(buffer);
  var out = data.map(function(x) {
    return {
            "user": x.user.login,
            "body": x.body,
      "created_at": x.created_at,
      "updated_at": x.updated_at,
    }
  });
  return out;
}

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
  console.log("Got pull requests: size " + size);
  socket.write(Buffer.concat([header, body]), ENCODING);
}
