var net = require('net');
var TelnetInput = require('telnet-stream').TelnetInput;
var TelnetOutput = require('telnet-stream').TelnetOutput;

var socket = net.createConnection(2010, 'eotl.org', function() {
    var telnetInput = new TelnetInput();
    var telnetOutput = new TelnetOutput();

    socket.pipe(telnetInput).pipe(process.stdout);
    process.stdin.pipe(telnetOutput).pipe(socket);
});
