// hello.js
const addon = require('./build/Release/hello_addon');

console.log(addon.hello()); // 'world'