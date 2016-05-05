"use strict"; 

const worker = require("streaming-worker");
const through = require('through');
const path = require("path");

var addon_path = path.join(__dirname, "build/Release/factorization");

const factorizer = worker(addon_path, {n: 9007199254740991});

factorizer.fromAddon.on('factor', function(factor){
    console.log("Factor:  " + factor);
});

factorizer.fromAddon.on('error', function(e) {
    console.log(e);
});

