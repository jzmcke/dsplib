
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
