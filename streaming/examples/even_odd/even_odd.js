"use strict"; 

const worker = require("streaming-worker");
const through = require('through');
const path = require("path");

var addon_path = path.join(__dirname, "build/Release/even_odd_worker");

const eo = worker(addon_path);

eo.to.emit("go", 10);

setTimeout(function() {
    eo.to.emit("start", 20);
    eo.to.emit("start", -1);
}, 5000);

eo.from.on('even_event', function(value){
    console.log("EVEN:  " + value);
});

eo.from.on('odd_event', function(value){
    console.log("ODD:   " + value);
});