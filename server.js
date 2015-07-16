var http = require('http');
var fs = require('fs');
var url = require('url');
String.prototype.endsWith = function(suffix) {
    return this.indexOf(suffix, this.length - suffix.length) !== -1;
};

http.createServer(function (req, res) {
    var path = '/home/bobby/work/gabbo-client/' + url.parse(req.url).pathname;
    if (path.endsWith('/')) {
        path += 'index.html';
    }
    if (fs.existsSync(path)) {
        var content = fs.readFileSync(path);
        var contentType = 'text/plain';
        if (path.endsWith(".html")) {
          contentType = 'text/html';
        } else if (path.endsWith(".css")) {
            contentType = 'text/css';
        } else if (path.endsWith(".js")) {
            contentType = 'text/javascript';
        }
        res.writeHead(200, {'Content-Type': contentType});
        res.end(content);
    } else {
        res.writeHead(404);
    }
}).listen(2080);
