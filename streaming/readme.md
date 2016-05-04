# Purpose
`stream-to-node` is designed to give you a simple interface for receiving events/messages from a long running asynchronous C++ Node.js Addon.

# Usage
To use stream_to_node in your JavaScript code, you need to do an `npm install stream-to-node`.  The module exports a single factory method which accepts a path to an asynchronous C++ Node.js addon that is setup to use stream-to-node (see below).  The second parameter (optional) is for any parameters you wish to send to C++ to initialize the addon.

```js
const strm2node = require("stream-to-node");

var emitter = strm2node('./examples/primes/build/Release/primes', {max:100});
```

The factory method builds an EventEmitter.  You can use this to capture messages sent from the targeted C++ addon.

```js
emitter.on('prime', function(value){
    console.log("Prime Number:  " + value);
});

emitter.on('close', function(){
    console.log("Emitter has closed");
});

emitter.on('error', function(e) {
    console.log(e);
})
```

The `emitter` can be used just like any other EventEmitter - so you can turn it into a stream, etc.

```js
const strm2node = require("stream-to-node");
const emitStream = require('emit-stream');
const JSONStream = require('JSONStream');

var emitter = strm2node('./examples/primes/build/Release/primes', {max:100});
emitStream(emitter)
            .pipe(JSONStream.stringify())
            .pipe(process.stdout);
```

