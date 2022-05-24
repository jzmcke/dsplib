let ws = new WebSocket('ws://172.21.143.247:8000');
ws.binaryType = 'arraybuffer';
var data = [];
var last_data_point = 0;
var dec = new TextDecoder("utf-8");

function readInt32(buffer)
{
    dv = new DataView(buffer);
    val = dv.getInt32(0, true);
    buffer = buffer.slice(4);
    return [buffer, val];
}

function readString(buffer)
{
    val = String.fromCharCode.apply(null, new Uint8Array(buffer.slice(0,128))).replace(/\0.*$/g,'');
    buffer = buffer.slice(128);
    return [buffer, val];
}

function blobDecode(buffer)
{
    var out = {};
    var nodename;
    var n_children;
    var b_has_blob;
    var n_variables;
    var varnames, varname;
    var types, type;
    var lens, len;

    [buffer, nodename] = readString(buffer);

    [buffer, n_children] = readInt32(buffer);

    [buffer, b_has_blob] = readInt32(buffer);

    if (b_has_blob == 1) {
        [buffer, n_repetitions] = readInt32(buffer);
        [buffer, n_variables] = readInt32(buffer);

        varnames = []
        types = []
        lens = []
        for (i=0; i<n_variables; i++)
        {
            [buffer, varname] = readString(buffer);
            out[varname] = []
            varnames.push(varname);
        }
        for (i=0; i<n_variables; i++)
        {
            [buffer, type] = readInt32(buffer);
            types.push(type);
        }
        for (i=0; i<n_variables; i++)
        {
            [buffer, len] = readInt32(buffer);
            lens.push(len);
        }
        
        for (rep=0; rep<n_repetitions+1; rep++)
        {
            for (i=0; i<n_variables; i++)
            {
                if (types[i] == 0)
                {
                    /* int */
                    val = new Int32Array(buffer.slice(0, lens[i] * 4));
                    buffer = buffer.slice(lens[i] * 4);
                } else if (types[i] == 1)
                {
                    /* float */
                    val = new Float32Array(buffer.slice(0, lens[i] * 4));
                    buffer = buffer.slice(lens[i] * 4);
                } else if (types[i] == 2)
                {
                    /* unsigned int */
                    val = new Uint32Array(buffer.slice(0, lens[i] * 4));
                    buffer = buffer.slice(lens[i] * 4);
                }

                out[varnames[i]].push(val)
            }
        }
    }
    for (i=0; i<n_children; i++)
    {
        [buffer, childout, childname] = blobDecode(buffer);
        out[childname] = childout;
    }
    
    return [buffer, out, nodename];
}
var trace_vars_per_plot = [];
var n_traces_per_plot = [];

function addTrace()
{  
    id = this.id.split('-')[1]
    var dropdown = document.getElementById("dropdown-" + id);

    
    Plotly.addTraces('plot-' + id, {y: [], name: dropdown.value});
    n_traces_per_plot[id] = n_traces_per_plot[id] + 1;
    trace_vars_per_plot[id].push(dropdown.value); 
    
};

var plot_id = 0;
var valid_plot_ids = [];
var valid_logs = [];


function addPlot()
{
    if (valid_logs.length == 0)
    {
        console.log("No data is streaming yet");
        return;
    }
    var trace1 = {y: [],
                mode: 'lines',
                type: 'scatter',
                name: 'Broadcasted data',
                marker: { size: 12 }
                };

    var plotdata = [ trace1 ];
    
    var plotlayout = {
        title:'Data Labels Hover'
    };

    var new_plot_div = document.createElement("div");
    new_plot_div.setAttribute("id", "plot-div-" + plot_id);

    var plot_section = document.createElement("div");
    plot_section.setAttribute("id", "plot-" + plot_id);

    var plot_selection = document.createElement("select");
    plot_selection.id = "dropdown-" + plot_id;

    for (const log of valid_logs)
    {
        var option = document.createElement("option");
        option.value = log.split(" ");
        option.text = log;
        plot_selection.appendChild(option);
    }

    var add_trace = document.createElement("button");
    add_trace.type ='button';
    add_trace.innerHTML = 'Add trace'
    add_trace.id = "button-" + plot_id;
    add_trace.onclick = addTrace;

    var element = document.getElementById("plots");
    
    new_plot_div.appendChild(plot_section);
    new_plot_div.appendChild(plot_selection);
    new_plot_div.appendChild(add_trace);
    element.appendChild(new_plot_div);
    
    Plotly.newPlot('plot-' + plot_id, plotdata, plotlayout);
    valid_plot_ids.push(plot_id);
    plot_id = plot_id + 1;
    trace_vars_per_plot.push([]);
    n_traces_per_plot.push(0);
}

function propertiesToArray(obj) {
    const isObject = val =>
      val && typeof val === 'object' && !Array.isArray(val);
  
    const addDelimiter = (a, b) =>
      a ? `${a}.${b}` : b;
  
    const paths = (obj = {}, head = '') => {
      return Object.entries(obj)
        .reduce((product, [key, value]) => 
          {
            let fullPath = addDelimiter(head, key)
            return isObject(value) ?
              product.concat(paths(value, fullPath))
            : product.concat(fullPath)
          }, []);
    }
  
    return paths(obj);
}
var max_plot_len = 1000;
var x_pos = 0;
var b_discovered = false;
// message received - show the message in div#messages
ws.onmessage = function(event) {
    let dv = new DataView(event.data);
    [buffer, out, nodename] = blobDecode(dv.buffer);
    root = {};
    root[nodename] = out;
    
    if (!b_discovered)
    {
        valid_logs = propertiesToArray(out);
        b_discovered = true;
    }
    
    for (plot_id=0; plot_id<valid_plot_ids.length; plot_id++)
    {
        var data = [];
        var data_x = [];
        var indices = [];
        var traces = trace_vars_per_plot[plot_id];
        var index = 0;
        for (trace of traces)
        {
            var scopes = trace.split('.');
            var this_var = out[scopes[0]];
            for (scope of scopes.slice(1))
            {
                this_var = this_var[scope];
            }
            data.push([this_var[0][0]])
            data_x.push([x_pos]);
            indices.push(index);
            index = index + 1;
        }
        if (data.length > 0)
        {
            data_to_plot = data;
            Plotly.extendTraces('plot-' + plot_id, {y: data_to_plot}, indices, max_plot_len);
        }
        else
        {
            data_to_plot = [last_data_point];
            indices = [0];
        }
       
        last_data_point = data_to_plot[data_to_plot.length-1];
        data = [];
    }
    x_pos = x_pos + 1 % max_plot_len;
};

ws.onopen = function() {
    console.log('WebSocket Client is has connected foo bah.');
};





