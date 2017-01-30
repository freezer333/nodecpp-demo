var rt = require('native_rt');

var start = rt.now();
setTimeout(function() {
    let end = rt.now();
    console.log(end - start);
}, 1000)