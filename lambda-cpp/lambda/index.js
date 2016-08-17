exports.averageHandler = function(event, context, callback) {
   const addon = require('average');
   console.log(event);
   var result = addon.average(event.op1, event.op2, event.op3)
   console.log(result);
   callback(null, result);
}