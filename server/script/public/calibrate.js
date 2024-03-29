var control_blob = {};
control_blob['speed'] = 0;

let ws = new WebSocket('ws://192.168.50.115:8000');
var b_pause = false;
var b_begin = false;

var cal_index = 0;
var speed_level = []
function toggle()
{
    elem = document.getElementById("button-pause");
    if (elem.innerHTML  == 'Pause')
    {
        b_pause = true;
        elem.innerHTML  = 'Unpause';
    }
    else
    {
        b_pause = false;
        elem.innerHTML  = 'Pause';
    }
}

function begin()
{
    var buffer = new ArrayBuffer(0);
    var i, j;
    var speed_max = 1.0;
    var speed_off = 0.0;
    var speeds = [0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.7, 1.0]
    for (i=0; i<speeds.length; i++)
    {
        for (j=0; j<speeds.length; j++)
        {
            speed_level.push(speeds[i]);
            speed_level.push(speeds[j]);
            speed_level.push(speed_off);
            speed_level.push(-speeds[i]);
            speed_level.push(-speeds[j]);
            speed_level.push(speed_off);

            speed_level.push(speeds[i]);
            speed_level.push(-speeds[j]);
            speed_level.push(speed_off);
            speed_level.push(-speeds[i]);
            speed_level.push(speeds[j]);
            speed_level.push(speed_off);
            
        }
    }
    cal_index = 0;
}

function progressUpdate() {
    var elem = document.getElementById("myBar");
    var width;
    if (speed_level.length == 0)
    {
        width = 0;
    }
    else
    {
        width = cal_index / speed_level.length * 100;
    }
    elem.style.width = width + "%";
}


function calibrate()
{
    if (cal_index < speed_level.length)
    {
        if (!b_pause)
        { 
            buffer = new ArrayBuffer(0);
            control_blob['speed'] = new Float32Array([speed_level[cal_index]]);
            buffer = blobEncode(control_blob, 'main', buffer);
            ws.send(buffer);
            cal_index++;
            progressUpdate();
        }
    }    
}

setInterval(calibrate, 500);
