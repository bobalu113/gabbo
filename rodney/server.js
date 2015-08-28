var net = require('net');
var md5 = require('md5');
var url = require('url');
var request = require('request');
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
          var send = function(data) {
            sendResponse(socket, transactionId, data);
          };

          if (query == "github.pullRequests") {
            getPullRequests(send, console.error);
          } else if (query.substring(0, 19) == "github.pullRequest.") {
            getPullRequest(send, console.error, query.substring(19));
          } else {
            console.warn("Unsupported operation: " + query);
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

function apiUrl(path) {
  return "https://api.github.com" + path;
}

function apiRequest(onFulfilled, onRejected, target, marshall) {
  var options = {
    url: url.parse(target),
    headers: {'User-agent': 'Rodney for Gabbo'},
  };
  request(options, function(error, res, body) {
    if (error) {
      onRejected(error);
    } else {
      console.log("apiRequest status code: " + res.statusCode);
      if (res.statusCode == 200) {
        onFulfilled(marshall(body));
      }
    }
  });
}

function getPullRequests(onFulfilled, onRejected) {
  apiRequest(onFulfilled,
             onRejected,
             apiUrl('/repos/bobalu113/gabbo/pulls'),
             marshallPullRequests);
}

function getPullRequest(onFulfilled, onRejected, number) {
  var diffRequest = function(data) {
    var diffUrl = data.diff_url;
    var diffFulfilled = function(diffData) {
      delete data.diff_url;
      data.files = diffData;
      onFulfilled(data);
    };
    apiRequest(diffFulfilled,
               onRejected,
               diffUrl,
               marshallFiles);
  }
  var commentsRequest = function(data) {
    var commentsFulfilled = function(commentsData) {
      data.comments = commentsData;
      diffRequest(data);
    };
    apiRequest(commentsFulfilled,
               onRejected,
               apiUrl('/repos/bobalu113/gabbo/issues/' + number +
                      '/comments'),
               marshallComments);
  }
  var issueRequest = function(data) {
    var issueFulfilled = function(issueData) {
      for (var attr in issueData) { data[attr] = issueData[attr]; }
      commentsRequest(data);
    };
    apiRequest(issueFulfilled,
               onRejected,
               apiUrl('/repos/bobalu113/gabbo/issues/' + number),
               marshallIssue);
  }
  apiRequest(issueRequest,
             onRejected,
             apiUrl('/repos/bobalu113/gabbo/pulls/' + number),
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
      "created_at": marshallTimestamp(x.created_at),
      "updated_at": marshallTimestamp(x.updated_at),
       "closed_at": marshallTimestamp(x.closed_at),
       "merged_at": marshallTimestamp(x.merged_at),
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
       "created_at": marshallTimestamp(data.created_at),
       "updated_at": marshallTimestamp(data.updated_at),
        "closed_at": marshallTimestamp(data.closed_at),
        "merged_at": marshallTimestamp(data.merged_at),
             "user": data.user.login,
           "merged": data.merged,
        "mergeable": data.mergeable,
//         "comments": data.comments,
          "commits": data.commits,
        "additions": data.additions,
        "deletions": data.deletions,
    "changed_files": data.changed_files,
         "diff_url": data.diff_url,
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
      "created_at": marshallTimestamp(x.created_at),
      "updated_at": marshallTimestamp(x.updated_at),
    }
  });
  return out;
}

function marshallCommits(buffer) {
  var data = JSON.parse(buffer);
  var out = data.map(function(x) {
    return {
            "user": x.user.login,
            "body": x.body,
      "created_at": marshallTimestamp(x.created_at),
      "updated_at": marshallTimestamp(x.updated_at),
    }
  });
  return out;
}

function marshallFiles(buffer) {
  var files = [ ];
  var i = 0;
  while (i < buffer.length) {
    var j = buffer.indexOf("\n", i);
    if (j == -1) { j = buffer.length; }
    var line = buffer.substring(i, j);
    if (line.substring(0, 6) == "+++ b/") {
      files.push(line.substr(6));
    }
    i = j + 1;
  }
  return files;
}

function marshallTimestamp(timestamp) {
  return Date.parse(timestamp) / 1000;
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
  console.log("Sending response: size " + size);
  socket.write(Buffer.concat([header, body]), ENCODING);
}
