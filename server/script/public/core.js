let ws = new WebSocket('ws://192.168.50.115:8000');
ws.binaryType = 'arraybuffer';
new_plot_id = 0;


function addTraceToPlot()
{
    /* Add a trace to a given plot when this button is pressed. Unfortunately cannot make
       this a method of class Plot, because there is a conflict with the *this* keyword (used
       by both the button and the object instance */
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
    constructor(plot_type, plot_id, trace_options)
    {
        this.plot_type = plot_type;
        this.plot_id = plot_id;
        this.n_traces = 0;
        this.traces = [];

        this.max_plot_len = 1000;
        this.plot_data = [];
        this.max_heatmap_len = 100;

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

        for (const trace of trace_options)
        {
            var option = document.createElement("option");
            option.class = "dropdown-option";
            option.value = trace.split(" ");
            option.text = trace;
            this.plot_selection.appendChild(option);
        }

        this.add_trace = document.createElement("button");
        this.add_trace.class = "add-trace";
        this.add_trace.type = 'button';
        this.add_trace.innerHTML = 'Add trace';
        this.add_trace.id = "button-" + plot_id;
        this.add_trace.onclick = addTraceToPlot;

        this.element = document.getElementById("plots");
        
        this.plot_div.appendChild(this.plot_section);
        this.plot_div.appendChild(this.plot_selection);
        this.plot_div.appendChild(this.add_trace);
        this.element.appendChild(this.plot_div);
        
        /* Update this for each datapoitn added to the plot, before the plot operation is called */
        this.data_added_since_plot = 0;
        this.update_count_thresh = 1;
        this.plot_len = 100;
        this.indices = [];
        Plotly.newPlot('plot-' + this.plot_id, this.plotdata, this.plotlayout);
    }

    setUpdateCountThresh(update_count_thresh)
    {
        this.update_count_thresh = update_count_thresh;
    }

    addTrace()
    {
        var trace;
        var dropdown = document.getElementById("dropdown-" + id);
        Plotly.addTraces('plot-' + this.plot_id, {y: [], name: dropdown.value});
        
        this.traces.push(dropdown.value);

        this.plot_data = [];
        for (trace of this.traces)
        {
            this.plot_data.push([]);
            this.indices.push(this.n_traces);
        }
        this.n_traces = this.n_traces + 1;
    }

    addData(in_data)
    {
        
        var index = 0;
        var trace;
        var trace_idx = 0;
        for (trace of this.traces)
        {
            var data;
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
            
            index = index + 1;
            trace_idx = trace_idx + 1;
        }
        this.data_added_since_plot = this.data_added_since_plot + 1;
        if (this.data_added_since_plot >= this.update_count_thresh)
        {
            this.plot()
        }
    }

    plot()
    {
        var indices = [];
        var trace;
        var i = 0;
        for (trace of this.plot_data)
        {
            indices.push(i);
            i = i + 1;
        }
        if (this.plot_data.length > 0)
        {
            if (this.plot_type == 'heatmap')
            {
                Plotly.restyle('plot-' + this.plot_id, {z: [...this.plot_data]}, indices)

                if (this.plot_data.length >= this.plot_len)
                {
                    this.plot_data = this.plot_data.slice(1);
                }
            }
            else
            {
                if (this.plot_data[0].length >= this.plot_len)
                {
                    var i = 0;
                    let trace;
                    for (trace of this.traces)
                    {
                        this.plot_data[i] = this.plot_data[i].slice(this.plot_data[i].length - this.plot_len);
                        i += 1;
                    } 
                    
                }
                Plotly.restyle('plot-' + this.plot_id, {y: this.plot_data}, indices);
            }
        }
        this.data_added_since_plot = 0;
    }
}

function addPlot()
{
    var plot_type = document.getElementById("plot-type-select");
    var ip_addr_dropdown = document.getElementById("plot-ip-address");
    
    if (!(ip_addr_dropdown.value))
    {
        console.log("No data is streaming yet");
        return;
    }
    a_plots[new_plot_id] = new Plot(plot_type.value, new_plot_id, ip_options[ip_addr_dropdown.value]);
    a_plot_device_ip[new_plot_id] = ip_addr_dropdown.value;
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
var a_plots = [];
var a_plot_device_ip = [];
var ip_options = {};
// message received - show the message in div#messages
ws.onmessage = function(event) {
    let dv = new DataView(event.data);
    var ip_addr;
    var trace_options;
    
    [buffer, ip_addr] = readString(dv.buffer);
    [buffer, out, nodename] = blobDecode(buffer);
    root = {};
    root[nodename] = out;

    if (!(ip_addr in ip_options))
    {
        /* Add the IP address option to the global dropdown */
        /* For now, multiple devices from the same IP are not supported */
        var ip_dropdown = document.getElementById("plot-ip-address");
        var option = document.createElement("option");
        option.class = "dropdown-option";
        option.value = ip_addr;
        option.text = ip_addr;
        ip_dropdown.appendChild(option);

        trace_options = propertiesToArray(out);
        ip_options[ip_addr] = trace_options;        
    }
    
    for (plot_idx=0; plot_idx<a_plots.length; plot_idx++)
    {
        /* only add data to the plot if the IP address matches */
        if (a_plot_device_ip[plot_idx] == ip_addr)
        {
            a_plots[plot_idx].addData(out);
        }
    }
};

ws.onopen = function() {
    console.log('WebSocket Client is has connected foo bah.');
};





