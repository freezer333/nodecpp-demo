# Purpose
`streaming-worker` is designed to give you a simple interface for sending and receiving events/messages from a long running asynchronous C++ Node.js Addon.

# Usage
To use `streaming-worker` in your JavaScript code, you need to do an `npm install streaming-worker`.  The module exports a single factory method which accepts a path to an asynchronous C++ Node.js addon that is setup to use the streaming-worker (see below) C++ API.  The second parameter (optional) is for any parameters you wish to send to C++ to initialize the addon.

## Step 1:  Download `/dist` or `npm streaming-worker`

## Step 2:  Build your C++ addon
### Setup binding.gyp
### Setup package.json
### Implement StreamingWorker interface

## Step 3:  Write your JavaScript program


# Examples
Below are five examples designed to give you an overview of some of the ways you can interact with addons written using the streaming-worker API.  The first two use the event emitter interface to communicate with the addons.  The third and fourth examples show you how to use the stream interface.  The fifth example combines a few of the examples by piping addons together.

To get started, do a `git clone https://github.com/freezer333/nodecpp-demo.git` and `cd` into the `streaming` directory.
## Prime Factorization
Using startup parameters and listening to events from the addon

## Even/Odd Generator
This example shows you how to send input to your addons via the event emitter interface.  The `even_odd` C++ code sits in a loop waiting for integer input (N).  Once received, if N is not -1, it emits `even_event` and `odd_event` messages for each number from 0...N.  

This example also accepts optional parameters on startup, in this case which number to start from (instead of 0).

The example itself emits a few inputs (10, then 5 seconds later, 20) to the addon and closes it by emitting a -1 at the end.

The demo is found in `/streaming/examples/even_odd`.  Do an `npm install` followed by an `npm start` to run it.

## Sensor Data (simulation)
This example demonstrates creating a **streaming interface** to capture output from a C++ addon.  It's meant to mimick what you might see from a positional tracker, like something reporting the position of a head mounted display in a VR application.   The C++ addon emits sensor data at a regular interval. The sensor data emitted is an *object*, which is **serialized to JSON** strings using the [JSON](https://github.com/nlohmann/json) library for C++.

The JavaScript instantiates the addon, which immediately starts emitting the sensor data.  In the example, to interfaces to capture the output are used.

The first method is the same event emitter interface from the previous examples.  In the second, an output stream is created (using [`emit-stream`](https://github.com/substack/emit-stream) internally).  That stream is subsequently piped through some transforms to deserialize the JSON and pull out the data (not the message name) using [`through`].  The result is streaming sensor data printed to the console.

The demo is found in `/streaming/examples/sensor_sim`.  Do an `npm install` followed by an `npm start` to run it.

## Accumulation in C++ 
This example demonstrates streaming data *into* your addon.  The addon is a simple accumulator - it receives events, assuming the event data are integers.  The addon sums up all the events it reads from the stream and emits a single "sum" event once the stream has been closed.  

In this example, stream closing is propogated to the addon via a single message whose data is -1.  This is achieved by specifying a custom `end` callback when creating the addon's input stream.  This callback is invoked when the stream is closed.

The accumulator C++ addon also allows filtering of messages, allowing it to selectively choose which events it adds to the "sum".  This demonstrates the use of initialization options again, and is helpfulf or the final example (below).

The demo is found in `/streaming/examples/accumulation`.  Do an `npm install` followed by an `npm start` to run it.

## Piping addons
This example doesn't create any new C++ addons, it just demonstrates how you can pipe together streaming-worker addons.  The example starts out by creating an `even_odd` streaming worker.  Next, two `accumulator` addons are created with filters on them so one captures only even events (`even_event`) and another captures odd events (`odd_event`).  An input stream is created for both the even and odd accumulator, in both cases input stream closing is mapped to sending an event with `-1` as it's data.  The output stream from the `even_odd` addon is piped into **both** accumulator inputs streams.

Finally, the `even_odd` addon is kicked off by emiting `10` to it using its event emitter interface.  The result is that both accumulators receive all the events, but sum up only even or odd respectively.

The demo is found in `/streaming/examples/piping`.  Do an `npm install` followed by an `npm start` to run it.  **Note:** in order to use this example, you also need to do an `npm install` in the `even_odd` and `accumulator` examples!
