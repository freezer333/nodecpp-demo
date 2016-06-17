var say = require('hello-world-nodecpp');
var nansay = require('hello-world-nan-nodecpp');
var addlib = require('addlib');
console.log( say.hello() );
console.log( nansay.hello() );


var sum = addlib.add(5, 2);
console.log(sum);