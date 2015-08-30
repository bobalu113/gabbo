var net = require('net');
var md5 = require('md5');
var url = require('url');
var request = require('request');
require('longjohn');

var SIZE_WIDTH = 4;
var MD5_WIDTH = 32;
var ENCODING = 'utf8'

var apiCache;

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
            apiCache = null;
          };

          var get = null;
          if (query == "github.pullRequests") {
            get = function() { getPullRequests(send, console.error) };
          } else if (query.substring(0, 19) == "github.pullRequest.") {
            var pos = query.indexOf(".", 19);
            if (pos < 0) {
              var number = query.substring(19);
              get = function() {
                getPullRequest(send, console.error, number);
              };
            } else {
              var number = query.substring(19, pos);
              var pos2 = query.indexOf(".", pos + 1);
              if (pos2 >= 0) {
                var op = query.substring(pos + 1, pos2)
                if (op == "diff") {
                  var file = query.substring(pos2 + 1);
                  get = function() {
                    getDiffRequest(send, console.error, number, file);
                  }
                }
              }
            }
          }

          if (get) {
            apiCache = { };
            get();
          }
          else {
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
  if (target in apiCache) {
    onFulfilled(marshall(apiCache[target]));
  } else {
    var options = {
      url: url.parse(target),
      headers: {'User-agent': 'Rodney for Gabbo'},
    };
    request(options, function(error, res, body) {
      if (error) {
        onRejected(error);
      } else {
        console.log("apiRequest status code: " + res.statusCode +
                    ", target: " + target);
        if (res.statusCode == 200) {
          apiCache[target] = body;
          onFulfilled(marshall(body));
        }
      }
    });
  }
}

function getPullRequests(onFulfilled, onRejected) {
  apiRequest(onFulfilled,
             onRejected,
             apiUrl('/repos/bobalu113/gabbo/pulls'),
             marshallPullRequests);
}

function getPullRequest(onFulfilled, onRejected, number) {
  var filesRequest = function(data) {
    var diffUrl = data.diff_url;
    var filesFulfilled = function(filesData) {
      delete data.diff_url;
      data.files = filesData;
      onFulfilled(data);
    };
    apiRequest(filesFulfilled,
               onRejected,
               diffUrl,
               marshallFiles);
  }
  var commentsRequest = function(data) {
    var commentsFulfilled = function(commentsData) {
      data.comments = commentsData;
      filesRequest(data);
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

function getDiffRequest(onFulfilled, onRejected, number, file) {
  var reviewCommentsRequest = function(data) {
    var reviewCommentsFulfilled = function(reviewCommentsData) {
      data.comments = reviewCommentsData;
      onFulfilled(data);
    }
    var marshallReviewCommentsWrapper = function(buffer) {
      return marshallReviewComments(buffer, data.path);
    }
    apiRequest(reviewCommentsFulfilled,
               onRejected,
               apiUrl('/repos/bobalu113/gabbo/pulls/' + number +
                      '/comments'),
               marshallReviewCommentsWrapper);
  }
  var diffRequest = function(data) {
    var diffUrl = data.diff_url;
    delete data.diff_url;
    var diffFulfilled = function(diffData) {
      delete data.diff_url;
      for (var attr in diffData) { data[attr] = diffData[attr]; }
      reviewCommentsRequest(data);
    };
    var marshallDiffWrapper = function(buffer) {
      return marshallDiff(buffer, data.path);
    }
    apiRequest(diffFulfilled,
               onRejected,
               diffUrl,
               marshallDiffWrapper);
  }
  var filesRequest = function(data) {
    var diffUrl = data.diff_url;
    var filesFulfilled = function(filesData) {
      var fileIndex = -1;
      var tmp = parseInt(file, 10);
      if (file == tmp) {
        if ((tmp >= 0) && (tmp < filesData.length)) {
          file = filesData[tmp];
          fileIndex = tmp;
        }
      } else {
        fileIndex = filesData.indexOf(file);
      }
      if (fileIndex < 0) {
        onRejected(new Error("invalid file for diff"));
      } else {
        data.path = file;
        data.file = fileIndex;
        diffRequest(data);
      }
    };
    apiRequest(filesFulfilled,
               onRejected,
               diffUrl,
               marshallFiles);
  }
  apiRequest(filesRequest,
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

function marshallDiff(buffer, path) {
  var out = { };
  var i = 0;
  var lines = null;
  while (i < buffer.length) {
    var j = buffer.indexOf("\n", i);
    if (j == -1) { j = buffer.length; }
    var line = buffer.substring(i, j);
    if (lines) {
      var token = "diff --git ";
      if (line.substring(0, token.length) == token) {
        break;
      } else {
        lines.push(line);
      }
    } else {
      var token = "diff --git a/" + path + " b/" + path;
      if (line.substring(0, token.length) == token) {
        lines = [ line ];
      }
    }
    i = j + 1;
  }
  out.diff = lines;
  return out;
}

function marshallReviewComments(buffer, path) {
  var data = JSON.parse(buffer);
  var out = { };
  data.forEach(function(comment) {
    if (comment.path == path) {
      var pos = comment.position;
      if (!(pos in out)) {
        out[pos] = [ ];
      }
      out[pos].push({
              "user": comment.user.login,
              "body": comment.body,
        "created_at": marshallTimestamp(comment.created_at),
        "updated_at": marshallTimestamp(comment.updated_at),
      });
    }
  });
  return out;
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
