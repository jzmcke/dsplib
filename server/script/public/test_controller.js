var speed_slider = document.getElementById("speed-range");
var speed_display = document.getElementById("speed-display");

let ws = new WebSocket('ws://192.168.50.115:8000');

speed_display.innerHTML = speed_slider.value; // Display the default slider value
var speed, angle;

// Update the current slider value (each time you drag the slider handle)
speed_slider.oninput = function() {
    speed_display.innerHTML = this.value / 100;
    speed = this.value / 100;
}

var angle_slider = document.getElementById("angle-range");
var angle_display = document.getElementById("angle-display");
angle_display.innerHTML = angle_slider.value; // Display the default slider value

// Update the current slider value (each time you drag the slider handle)
angle_slider.oninput = function() {
    angle_display.innerHTML = this.value / 100;
    angle = this.value / 100;
}

var kick_input = document.getElementById("kick-timeout-ms");
var kick_display = document.getElementById("kick-display");
var kick_timeout_ms = 0;
kick_input.oninput = function() {
    kick_display.innerHTML = this.value;
    kick_timeout_ms = this.value;
}

var deadzone_speed_input = document.getElementById("deadzone-speed");
var deadzone_speed_display = document.getElementById("deadzone-speed-display");
var deadzone_speed = 0;
deadzone_speed_input.oninput = function() {
    deadzone_speed_display.innerHTML = this.value;
    deadzone_speed = this.value;
}


var deadzone_input = document.getElementById("deadzone-compensate");
var deadzone_display = document.getElementById("deadzone-display");
var b_deadzone_compensate = 0;
deadzone_input.oninput = function() {
    deadzone_display.innerHTML = this.checked;
    b_deadzone_compensate = this.checked;
}


var control_blob = {};
function command()
{

    buffer = new ArrayBuffer(0);
    control_blob['speed'] = new Float32Array([speed]);
    control_blob['angle'] = new Float32Array([angle]);
    control_blob['b_deadzone_comp'] = new Int32Array([b_deadzone_compensate]);
    control_blob['kick_timeout_ms'] = new Uint32Array([kick_timeout_ms]);
    control_blob['deadzone_speed'] = new Float32Array([deadzone_speed]);
    buffer = blobEncode(control_blob, 'main', buffer);
    ws.send(buffer);
}

setInterval(command, 100);