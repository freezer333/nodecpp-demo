const worker = require("streaming-worker");
const path = require("path");


var acc_path = path.join(__dirname, "../accumulate/build/Release/accumulate");
var eo_path = path.join(__dirname, "../even_odd/build/Release/even_odd_worker");

// TODO: This example doesn't seem to work on OS X - there is a hangup whenever two addons are used...
//       Works on Linux and Windows
//       Pull requests welcome :)
const eacc = worker(acc_path, {filter:"even_event"});
const oacc = worker(acc_path, {filter:"odd_event"});
const eo = worker(eo_path, {start:1});

[eacc, oacc].forEach(function(acc) {
	const input = acc.to.stream("value",
		function () {
			acc.to.emit('value', -1);
		});
	
	eo.from.stream().pipe(input);
});

eacc.from.on('sum', function(value){
    console.log("Accumulated Sum of Evens:  " + value);
});
oacc.from.on('sum', function(value){
    console.log("Accumulated Sum of Odds:  " + value);
});

eo.to.emit("go", 10);
eo.to.emit("go", -1);
eo.close();
