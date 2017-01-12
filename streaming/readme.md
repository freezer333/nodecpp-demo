# Purpose
`streaming-worker` is designed to give you a simple interface for sending and receiving events/messages from a long running asynchronous C++ Node.js Addon.

**Note** - this library is really only for creating very specific types of Node.js C++ addons, ones that are relatively long running and either need to receive a continuous stream of inputs or will send a stream of outputs to/from your JavaScript code (or both).  The addons operate in separate worker threads, and use `nan`'s [AsyncProgressWorker](https://github.com/nodejs/nan/blob/master/doc/asyncworker.md) to facilitate stream-like and event-like interfaces.

This library is based on a chapter in [Node C++ Integration](http://scottfrees.com/ebooks/nodecpp/) - which covers its implementation in a lot more detail.

# Usage
`streaming-worker` is a C++/JS SDK for building streaming addons - it's not an addon itself!.  You create addons by inheriting from the `StreamingWorker` abstract class defined in the SDK.  Your addon, at a minimum, needs to implement a few virtual functions (most importantly, `Execute`), and can utilize standard methods to read and write `Message` objects to and from JavaScript.  `Message` objects are name/value (string) pairs.

Once your C++ addon is built, you'll package it up with the JavaScript adapter (`index.js`) in the SDK.  This exports a single factory method which accepts a path to the asynchronous C++ Node.js addon you've built.  The second parameter (optional) is for any parameters you wish to send to C++ to initialize the addon.

## Step 1:  Download `/dist`
To build an addon using this interface, you need to download the C++ header and JavaScript hook/adapter. You can get the source from github:

```
git clone https://github.com/freezer333/nodecpp-demo.git
cd nodecpp-demo/streaming
```  

## Step 2:  Build your C++ addon
When you download the code, the `/dist` directory contains all you need to create a new addon.  To build an addon, you need to setup (probably in a seperate directory) a `binding.gyp` file, a `package.json` file, and whatever C++ source files you'll need for your addon.

### Setup binding.gyp
Your `binding.gyp` file is the input to the build process, it's necessary whenever you are creating C++ addons.  To use the SDK, you need to make sure you (at a minimum) add the necessary information so `node-gyp` can find the `/dist` directory with the SDK's header file and so it can use `NAN`.  In addition, you'll probably want to enable C++11 at least.

```js
{
  "targets": [
    {
      "target_name": "ADDON_NAME",
      "sources": [ "YOUR ADDON SOURCE.cpp" ], 
      "cflags": ["-Wall", "-std=c++11"],
      "include_dirs" : ["<!(node -e \"require('nan')\")", "PATH TO YOUR /dist DIRECTORY"],
      'xcode_settings': {
        'OTHER_CFLAGS': [
          '-std=c++11'
        ],
      },
      "conditions": [
        [ 'OS=="mac"', {
            "xcode_settings": {
                'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11','-stdlib=libc++'],
                'OTHER_LDFLAGS': ['-stdlib=libc++'],
                'MACOSX_DEPLOYMENT_TARGET': '10.7' }
            }
        ]
      ]
    }
  ]
}
```
Checkout the examples below for some actual working `binding.gyp` files.

### Setup package.json
Your `package.json` works in conjunction with node-gyp, and will ensure `nan` is installed properly.  The key fields you need to put here are:
1. the ADDON_NAME, which should match the string your put in as `target_name` in `binding.gyp`
2. `gypfile` should be `true` so `npm install` knows to build the addon
3. `nan` and `streaming-worker` need to be dependencies.  Remember, your JavaScript program that uses your addon also uses the JS adapter that comes with this SDK.  You can link it locally, or include it from `npm`.  More on this in Step 3.  

```
{
  "name": "ADDON_NAME",
  "gypfile": true,
  "dependencies": {
  	"nan": "*",
    "streaming-worker" : "file:../../dist"
  }
}
```

### Implement StreamingWorker interface
Finally, your C++ addon code!  You'll need to implement a class that inherits from the `StreamingWorker` abstract base class defined in `/dist/streaming-worker.h`.  This class actually already extends `nan`'s AsyncProgressWorker class.  You also need to create a proper constructor in your addon class, along with a factory method to help the base class build your addon.

#### Addon class constructor
Your constructor must, at a minimum, accept three callback objects and on options object.  You do not need to directly deal with these callbacks in anyway - other than passing them along to the base class constructor.  They are specified by the SDK's JavaScript adapter/module - not your actual JavaScript programs that use the addon.

```cpp
class MyStreamingAddon : public StreamingWorker {
  public:
    MyStreamingAddon(Callback *data, Callback *complete, Callback *error_callback,  v8::Local<v8::Object> & options) 
          : StreamingWorker(data, complete, error_callback) // MUST pass the callbacks to the base constructor!
    {
        if (options->IsObject() ) {
          // extract any options you want to work with. The user of the addon must supply these from JavaScript
        }
    }
    ...
};
```

#### Addon `Execute` method
Once the addon is created, the base class will queue your addon up as a worker thread, hooked into `libuv`'s event loop.  Your addon will be invoked within its `Execute` function.  **Beware, this function is executed in a worker thread, not the Node.js event loop thread!**.  Synchronization when accessing any state variables (member variables) is your responsibility.  The `Execute` function must accept a `progress` object, which is defined by `nan`'s [AsyncProgressWorker](https://github.com/nodejs/nan/blob/master/doc/asyncworker.md) class.  You won't use it directly, but you'll pass it along to the sending method when you want to send messages to Node.js.

```cpp
// Member method of MyStreamingAddon
void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
       // send and receive messages from Node.js code
}
```

You communicate with Node.js through `Message` objects - which are simple name value pairs (values are strings, feel free to extend the implementation to accomodate other types/templates!).


```cpp
/// defined in streaming-worker.h
class Message {
public:
  string name;
  string data;
  Message(string name, string data) : name(name), data(data){}
};
```

Your addon inherits a thread-safe queues, `fromNode`, for reading messages sent to it from JavaScript (see below for the JavaScript API).  You can extract Message objects from the queue at any time by calling `read()` - but keep in mind this is a *blocking* call.

Your addon also inherits a thread-safe method for sending messages back to JavaScript, called `writeToNode`.  This call requires two parameters, the `progress` object passed into your `Execute` method, and the message to send.

Below is a simple `Execute` function that receives a message from Node and just returns the square.  It stops if it receives -1.

```cpp
// Member method of MyStreamingAddon
void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
  int value;
  do {
    Message m = fromNode.read();
    value = std::stoi(m.data);
    Message tosend("square", std::to_string(value*value));
    writeToNode(progress, tosend);
 } while (value > 0);
}
```

#### Addon setup code
You need to add just a little bit of boilerplate C++ code to properly bootstrap your addon.  The following functions are defined outside of your addon class.

The first is a factory method named `create_worker`.  You MUST include this function, and you cannot alter the signature at all.  The base wrapper class calls this to build your particular worker.  The prototype for this function is defined in addon-streams.h

```cpp
StreamingWorker * create_worker(Callback *data
    , Callback *complete
    , Callback *error_callback, v8::Local<v8::Object> & options) {
        
 return new MyStreamingAddon(data, complete, error_callback, options);
}
```
Lastly, you need to initialize your module using the standard Node.js convention.  Replace <ADDON_NAME> with whatever you named your module (`target_name` in `binding.gyp`).  Keep `StreamWorkerWrapper::Init` unchanged, it's the bootstrap method defined in `streaming-worker.h`.  It ultimately calls the `create_worker` method defined above.

```cpp
NODE_MODULE(<ADDON_NAME>, StreamWorkerWrapper::Init)
```

## Step 3:  Write your JavaScript program
A Node.js program that uses the addon must require the `streaming-worker` module:

```js
const worker = require("streaming-worker");
```
Since you've already downloaded the SDK to develop your addon, you might want to just use your local `streaming-worker` module - you can just put the following in your `package.json` file:

```js
"dependencies": {
    "nan": "*",
    "streaming-worker" : "file:<path to /dist directory>"
  }
```

Alternatively, you can do an `npm install --save streaming-worker`.  The module exports a factory function, which is used to startup your C++ addon.  All you need to do is give it the file path where your addon is located:

```js
const path = require("path");
var addon_path = path.join(__dirname, "PATH TO ADDON");
const streaming_addon = worker(addon_path, {foo: "bar"});
```
The optional second parameter is for initialization variables - see the examples below for usage.

### Using the Event Emitter API
The `streaming_addon` object represents your C++ addon, and `streaming-worker` has added two `EventEmitter`-like interfaces on it - `to` and `from`.  As you might expect, `to` allows you to emit events *to* your addon - your addon will read the messages using the `fromNode.read()` call.  The `from` object lets you listen for events sent *from* your addon - which is done in C++ using the `writeToNode` method.

```js

streaming_addon.to.emit("input", "here's something");
streaming_addon.to.emit("input", "here's something else");

streaming_addon.from.on('event', function(message){
    console.log("Got something from the addon!");
});

streaming_addon.from.on('error', function(e) {
    console.error("Something has gone wrong in the addon");
});

streaming_addon.from.on('close', function() {
    console.error("The addon has terminated (natural causes)");
});
```

When you send an event to your addon, the C++ code will read a `Message` object whose name is set to the event name (first parameter of `emit`) and data is set to the actual message (second parameter of `emit`).  Likewise, when sending a `Message` object from C++, its name will be the name of the event that gets emitted.

### Using the Streaming API
Ok, so that's not exactly "streaming"... the `streaming-worker` adapter also adds two methods for creating actual streams to and from your C++ code too though.  To capture the output of your C++ via a stream, you can use the `stream()` method on the `from` object:

```js
const str_out = streaming_addon.from.stream();
```

It's likely you'll want to pipe this to something, I recommend getting familiar with [`through`](https://github.com/dominictarr/through).

```js
str_out.pipe(
    through(function(message) { 
      	this.queue(JSON.parse(message));
    })).pipe(
    through(function(message) {
        console.log(message);
    }));
```

To create an input stream, us the `stream` method on the `to` object:

```js
const str_in = streaming_addon.to.stream("value",
	function () {
		str_in.to.emit('value', -1);
	});
```
Unlike the output stream, you need to specify a few more things when creating the input stream, since `streaming-worker` needs to know how to turn the streamed data into `Message` objects for your addon to read.  The first parameter specifies the name that should be attached to each data put into the stream.  The second parameter is a callback that will be invoked when the stream is closed.  Under normal circumstances, your C++ code is likely to be looking for some sort of sentinal value - and this is your opportunity to send it.  

```js
const streamify = require('stream-array');

// the addon will get -1 when the array is drained, 
// because we specified the end callback above 
streamify([1, 2, 3]).pipe(input); 
```

If the stream is expected to receive the sentinal anyway, then you can just leave the callback undefined.

```js
const str_in = streaming_addon.to.stream("value");
streamify([1, 2, 3, -1]).pipe(input); // the addon receives the -1 it is looking for...
```

# Examples
Below are five examples designed to give you an overview of some of the ways you can interact with addons written using the streaming-worker API.  The first two use the event emitter interface to communicate with the addons.  The third and fourth examples show you how to use the stream interface.  The fifth example combines a few of the examples by piping addons together.

To get started:

```
git clone https://github.com/freezer333/nodecpp-demo.git
cd nodecpp-demo/streaming
``` 
## Prime Factorization
This example uses a C++ addon to compute the prime factorization of larg(ish) numbers.  The number to be factored is passed in as options to the addon, which immediately begins emitting prime factors as it finds them.  The JavaScript code listens to the event emitter interface exposed by the C++ worker and simply prints the factors to the screen.  

The demo is found in `/streaming/examples/factorization`.  Do an `npm install` followed by an `npm start` to run it. 

## Even/Odd Generator
This example shows you how to send input to your addons via the event emitter interface.  The `even_odd` C++ code sits in a loop waiting for integer input (N).  Once received, if N is not -1, it emits `even_event` and `odd_event` messages for each number from 0...N.  

This example also accepts optional parameters on startup, in this case which number to start from (instead of 0).

The example itself emits a few inputs (10, then 5 seconds later, 20) to the addon and closes it by emitting a -1 at the end.

The demo is found in `/streaming/examples/even_odd`.  Do an `npm install` followed by an `npm start` to run it.

## Sensor Data (simulation)
This example demonstrates creating a **streaming interface** to capture output from a C++ addon.  It's meant to mimic what you might see from a positional tracker, like something reporting the position of a head-mounted display in a VR application.   The C++ addon emits sensor data at a regular interval. The sensor data emitted is an *object*, which is **serialized to JSON** strings using the [JSON](https://github.com/nlohmann/json) library for C++.

The JavaScript instantiates the addon, which immediately starts emitting the sensor data.  In the example, to interfaces to capture the output are used.

The first method is the same event emitter interface from the previous examples.  In the second, an output stream is created (using [`emit-stream`](https://github.com/substack/emit-stream) internally).  That stream is subsequently piped through some transforms to deserialize the JSON and pull out the data (not the message name) using [`through`].  The result is streaming sensor data printed to the console.

The demo is found in `/streaming/examples/sensor_sim`.  Do an `npm install` followed by an `npm start` to run it.

## Accumulation in C++ 
This example demonstrates streaming data *into* your addon.  The addon is a simple accumulator - it receives events, assuming the event data are integers.  The addon sums up all the events it reads from the stream and emits a single "sum" event once the stream has been closed.  

In this example, stream closing is propagated to the addon via a single message whose data is -1.  This is achieved by specifying a custom `end` callback when creating the addon's input stream.  This callback is invoked when the stream is closed.

The accumulator C++ addon also allows filtering of messages, allowing it to selectively choose which events it adds to the "sum".  This demonstrates the use of initialization options again, and is helpful or the final example (below).

The demo is found in `/streaming/examples/accumulation`.  Do an `npm install` followed by an `npm start` to run it.

## Piping addons
This example doesn't create any new C++ addons, it just demonstrates how you can pipe together streaming-worker addons.  The example starts out by creating an `even_odd` streaming worker.  Next, two `accumulator` addons are created with filters on them so one captures only even events (`even_event`) and another captures odd events (`odd_event`).  An input stream is created for both the even and odd accumulator, in both cases input stream closing is mapped to sending an event with `-1` as its data.  The output stream from the `even_odd` addon is piped into **both** accumulator inputs streams.

Finally, the `even_odd` addon is kicked off by emitting `10` to it using its event emitter interface.  The result is that both accumulators receive all the events, but sum up only even or odd respectively.

The demo is found in `/streaming/examples/piping`.  Do an `npm install` followed by an `npm start` to run it.  **Note:** in order to use this example, you also need to do an `npm install` in the `even_odd` and `accumulator` examples!

*Note - this example is only working on Windows and Linux, there is a hangup whenever two addons are used in OS X - working on it!  Pull requests welcome!*
