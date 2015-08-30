var gitcore = require('./gitcore.js');

exports.query = query;

function apiUrl(path) {
  return "https://api.github.com/repos/bobalu113/eotl-mudlib" + path;
}

function query(query, onFulfilled, onRejected) {
  var onFulfilledWrapper = function(data) {
    gitcore.teardownCache();
    onFulfilled(data);
  };
  var onRejectedWrapper = function(data) {
    gitcore.teardownCache();
    onRejected(data);
  };

  var get;
  if (query == "pullRequests") {
    get = function() {
      getPullRequests(onFulfilledWrapper, onRejectedWrapper)
    };
  } else if (query.substring(0, 12) == "pullRequest.") {
    var pos = query.indexOf(".", 12);
    if (pos < 0) {
      var number = query.substring(12);
      get = function() {
        getPullRequest(onFulfilledWrapper, onRejectedWrapper, number);
      };
    } else {
      var number = query.substring(12, pos);
      var pos2 = query.indexOf(".", pos + 1);
      if (pos2 >= 0) {
        var op = query.substring(pos + 1, pos2)
        if (op == "diff") {
          var file = query.substring(pos2 + 1);
          get = function() {
            getPullReqReview(onFulfilledWrapper, onRejectedWrapper, number,
                             file);
          }
        }
      }
    }
  }

  gitcore.setupCache();
  if (get) {
    get();
  }
  else {
    onRejectedWrapper("Unsupported operation: " + query);
  }
}

function getPullRequests(onFulfilled, onRejected) {
  gitcore.apiRequest(onFulfilled,
                     onRejected,
                     apiUrl('/pulls'),
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
    gitcore.apiRequest(filesFulfilled,
                       onRejected,
                       diffUrl,
                       marshallFiles);
  }
  var commentsRequest = function(data) {
    var commentsFulfilled = function(commentsData) {
      data.comments = commentsData;
      filesRequest(data);
    };
    gitcore.apiRequest(commentsFulfilled,
                       onRejected,
                       apiUrl('/issues/' + number + '/comments'),
                       marshallComments);
  }
  var issueRequest = function(data) {
    var issueFulfilled = function(issueData) {
      for (var attr in issueData) { data[attr] = issueData[attr]; }
      commentsRequest(data);
    };
    gitcore.apiRequest(issueFulfilled,
                       onRejected,
                       apiUrl('/issues/' + number),
                       marshallIssue);
  }
  gitcore.apiRequest(issueRequest,
                     onRejected,
                     apiUrl('/pulls/' + number),
                     marshallPullRequest);
}

function getPullReqReview(onFulfilled, onRejected, number, file) {
  var reviewCommentsRequest = function(data) {
    var reviewCommentsFulfilled = function(reviewCommentsData) {
      data.comments = reviewCommentsData;
      onFulfilled(data);
    }
    var marshallReviewCommentsWrapper = function(buffer) {
      return marshallReviewComments(buffer, data.path);
    }
    gitcore.apiRequest(reviewCommentsFulfilled,
                       onRejected,
                       apiUrl('/pulls/' + number + '/comments'),
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
    gitcore.apiRequest(diffFulfilled,
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
    gitcore.apiRequest(filesFulfilled,
                       onRejected,
                       diffUrl,
                       marshallFiles);
  }
  gitcore.apiRequest(filesRequest,
                     onRejected,
                     apiUrl('/pulls/' + number),
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

