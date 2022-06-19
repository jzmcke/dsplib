var control_blob = {};
control_blob['pwm_level'] = 0;

let ws = new WebSocket('ws://192.168.50.115:8000');
var b_pause = false;
var b_begin = false;

var cal_index = 0;
var pwm_level = []
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
    pwm_max = 4096;
    levels = 20;
    for (i=0; i<pwm_max; i += (pwm_max / levels))
    {
        for (j=0; j<pwm_max; j += (pwm_max / levels))
        {
            pwm_level.push(4096 + i);
            pwm_level.push(4096 + j);
            pwm_level.push(4096);
            pwm_level.push(4096 - i);
            pwm_level.push(4096 - j);
            pwm_level.push(4096);
            pwm_level.push(4096 + i);
            pwm_level.push(4096 - j);
            pwm_level.push(4096);
            pwm_level.push(4096 - i);
            pwm_level.push(4096 + j);
            pwm_level.push(4096);
        }
    }
    cal_index = 0;
}

function progressUpdate() {
    var elem = document.getElementById("myBar");
    var width;
    if (pwm_level.length == 0)
    {
        width = 0;
    }
    else
    {
        width = cal_index / pwm_level.length * 100;
    }
    elem.style.width = width + "%";
}


function calibrate()
{
    if (cal_index < pwm_level.length)
    {
        if (!b_pause)
        {            
            buffer = new ArrayBuffer(0);
            control_blob['pwm_level'] = new Int32Array([pwm_level[cal_index]]);
            buffer = blobEncode(control_blob, 'main', buffer);
            ws.send(buffer);
            cal_index++;
            progressUpdate();
        }
    }    
}

setInterval(calibrate, 500);
