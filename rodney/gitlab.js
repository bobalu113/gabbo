/**
 * The GitLab adapter for issue tracking and pull requests.
 *
 * @module gitlab
 * @requires module:gitcore
 */

var gitcore = require('./gitcore.js');

exports.query = query;

/**
 * Return the GitHub API url for the specified path.
 *
 * @param  {string} path the path of the API request
 * @return {string}      the full URL for API request
 */
function apiUrl(path) {
  return "https://api.gitlab.com/repos/bobalu/eotl" + path;
}

/**
 * Query the GitLab API.
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

  return runQuery;
}
