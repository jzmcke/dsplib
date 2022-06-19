var test = {}

test['floats'] = new Float32Array([1.0, 2.5, 6.9874]);
test['ints'] = new Int32Array([-3, 4, 102956, -1238974]);
test['uints'] = new Uint32Array([1, 7, 14, 19238]);
test['child_one'] = {'one_deep': new Int32Array([13]), 'two_deep': {'two_deep_var': new Float32Array([0.333333])}};
test['child_two'] = {'end': new Uint32Array([0])};

var buffer = new ArrayBuffer();
var out;
var node_name;
var foo;
buffer = blobEncode(test, 'main', buffer);

foo, out, node_name = blobDecode(buffer);

var foo = 1;