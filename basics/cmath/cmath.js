const cmath = require('./build/Release/cmath');
const five = cmath.add(2, 3);
console.log( '2 + 3 is ', five );

var u = cmath.add(1)

console.log(u);
console.log(u===undefined);