"use strict"; 

const worker = require("streaming-worker");
const path = require("path");

var addon_path = path.join(__dirname, "build/Release/factorization");

const factorizer = worker(addon_path, {n: 9007199254740991});

factorizer.from.on('factor', function(factor){
    console.log("Factor:  " + factor);
});

factorizer.from.on('error', function(e) {
    console.log(e);
});

