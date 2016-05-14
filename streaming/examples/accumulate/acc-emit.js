"use strict"; 

const worker = require("streaming-worker");
const path = require("path");

var addon_path = path.join(__dirname, "build/Release/accumulate");
const acc = worker(addon_path);

acc.to.emit("value", 3);
acc.to.emit("value", 16);
acc.to.emit("value", 42);
acc.to.emit("value", -1);

acc.from.on('sum', function(value){
    console.log("Accumulated Sum:  " + value);
});