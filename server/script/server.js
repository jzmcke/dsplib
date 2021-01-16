var http = require('http');
var url = require('url');
var fs = require('fs').promises;
const ws = new require('ws');
var port = 8000;
var host = 'localhost';
let indexFile;

let data = [];


const wss = new ws.Server('ws://localhost:8080', {port: 8080});


wss.on('message', function(message) {
    message = message.slice(0, 50); // max message length will be 50
    data.push(message);
    console.log(message);
});


const requestListener = function(req, res) {
    // here we only handle websocket connections
    // in real project we'd have some other code here to handle non-websocket requests
    console.log(req.headers);
    res.setHeader("Content-Type", "text-html");
    res.writeHead(200);
    res.end(indexFile);
}

const server = http.createServer(requestListener);

fs.readFile(__dirname + "/index.html")
    .then(contents => {
        indexFile = contents;
        server.listen(port, host, () => {
            console.log(`Server is running on http://${host}:${port}`);
        });
    })
    .catch(err => {
        console.error(`Could not read index.html file: ${err}`);
        process.exit(1);
    });


var j = 2;