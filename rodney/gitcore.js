var url = require('url');
var request = require('request');

exports.apiRequest = apiRequest;
exports.setupCache = setupCache;
exports.teardownCache = teardownCache;

var apiCache;

function setupCache() {
  apiCache = { };
}

function teardownCache() {
  apiCache = null;
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
