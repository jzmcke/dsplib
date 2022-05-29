let ws = new WebSocket('ws://192.168.50.115:8000');
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

function addTrace()
{  
    id = this.id.split('-')[1]
    var dropdown = document.getElementById("dropdown-" + id);
    
    Plotly.addTraces('plot-' + id, {y: [], name: dropdown.value});
    n_traces_per_plot[id] = n_traces_per_plot[id] + 1;
    trace_vars_per_plot[id].push(dropdown.value); 
    
};

function addTraceToPlot()
{
    id = this.id.split('-')[1]

    var i = 0;

    for (i=0; i<a_plots.length; i++)
    {
        if (a_plots[i].plot_id == id)
        {
            a_plots[i].addTrace();
        }
    }
    
}

class Plot
{
    constructor(plot_type, plot_id)
    {
        this.plot_type = plot_type;
        this.plot_id = plot_id;
        this.n_traces = 0;
        this.traces = [];

        this.max_plot_len = 1000;
        this.x_pos = 0;
        this.plot_data = [];
        this.max_heatmap_len = 100;

        this.last_data_point = 0;

        if (valid_logs.length == 0)
        {
            console.log("No data is streaming yet");
            return;
        }

        this.plotdata = [];
        
        this.plotlayout = {
            title: plot_type + ' ' + plot_id,
            // paper_bgcolor: '#3b3838',
            // plot_bgcolor: '#3b3838'
        };

        this.plot_div = document.createElement("div");
        this.plot_div.id = "plot-div-" + plot_id;
        this.plot_div.class = "plot-container";

        this.plot_section = document.createElement("div");
        this.plot_section.setAttribute("id", "plot-" + plot_id);

        this.plot_selection = document.createElement("select");
        this.plot_selection.id = "dropdown-" + plot_id;
        this.plot_selection.class = "dropdown-traces";

        for (const log of valid_logs)
        {
            var option = document.createElement("option");
            option.class = "dropdown-option";
            option.value = log.split(" ");
            option.text = log;
            this.plot_selection.appendChild(option);
        }

        this.add_trace = document.createElement("button");
        this.add_trace.class = "add-trace";
        this.add_trace.type = 'button';
        this.add_trace.innerHTML = 'Add trace'
        this.add_trace.id = "button-" + plot_id;
        this.add_trace.onclick = addTraceToPlot;

        this.element = document.getElementById("plots");
        
        this.plot_div.appendChild(this.plot_section);
        this.plot_div.appendChild(this.plot_selection);
        this.plot_div.appendChild(this.add_trace);
        this.element.appendChild(this.plot_div);
        
        Plotly.newPlot('plot-' + this.plot_id, this.plotdata, this.plotlayout);
    }
    
    addTrace()
    {
        var trace;
        var dropdown = document.getElementById("dropdown-" + id);
        Plotly.addTraces('plot-' + this.plot_id, {y: [], name: dropdown.value});
        this.n_traces = this.n_traces + 1;
        this.traces.push(dropdown.value);

        this.plot_data = [];
        for (trace of this.traces)
        {
            this.plot_data.push([]);
        }
    }

    addData(in_data)
    {
        var data_to_plot;
        var indices = [];
        var index = 0;
        var trace;
        var trace_idx = 0;
        for (trace of this.traces)
        {
            var data;
            var these_vars = [];
            var scopes = trace.split('.');
            var this_var = in_data[scopes[0]];
            for (scope of scopes.slice(1))
            {
                this_var = this_var[scope];
            }
            if (this.plot_type == 'scatter')
            {
                data = this_var[0][0];
            }
            else if (this.plot_type == 'heatmap')
            {
                data = this_var[0];
            }
            this.plot_data[trace_idx].push(data);
            indices.push(index);
            index = index + 1;
            trace_idx = trace_idx + 1;
        }
        if (this.plot_data.length > 0)
        {
            data_to_plot = data;
            if (this.plot_type == 'heatmap')
            {
                // data_to_add = Array.from(data_to_plot[0][0])
                // this.plot_data.push(data_to_add);
                // data_to_plot = math.reshape(this.plot_data, [data.length, this.plot_data.length])
                Plotly.restyle('plot-' + this.plot_id, {z: [...this.plot_data]}, indices)

                if (this.plot_data.length >= 100)
                {
                    this.plot_data = this.plot_data.slice(1);
                }
            }
            else
            {
                // data_to_add = Array.from()
                // this.plot_data.push(data_to_plot)
                // var to_plot = math.reshape(this.plot_data, [this.n_traces, this.plot_data.length])
                Plotly.restyle('plot-' + this.plot_id, {y: this.plot_data}, indices);
                if (this.plot_data[0].length >= 100)
                {
                    var i = 0;
                    for (trace of this.traces)
                    {
                        this.plot_data[i] = this.plot_data[i].slice(1);
                        i += 1;
                    } 
                    
                }
            }
        }
        else
        {
            data_to_plot = [this.last_data_point];
            indices = [0];
        }
       
        this.last_data_point = data_to_plot[data_to_plot.length-1];
        this.x_pos = this.x_pos + 1 % this.max_plot_len;
    }
}

var new_plot_id = 0;

function addPlot()
{
    var plot_type = document.getElementById("plot-type-select");
    a_plots[new_plot_id] = new Plot(plot_type.value, new_plot_id);
    valid_plot_ids.push(new_plot_id);
    new_plot_id = new_plot_id + 1;
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


var b_discovered = false;
var valid_logs = [];
var a_plots = [];
var valid_plot_ids = [];
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
    
    for (plot_idx=0; plot_idx<a_plots.length; plot_idx++)
    {
        a_plots[plot_idx].addData(out);
    }
};

ws.onopen = function() {
    console.log('WebSocket Client is has connected foo bah.');
};





