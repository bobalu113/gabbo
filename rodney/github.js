/**
 * The GitHub adapter for issue tracking and pull requests.
 * @module github
 * @requires  module:gitcore
 */

var Gitcore = require('./gitcore.js');
var gitcore = Gitcore(null, null);

exports.query = query;

/**
 * Return the GitHub API url for the specified path.
 *
 * @param  {string} path the path of the API request
 * @return {string}      the full URL for API request
 */
function apiUrl(path) {
  return "https://api.github.com/repos/eotl/eotl-mudlib" + path;
}

/**
 * Query the GitHub API.
 *
 * @param  {string}   query        the BitBucket query string
 * @param  {function} onFulfilled  query result will be passed to this
 *                                 callback
 * @param  {function} onRejected   errors will be passed to this callback
 * @return {boolean}               true for valid query string, false
 *                                 otherwise
 */
function query(query, onFulfilled, onRejected) {
  var onFulfilledWrapper = function(data) {
    gitcore.teardownCache();
    onFulfilled(data);
  };
  var onRejectedWrapper = function(data) {
    gitcore.teardownCache();
    onRejected(data);
  };

  var runQuery = getRunQuery(query, onFulfilledWrapper, onRejectedWrapper);
  if (runQuery) {
    gitcore.setupCache();
    runQuery();
    return true;
  }
  else {
    return false;
  }
}

/**
 * Get query runner function for specified query and callbacks.
 *
 * @param  {string}   query        the query to run
 * @param  {function} onFulfilled  query result will be passed to this
 *                                 callback
 * @param  {function} onRejected   errors will be passed to this callback
 * @return {function}              the function which will execute the query
 */
function getRunQuery(query, onFulfilled, onRejected) {
  var runQuery;

  if (query == "pullRequests") {
    runQuery = function() {
      getPullRequests(onFulfilled, onRejected)
    };
  } else if (query.substring(0, 12) == "pullRequest.") {
    var pos = query.indexOf(".", 12);
    if (pos < 0) {
      var id = query.substring(12);
      runQuery = function() {
        getPullRequest(onFulfilled, onRejected, id)
      };
    } else {
      var id = query.substring(12, pos);
      var pos2 = query.indexOf(".", pos + 1);
      if (pos2 >= 0) {
        var op = query.substring(pos + 1, pos2)
        if (op == "review") {
          var file = query.substring(pos2 + 1);
          runQuery = function() {
            getPullReqReview(onFulfilled, onRejected, id, file);
          }
        }
      }
    }
  }

  return runQuery;
}

/**
 * Execute API requests for pull request list query.
 *
 * @param  {function} onFulfilled query result will be passed to this callback
 * @param  {function} onRejected  errors will be passed to this callback
 */
function getPullRequests(onFulfilled, onRejected) {
  gitcore.apiRequest(onFulfilled,
                     onRejected,
                     apiUrl('/pulls'),
                     marshallPullRequests);
}

/**
 * Execute API requests for single pull request query.
 *
 * @param  {function} onFulfilled query result will be passed to this callback
 * @param  {function} onRejected  errors will be passed to this callback
 * @param  {number}   id          the pull request id to get
 */
function getPullRequest(onFulfilled, onRejected, id) {
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
                       apiUrl('/issues/' + id + '/comments'),
                       marshallComments);
  }
  gitcore.apiRequest(commentsRequest,
                     onRejected,
                     apiUrl('/pulls/' + id),
                     marshallPullRequest);
}

/**
 * Execute the API requests for a code review on pull request file.
 *
 * @param  {function} onFulfilled query result will be passed to this callback
 * @param  {function} onRejected  errors will be passed to this callback
 * @param  {number}   id          the pull request id
 * @param  {mixed}    file        the file to review, either the string path
 *                                of the file or the numerical index in the
 *                                file list
 */
function getPullReqReview(onFulfilled, onRejected, id, file) {
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
                       apiUrl('/pulls/' + id + '/comments'),
                       marshallReviewCommentsWrapper);
  }
  var diffRequest = function(data) {
    var diffUrl = data.diff_url;
    delete data.diff_url;
    var diffFulfilled = function(diffData) {
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
                     apiUrl('/pulls/' + id),
                     marshallPullRequest);
}

/**
 * Marshall the result of the pull requests API call into standard structure.
 *
 * @param  {object} buffer the result of the API call
 * @return {object}        an object containing information about the pull
 *                         requests
 */
function marshallPullRequests(buffer) {
  var data = JSON.parse(buffer);
  var out = data.map(function(x) {
    return {
              "id": x.number,
           "state": x.state,
           "title": x.title,
     "description": x.body,
      "created_at": marshallTimestamp(x.created_at),
      "updated_at": marshallTimestamp(x.updated_at),
       "closed_at": marshallTimestamp(x.closed_at),
          "author": x.user.login,
    }
  });
  return out;
}

/**
 * Marshall the result of the single pull request API call into standard
 * structure.
 *
 * @param  {object} buffer the result of the API call
 * @return {object}        an object containing information about the pull
 *                         request
 */
function marshallPullRequest(buffer) {
  var data = JSON.parse(buffer);
  var out = {
               "id": data.number,
            "state": data.state,
            "title": data.title,
      "description": data.body,
       "created_at": marshallTimestamp(data.created_at),
       "updated_at": marshallTimestamp(data.updated_at),
        "closed_at": marshallTimestamp(data.closed_at),
        "merged_at": marshallTimestamp(data.merged_at),
           "author": data.user.login,
           "merged": data.merged,
          "commits": data.commits,
        "additions": data.additions,
        "deletions": data.deletions,
    "changed_files": data.changed_files,
         "diff_url": data.diff_url,
  };
  return out;
}

/**
 * Marshall the result of the pull request comments API call into standard
 * structure.
 *
 * @param  {object} buffer the result of the API call
 * @return {array}         an array of objects containing information about
 *                         each comment
 */
function marshallComments(buffer) {
  var data = JSON.parse(buffer);
  var out = data.map(function(x) {
    return {
            "user": x.user.login,
         "message": x.body,
      "created_at": marshallTimestamp(x.created_at),
      "updated_at": marshallTimestamp(x.updated_at),
    }
  });
  return out;
}

/**
 * Marshall the result of the pull request diff API call into standard
 * structure.
 *
 * @param  {object} buffer the result of the API call
 * @return {array}         an array of filenames affected by the pull request
 */
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

/**
 * Marshall the result of a unified diff into single diff for specified file.
 *
 * @param  {object} buffer the result of the API call
 * @param  {string} path   the path of the file to extract from diff
 * @return {object}        an object containing the file's diff
 */
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

/**
 * Marshall the result of a review comments API call into standard structure.
 *
 * @param  {object} buffer the result of the API call
 * @param  {string} path   the path of the file for which to get review
 *                         comments
 * @return {object}        an object with diff position (line number) as
 *                         properties and comment info objects as values
 */
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
           "message": comment.body,
        "created_at": marshallTimestamp(comment.created_at),
        "updated_at": marshallTimestamp(comment.updated_at),
      });
    }
  });
  return out;
}

/**
 * Marshall a timestamp back into unixtime.
 *
 * @param  {string} timestamp a timestamp from API call
 * @return {number}           the unixtime of that timestamp
 */
function marshallTimestamp(timestamp) {
  return Date.parse(timestamp) / 1000;
}

