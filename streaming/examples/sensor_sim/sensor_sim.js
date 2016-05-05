"use strict"; 

const worker = require("streaming-worker");
const through = require('through');
const path = require("path");
const JSONStream = require('JSONStream');

const addon_path = path.join(__dirname, "build/Release/sensor_sim");
const wobbly_sensor = worker(addon_path, {name: "Head Mounted Display"});

// Option 1 - Just use the emitter interface
wobbly_sensor.from.on('position_sample', function(sample){
	console.log("----------- Event -----------");
    console.log(JSON.parse(sample));
    console.log("-----------------------------");
});


// Option 2, we can work with a streaming interface:
const pluck_and_parse = through(function (data) {
           		// the data coming in is an array, 
           		// Element 0 is the name of the event emitted by the addon (position_sample)
           		// Element 1 is the data - which in this case is 
           		this.queue(JSON.parse(data[1]));
               });

// the stream isn't created unless you ask for it...
const out = wobbly_sensor.from.stream();
out.pipe(pluck_and_parse) // extract data
   .pipe(through(function(sample) { // just print the sample object
      		console.log(">>>>>>>>>>> Stream >>>>>>>>>>");
      		console.log(sample);
      		console.log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
      }))
      
// The addon must poll for the close signal (see sensor_sim.cpp)
setTimeout(function(){wobbly_sensor.close()}, 5000);