var gitcore = require('./gitcore.js');

exports.query = query;

function apiUrl(path) {
  return "https://api.github.com/repos/bobalu113/gabbo" + path;
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
  gitcore.setupCache();
  if (get) {
    get();
  }
  else {
    onRejectedWrapper("Unsupported operation: " + query);
  }
}
