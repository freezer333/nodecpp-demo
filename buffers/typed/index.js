'use strict';
const addon = require('./build/Release/typed_example');

const data = new Float64Array([1.2, 2.3, 3.4]);

// synchronous, increment each float by +100
var result = addon.increment(data, 100);

console.log(data);
