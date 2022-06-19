


var control_blob = {};
control_blob['forward'] = new Int32Array([0]);
control_blob['backward'] = new Int32Array([0]);
control_blob['left'] = new Int32Array([0]);
control_blob['right'] = new Int32Array([0]);
control_blob['stop'] = new Int32Array([0]);

let ws = new WebSocket('ws://192.168.50.115:8000');

function goForward()
{
    var buffer = new ArrayBuffer(0);
    control_blob['forward'] = new Int32Array([1]);
    control_blob['backward'] = new Int32Array([0]);
    control_blob['left'] = new Int32Array([0]);
    control_blob['right'] = new Int32Array([0]);
    control_blob['stop'] = new Int32Array([0]);

    buffer = blobEncode(control_blob, 'main', buffer);
    ws.send(buffer);
}

function goBackward()
{
    var buffer = new ArrayBuffer(0);
    control_blob['forward'] = new Int32Array([0]);
    control_blob['backward'] = new Int32Array([1]);
    control_blob['left'] = new Int32Array([0]);
    control_blob['right'] = new Int32Array([0]);
    control_blob['stop'] = new Int32Array([0]);

    buffer = blobEncode(control_blob, 'main', buffer);
    ws.send(buffer);
}

function goLeft()
{
    var buffer = new ArrayBuffer(0);
    control_blob['forward'] = new Int32Array([0]);
    control_blob['backward'] = new Int32Array([0]);
    control_blob['left'] = new Int32Array([1]);
    control_blob['right'] = new Int32Array([0]);
    control_blob['stop'] = new Int32Array([0]);

    buffer = blobEncode(control_blob, 'main', buffer);
    ws.send(buffer);
}

function goRight()
{
    var buffer = new ArrayBuffer(0);
    control_blob['forward'] = new Int32Array([0]);
    control_blob['backward'] = new Int32Array([0]);
    control_blob['left'] = new Int32Array([0]);
    control_blob['right'] = new Int32Array([1]);
    control_blob['stop'] = new Int32Array([0]);

    buffer = blobEncode(control_blob, 'main', buffer);
    ws.send(buffer);
}

function stop()
{
    var buffer = new ArrayBuffer(0);
    control_blob['forward'] = new Int32Array([0]);
    control_blob['backward'] = new Int32Array([0]);
    control_blob['left'] = new Int32Array([0]);
    control_blob['right'] = new Int32Array([0]);
    control_blob['stop'] = new Int32Array([1]);

    buffer = blobEncode(control_blob, 'main', buffer);
    ws.send(buffer);
}