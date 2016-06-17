// hello.js
const addon = require('./build/Release/who_addon');

var me = {
    hello : addon.hello
}

var who = me.hello();
console.log(who);
