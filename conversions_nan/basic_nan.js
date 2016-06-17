const addon = require('./build/Release/basic_nan');


console.log( addon.pass_number(5) );
console.log( addon.pass_number() );
console.log( addon.pass_number(function() { return "xyz"}));


console.log( addon.pass_integer(4));
console.log( addon.pass_integer());

console.log( addon.pass_boolean(false));

console.log( addon.pass_string("helle"));


var retval = addon.pass_object({x : 3, y: 10});
console.log(retval);

retval = addon.pass_object({});
console.log(retval);
retval = addon.pass_object();
console.log(retval);