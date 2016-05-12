"use strict"; 

const worker = require("streaming-worker");
const path = require("path");
const streamify = require('stream-array');

var addon_path = path.join(__dirname, "build/Release/accumulate");
const acc = worker(addon_path);

const input = acc.to.stream("value",
	function () {
		acc.to.emit('value', -1);
	});

streamify([1, 2, 3, 4, 5, 6]).pipe(input);

acc.from.on('sum', function(value){
    console.log("Accumulated Sum:  " + value);
});