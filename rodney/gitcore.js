/**
 * Core modules for managing git and git API requests.
 * @module gitcore
 * @requires  module:url
 * @requires  module:request
 */

var url = require('url');
var request = require('request');

/**
 * Initialize the gitcore module with optional username and password to be
 * used for for API calls.
 *
 * @constructor
 * @param  {string} username optional username to pass to API requests
 * @param  {string} password optional password to pass to API requests
 */
module.exports = function(username, password) {
  return {
    setupCache:    setupCache,
    teardownCache: teardownCache,
    apiRequest:    function(onFulfilled, onRejected, target, marshall) {
      apiRequest(onFulfilled, onRejected, username, password, target,
                 marshall);
    },
  };
};

var apiCache;

/**
 * (Re)initialize the cache of API call results.
 */
function setupCache() {
  apiCache = { };
}

/**
 * Free the cache of API call results.
 */
function teardownCache() {
  apiCache = null;
}

/**
 * Perform an API request to the specified target URL using the optional
 * username and password. Upon success, pass the result of the request to the
 * specified marshall function and pass the marshalled result to onFulfilled
 * callback. If any errors occur, onRejected will be called instead with a
 * informative error object
 *
 * @param  {function} onFulfilled  API response will be passed to this
 *                                 callback
 * @param  {function} onRejected   errors will be passed to this callback
 * @param  {string} username       optional username to pass to request
 * @param  {string} password       optional password to pass to request
 * @param  {string} target         target URL of API call
 * @param  {function} marshall     a function to marshall the result of the
 *                                 API call into a consumable object
 */
function apiRequest(onFulfilled, onRejected, username, password, target,
                    marshall) {
  if (target in apiCache) {
    onFulfilled(marshall(apiCache[target]));
  } else {
    var options = {
      url: url.parse(target),
      headers: {'User-agent': 'Rodney for Gabbo'},
    };
    if (username && password) {
      options.auth = {
                   'user': username,
               'password': password,
        'sendImmediately': true
      };
    }
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
