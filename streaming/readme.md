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

## Prime Factorization
Using startup parameters and listening to events from the addon

## Even/Odd Generator
Emit inputs to your addon.  Shut addon down by emitting a close event.  Listening to multiple types of events from the addon.

## Sensor Data (simulation)
Streaming position (x/y/z) data from your addon to JavaScript.  Uses JSON serialization to send full JSON objects back to JavaScript.

## Accumulation in C++ 
Streaming data into your addon, emit event as result.  Send event name as
filter.

## Piping addons
Using multiple addons (even/odd, accumulate).  Accumulate odd numbers only.