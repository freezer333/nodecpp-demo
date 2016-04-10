"use strict"; 

const cppemit = require("./cppemit");

var emitter = cppemit();


/* Todo 

    - Need to be able to have client (this code) close emitter
    - and have that shut down the C++ thread.

    - Need to handle errors and close events

    - Create a stream from the emitter, using substack's stuff.
*/

emitter.on('even_event', function(value){
    console.log("EVEN:  " + value);
});

emitter.on('odd_event', function(value){
    console.log("ODD:   " + value);
});

emitter.on('close', function(){
    console.log("Emitter has closed");
})