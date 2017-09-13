# Introduction

I love doing high-level work in node.js, but sometimes I'm working on data analysis that needs to be done in a higher performance language.  C++ is usually a good choice for these tasks, and a great thing about node is how easy it is to move data to and from C++ with the node's addon mechanism - using the V8 API.  There's a lot of documentation on the [node](http://nodejs.org/api/addons.html) site, but I've found it hard to locate full examples where there are full data structures flowing between JavaScript and C++... so I wrote this.  

In this article I'll show you how to call C++ from JavaScript, passing JavaScript objects to C++ - which are turned into first-class objects matching a C++ class definition.  I'll show you how to pass different C++ objects back to node as JavaScript objects.  I'll also show you how to pass lists of objects back and forth, along with nested class/object use-cases. Finally, I'll show you how to turn your addon into a fully asynchronous module using libuv so your Node application doesn't have to block when calling C++!

# Integration Pattern
I've chosen to handle objects in a way that minimizes the impact of the actual C++ code called by node.  This means that I did *not* employ the V8 class wrapping strategies, instead electing to code all transfer between V8 data types and C++ classes myself, in separate functions.  I like this method, because it keeps the V8 code isolated - and works when you don't want to directly mess with existing C++ code you are calling from node.  If you are looking to have a more automatic method of mapping V8 to C++ data structures, see this [excellent article](http://code.tutsplus.com/tutorials/writing-nodejs-addons--cms-21771), along with the [Node.js documentation](http://nodejs.org/api/addons.html#addons_wrapping_c_objects). 

# Node Version
The code presented is based on Node.js v0.12 and above.  Node v0.12 integrated a new version of the V8 JavaScript engine, which contained a lot of API breaking changes to C++ integration.  Read more about it [here](https://strongloop.com/strongblog/node-js-v0-12-c-apis-breaking/).  **If you aren't using Node v0.12 and above, some of this code won't work for you!**

# Data Model (The example)
I'm going to create a node program sends a json object containing rain fall sample data to C++ for processing.  The sample data contains a list of `locations`, marked by their latitude and longitude.  Each `location` also has a list of `samples` containing the date when the measurement was taken and the amount of rainfall in cm.  Below is an example.  

*Note - you can find all the source code for this post [on github, here](https://github.com/freezer333/nodecpp-demo)*.


```
{
  "locations" : [
    {
      "latitude" : "40.71",
      "longitude" : "-74.01",
      "samples" : [
          {
             "date" : "2014-06-07",
             "rainfall" : "2"
          },
          {
             "date" : "2014-08-12",
             "rainfall" : "0.5"
          },
          {
             "date" : "2014-09-29",
             "rainfall" : "1.25"
          }
       ]
    },
    {
      "latitude" : "42.35",
      "longitude" : "-71.06",
      "samples" : [
          {
             "date" : "2014-03-03",
             "rainfall" : "1.75"
          },
          {
             "date" : "2014-05-16",
             "rainfall" : "0.25"
          },
          {
             "date" : "2014-03-18",
             "rainfall" : "2.25"
          }
       ]
    }
  ]
}
```

The JavaScript code will call a C++ addon to calculate average and median rainfall for each location.  *Yes, I know average/median is not exactly a "heavy compute" task - this is just for show*.  In Part 1 of this tutorial, I'll just pass one location to C++ (with several rainfall samples) and C++ will just return the average as a simple numeric value.  

# Part 1 - Passing Data into C++ from Node

## Creating the C++ addon
We'll create the logic / library by defining simple C++ classes to represent locations and samples, and a function to calculate the average rainfall for a given location.  

```
class sample {
public:
  sample (); // in rainfall.cc
  sample (string d, double r) ; // in rainfall.cc
  string date;
  double rainfall;
};

class location {
public:
  double longitude;
  double latitude;
  vector<sample> samples;
};

// Will return the average (arithmetic mean) rainfall for the give location
double avg_rainfall(location & loc); // code in rainfall.cc
```

If you download the [source from github](https://github.com/freezer333/nodecpp-demo) you can see the implementation along with a simple test program.  Everything related to the C++ code and building the addon is in a directory called `/cpp`.

Now we need to make this code available to Node.js by building it as an addon.

## Creating the V8 entry point code
To expose our C++ "library" as a node.js addon we build some wrapper code.  The [Node.js official documentation](http://nodejs.org/api/addons.html) has some very good explanation of the basics to this.  

We need to create a new .cc file (I called it `rainfall_node.cc`), which includes
the `node.h` and `v8.h` headers.  Next, we need to define an entry point for our addon - which is achieved by creating a function and registering it via a macro provided by the node/v8 headers.

```
#include <node.h>
#include <v8.h>
#include "rainfall.h" 

using namespace v8;

void init(Handle <Object> exports, Handle<Object> module) {
 // we'll register our functions to make them callable from node here..
}

// associates the module name with initialization logic
NODE_MODULE(rainfall, init)  
```

In the `init` function (we can name it anything, as long as we associate it in the NODE_MODULE macro) we will define which functions are going to be exposed to Node.js when are module is included/required.  As you will see, the wrapper code to do all this gets a little ugly, which is why I think its important to keep your clean C++ code (the rainfall.h/cc files) separate from all this.

So the first thing I'll do is expose the `avg_rainfall` method from rainfall.h by creating a new function in `rainfall_node.cc`.

```cpp
void AvgRainfall(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  Local<Number> retval = v8::Number::New(isolate, 0);
  args.GetReturnValue().Set(retval)
}
```

The signature of this function is dictated by the node/v8 API - and its the first place we are seeing some [important changes](https://strongloop.com/strongblog/node-js-v0-12-c-apis-breaking/) that arrived in Node v 0.12.  The input arguments which will be passed from a JavaScript call are V8 objects.   The `isolate` object represents the actual V8 VM instance (your heap) - it will be passed around quite a bit, as its required when creating new instances of objects and primitives. 

The return value is set at the last line of the function (note, its a `void` function in Node v0.12+).  As currently written, the function always just returns 0 as the average rainfall - we'll fix that soon...

Now lets make this function callable from node, by registering it within the `init` function from earlier.

```
void init(Handle <Object> exports, Handle<Object> module) {
  NODE_SET_METHOD(exports, "avg_rainfall", AvgRainfall);
}
```

The init function is called when the module is first loaded in a node application; it is given an export and module object representing the module being constructed and the object that is returned after the `require` call in JavaScript.  The `NODE_SET_METHOD` call is adding a method called `avg_rainfall` to the exports, associated with our actual `AvgRainfall` function from above.  From JavaScript, we'll see a function called "avg_rainfall", which at this point just returns 0.

Much of what I've covered so far can be found in the standard Node tutorials.  Now its time to modify the `AvgRainfall` wrapper code so it can accept JavaScript objects (`location`) and transform them into the C++ versions in order to actually call the **actual** average rainfall function we defined originally in `rainfall.cc`.

### Mapping JavaScript object to C++ class
The `const v8::FunctionCallbackInfo<v8::Value>& args` input parameter represents a collection of all arguments passed by JavaScript when the `AvgRainfall` function is called.  I'll explain how this is setup later - but for example, you might have the following JavaScript code:

```
var rainfall = require("rainfall");
var location = {
	latitude : 40.71, longitude : -74.01,
       samples : [ 
          { date : "2014-06-07", rainfall : 2 },
          { date : "2014-08-12", rainfall : 0.5}
       ] };

console.log("Average rain fall = " + rainfall.avg_rainfall(location) + "cm");
```

To do this, we need some additional code to extract the object properties and instantiate C++ objects.  I'll pack this transfer code into a separate function called within the newly revised `AvgRainfall` function:

``` 
void AvgRainfall(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  
  location loc = unpack_location(isolate, args);
  double avg = avg_rainfall(loc);

  Local<Number> retval = v8::Number::New(isolate, avg);
  args.GetReturnValue().Set(retval);
}
```

The `unpack_location` function accepts the VM instance and the argument list, and unpacks the V8 object into a new `location` object - and returns it.

```
location unpack_location(Isolate * isolate, const v8::FunctionCallbackInfo<v8::Value>& args) {
  location loc;
  Handle<Object> location_obj = Handle<Object>::Cast(args[0]);
  Handle<Value> lat_Value = 
                location_obj->Get(String::NewFromUtf8(isolate,"latitude"));
  Handle<Value> lon_Value = 
                location_obj->Get(String::NewFromUtf8(isolate,"longitude"));
  loc.latitude = lat_Value->NumberValue();
  loc.longitude = lon_Value->NumberValue();

  Handle<Array> array =  Handle<Array>::Cast(
                        location_obj->Get(
                              String::NewFromUtf8(isolate,"samples")));

  int sample_count = array->Length();
  for ( int i = 0; i < sample_count; i++ ) {
    sample s = unpack_sample(isolate, Handle<Object>::Cast(array->Get(i)));
    loc.samples.push_back(s);
  }
  return loc;
}
```

The `unpack_sample` function is similar - this is all a matter of unpacking the data from V8's data types.  

```
sample unpack_sample(Isolate * isolate, const Handle<Object> sample_obj) {
  sample s;
  Handle<Value> date_Value = 
               sample_obj->Get(String::NewFromUtf8(isolate, "date"));
  Handle<Value> rainfall_Value = 
              sample_obj->Get(String::NewFromUtf8(isolate, "rainfall"));

  v8::String::Utf8Value utfValue(date_Value);
  s.date = std::string(*utfValue, utfValue.length());
 
  // Unpack the numeric rainfall amount directly from V8 value
  s.rainfall = rainfall_Value->NumberValue();
  return s;
}
```

## Installing node-gyp
To create a C++ addon we'll need to compile/package the .cc/.h files using `node-gyp`.  As discussed [here](http://www.benfarrell.com/2013/01/03/c-and-node-js-an-unholy-combination-but-oh-so-right/), you don't want to be using the deprecated WAF tools for this step.

You can find a lot more detail about `node-gyp` on [the project's site](https://github.com/TooTallNate/node-gyp).

>node-gyp is a cross-platform command-line tool written in Node.js for compiling native addon modules for Node.js. It bundles the gyp project used by the Chromium team and takes away the pain of dealing with the various differences in build platforms. 

Installing it is easy - but before executing the following make sure you have the following already installed on your machine:

* python (v2.7 recommended, v3.x.x is not supported)
* make (or Visual Studio on Windows)
* C/C++ compiler toolchain, like GCC (or Visual Studio on Windows)

If you meet those requirements, go ahead and install `node-gyp` globally on your system.

```console256
> npm install -g node-gyp
```
## Building the C++ addon
Next we need to create a build file that instructs `node-gyp` on how to assemble our addon.  Create a file called `binding.gyp` in the same directory as the C++ code you have already.  

```js
{
  "targets": [
    {
      "target_name": "rainfall",
      "sources": [ "rainfall.cc" , "rainfall_node.cc" ],
      "cflags": ["-Wall", "-std=c++11"],
      'xcode_settings': {
        'OTHER_CFLAGS': [
          '-std=c++11'
        ],
      },
    }
  ]
}
```
This is just a json file with a collection of properties.  The target name is your addon/module name - **it must match the name you gave in NODE_MODULE macro in the `rainfall_node.cc` file!**.  The sources property should list all C++ code files (you do not need to list the headers) that will be compiled.  I've also added compiler flags, particularly because I'm using some C++ 11 code in rainfall.cc. I needed to add the xcode_settings property to make this work on OS X (see background [here](https://github.com/TooTallNate/node-gyp/issues/26)).

With this is place, you can build your module:

```
> node-gyp configure build
```
If all goes well here you will have a `/build/Release` folder created right alongside your C++ code files.  Within that folder, there should be a `rainfall.node` output file.  **This is your addon**... ready to be required from node.

# Node.js app
Below is the same JavaScript listing from above, with the only change being the require call - which is a little ugly because we are requiring a local package (I'll explain how to package this for npm usage in another post).  Create this file (rainfall.js) in the directory **above** the cpp folder containing your C++ source code.

```js
var rainfall = require("./cpp/build/Release/rainfall.node");
var location = {
    latitude : 40.71, longitude : -74.01,
       samples : [
          { date : "2014-06-07", rainfall : 2 },
          { date : "2014-08-12", rainfall : 0.5}
       ] };

console.log("Average rain fall = " + rainfall.avg_rainfall(location) + "cm");
```
You should be able to run it - and see that your C++ module has been called!

```
> node rainfall.js
Average rain fall = 1.25cm
```

# Next up...
We now have a fully functional node app calling C++.  We've successfully transformed a single JavaScript object into a C++ object.  In [Part 2](http://blog.scottfrees.com/c-processing-from-node-js-part-2) of this series, I'll expand on this example so the C++ code returns a full "result" object - along the lines of the class defined below.

```c++
class rain_result {
   public:
       float median;
       float mean;
       float standard_deviation;
       int n;
};
``` 

# part 2
This article is Part 2 of a series of posts on moving data back and forth between Node.js and C++.  In [Part 1](http://blog.scottfrees.com/c-processing-from-node-js), I built up an example of processing rainfall accumulation data in C++ and returning a simple statistic (average) back to JavaScript.

The JavaScript object passed into C++ looked something like this:

```json
{
  "locations" : [
    {
      "latitude" : "40.71",
      "longitude" : "-74.01",
      "samples" : [
          {
             "date" : "2014-06-07",
             "rainfall" : "2"
          },
          {
             "date" : "2014-08-12",
             "rainfall" : "0.5"
          },
          {
             "date" : "2014-09-29",
             "rainfall" : "1.25"
          }
       ]
    },
    {
      "latitude" : "42.35",
      "longitude" : "-71.06",
      "samples" : [
          {
             "date" : "2014-03-03",
             "rainfall" : "1.75"
          },
          {
             "date" : "2014-05-16",
             "rainfall" : "0.25"
          },
          {
             "date" : "2014-03-18",
             "rainfall" : "2.25"
          }
       ]
    }
  ]
}
```

And we called C++ by invoking a function exposed using the V8 API.

```js
var rainfall = require("rainfall");
var location = {
	latitude : 40.71, longitude : -74.01,
       samples : [ 
          { date : "2014-06-07", rainfall : 2 },
          { date : "2014-08-12", rainfall : 0.5}
       ] };

console.log("Average rain fall = " + rainfall.avg_rainfall(location) + "cm");
```

In Part 1, we focused only on unpacking/transforming a JavaScript input object into a regular old C++ object.  We returned a primitive (average, as a `double`) - which was pretty easy.  I also wrote a lot about how to get your addon built using `node-gyp` - which I won't repeat here.  *Remember, this code is targeting the V8 C++ API distributed with Node v0.12 and above*

*Note - you can find all the source code for this post [on github, here](https://github.com/freezer333/nodecpp-demo)*.

In this post - Part 2 - I'm going to now return an object back to JavaScript consisting of several statistics about the list of rainfall samples passed into C++.  The result object will look like this in C++:

```c++
// declared in rainfall.h
class rain_result {
   public:
       float median;
       float mean;
       float standard_deviation;
       int n;
};
``` 

As explained in [Part 1](), I'm keeping the "business" part of my C++ code completely separate from the code dealing with V8 integration.  So the class above has been added to the `rainfall.h / rainfall.cc` files.

# Handling the input
We're going to now create a new callable function for the Node addon.  So, in the rainfall_node.cc file (where we put all our V8 integration logic), I'll add a new function and register it with the module's exports.

```c++
void RainfallData(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  
  location loc = unpack_location(isolate, args);
  rain_result result = calc_rain_stats(loc);

/*
 .... return the result object back to JavaScript  ....
*/
}
```
Recall from [Part 1](), the `unpack_location` function is where I'm extracting the location (and rainfall samples) from the JavaScript arguments.  I've introduced a new function in `rainfall.h / rainfall.cc` called `calc_rain_stats` which returns a `rain_result` instance based on the `location` instance it is given.  It computes mean/median/standard deviation (see [here](https://github.com/freezer333/nodecpp-demo/blob/master/cpp/rainfall.cc) for implementation.

The `RainfallData` function is exported with the addon by adding another call to `NODE_SET_METHOD` inside the `init` function in `rainfall_node.cc`.

```c++
void init(Handle <Object> exports, Handle<Object> module) {
  // from part 1
  NODE_SET_METHOD(exports, "avg_rainfall", AvgRainfall);
  // now added for part 2
  NODE_SET_METHOD(exports, "data_rainfall", RainfallData);
}
```

# Building the JavaScript object and returning it
After unpacking the `location` object inside the RainfallData function, we got a `rainfall_result` object:

```C++
rain_result result = calc_rain_stats(loc);
```

Now its time to return that - and to do so we'll create a new V8 object, transfer the rain_result data into it, and return it back to JavaScript.

```C++
void RainfallData(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  
  location loc = unpack_location(isolate, args);
  rain_result result = calc_rain_stats(loc);

  // Creates a new Object on the V8 heap
  Local<Object> obj = Object::New(isolate);
  
  // Transfers the data from result, to obj (see below)
  obj->Set(String::NewFromUtf8(isolate, "mean"), 
                            Number::New(isolate, result.mean));
  obj->Set(String::NewFromUtf8(isolate, "median"), 
                            Number::New(isolate, result.median));
  obj->Set(String::NewFromUtf8(isolate, "standard_deviation"), 
                            Number::New(isolate, result.standard_deviation));
  obj->Set(String::NewFromUtf8(isolate, "n"), 
                            Integer::New(isolate, result.n));

  // Return the object
  args.GetReturnValue().Set(obj);
}
```

First notice the similarities between this function and the AvgRainfall Function from Part 1. They both follow the similar pattern of creating a new variable on the V8 heap and returning it by setting the return value associated with the `args` variable passed into the function.  The difference now is that actually setting the value of the variable being returned is more complicated.  In AvgRainfall, we just created a new `Number`:

```C++
Local<Number> retval = v8::Number::New(isolate, avg);
```

Now, we have we instead move the data over one property at time:

```C++
Local<Object> obj = Object::New(isolate);
obj->Set(String::NewFromUtf8(isolate, "mean"), 
                   Number::New(isolate, result.mean));
obj->Set(String::NewFromUtf8(isolate, "median"), 
                   Number::New(isolate, result.median));
obj->Set(String::NewFromUtf8(isolate, "standard_deviation"), 
                   Number::New(isolate, result.standard_deviation));
obj->Set(String::NewFromUtf8(isolate, "n"), 
                   Integer::New(isolate, result.n));
```

While its a bit more code - the object is just being built up with a series of named properties - its pretty straightforward.  

# Invoking a from JavaScript
Now that we've completed the C++ side, we need to rebuild our addon:

```
> node-gyp configure build
```

In JavaScript, we can now call both methods, and we'll see the object returned by our new data_rainfall method returns a real JavaScript object.

```JavaScript
//rainfall.js
var rainfall = require("./cpp/build/Release/rainfall");
var location = {
    latitude : 40.71, longitude : -74.01,
       samples : [
          { date : "2015-06-07", rainfall : 2.1 },
          { date : "2015-06-14", rainfall : 0.5}, 
          { date : "2015-06-21", rainfall : 1.5}, 
          { date : "2015-06-28", rainfall : 1.3}, 
          { date : "2015-07-05", rainfall : 0.9}
       ] };

var avg = rainfall.avg_rainfall(location)
console.log("Average rain fall = " + avg + "cm");

var data = rainfall.data_rainfall(location);

console.log("Mean = " + data.mean)
console.log("Median = " + data.median);
console.log("Standard Deviation = " + data.standard_deviation);
console.log("N = " + data.n);
```

```console256
> node rainfall.js
Average rain fall = 1.26cm
Mean = 1.2599999904632568
Median = 1.2999999523162842
Standard Deviation = 0.6066300272941589
N = 5
```

# Next up...
You now have seen examples of passing simple objects back and forth between C++ and Node.js.  In the [next part](http://blog.scottfrees.com/c-processing-from-node-js-part-3-arrays) of the series, I'll look at some more complex use cases, where lists of objects and nested objects are being moved between JavaScript and the addon.

# part 3
This article is Part 3 of a series of posts on moving data back and forth between Node.js and C++. In [Part 1](http://blog.scottfrees.com/c-processing-from-node-js), I built up an example of processing rainfall accumulation data in C++ and returning a simple statistic (average) back to JavaScript. In [Part 2](http://blog.scottfrees.com/c-processing-from-node-js-part-2) I modified the C++ addon to return a JavaScript object representing more complete statistics about each location/sample.

In each of the previous posts, the JavaScript objects being passed into C++ looked like this:

```js
{
      "latitude" : "42.35",
      "longitude" : "-71.06",
      "samples" : [
          { date : "2015-06-07", rainfall : 2.1 },
          { date : "2015-06-14", rainfall : 0.5}, 
          { date : "2015-06-21", rainfall : 1.5}, 
          { date : "2015-06-28", rainfall : 1.3}, 
          { date : "2015-07-05", rainfall : 0.9}
       ]
    }
```

In Part 2, the C++ addon returned a `rain_result` object that looked list this:

```js
{
	median: 1.2999999523162842
	mean:1.2599999904632568
	standard_deviation: 0.6066300272941589
	n:5
}
```

Now we'll look at passing an array of location data into C++ and having C++ return an array of results back to us.  All the code for this series of posts is found [here](https://github.com/freezer333/nodecpp-demo).

# Receiving an Array from Node
If you haven't read [Parts 1](http://blog.scottfrees.com/c-processing-from-node-js) and [2](http://blog.scottfrees.com/c-processing-from-node-js-part-2) of this post please do so now - its important you understand how I'm integrating C++ and JavaScript classes.  Instead of using the V8 object wrapping API, I'm just packing/unpacking data from V8's native objects into and out of my [POCOs](https://en.wikipedia.org/wiki/Plain_Old_C%2B%2B_Object).  While there is a little added work upfront, we'll see this work now be leveraged to make list processing really very easy.  

## Registering the callable addon function
As always, we start by writing a C++ function in `/cpp/rainfall_node.cc` that will be callable from Node.js.

```cpp
void CalculateResults(const v8::FunctionCallbackInfo<v8::Value>&args) {
    Isolate* isolate = args.GetIsolate();
    std::vector<location> locations;  // we'll get this from Node.js
    std::vector<rain_result> results; // we'll build this in C++
    
    // we'll populate this with the results
    Local<Array> result_list = Array::New(isolate);
   
    // ... and send it back to Node.js as the return value
    args.GetReturnValue().Set(result_list);
}
....
void init(Handle <Object> exports, Handle<Object> module) {
  // part 1
  NODE_SET_METHOD(exports, "avg_rainfall", AvgRainfall);
  // part 2
  NODE_SET_METHOD(exports, "data_rainfall", RainfallData);
  // part 3
  NODE_SET_METHOD(exports, "calculate_results", CalculateResults);
}
```
The `CalculateResults` function will extract a list of location objects from the parameters (`args`) and eventually return a fully populated array of results.  We make it callable by calling the `NODE_SET_METHOD` in the `init` function - so we can call `calculate_results` in JavaScript.

Before we implement the C++, lets look at how this will all be called in JavaScript.  First step is to rebuild the addon from the `cpp` directory:

```
>  node-gyp configure build
```
In the rainfall.js, we'll construct an array of locations and invoke our addon:

```js
// Require the Addon
var rainfall = require("./cpp/build/Release/rainfall");

var makeup = function(max) {
    return Math.round(max * Math.random() * 100)/100;
}

// Build some dummy locations
var locations = []
for (var i = 0; i < 10; i++ ) {
    var loc = {
        latitude: makeup(180), 
        longitude: makeup(180), 
        samples : [
            {date: "2015-07-20", rainfall: makeup(3)}, 
            {date: "2015-07-21", rainfall: makeup(3)}, 
            {date: "2015-07-22", rainfall: makeup(3)}, 
            {date: "2015-07-23", rainfall: makeup(3)}
        ]
    }
    locations.push(loc);
}

// Invoke the Addon
var results = rainfall.calculate_results(locations);

// Report the results from C++
var i = 0;
results.forEach(function(result){
    console.log("Result for Location " + i);
    console.log("--------------------------");
    console.log("\tLatitude:         " + locations[i].latitude.toFixed(2));
    console.log("\tLongitude:        " + locations[i].longitude.toFixed(2));
    console.log("\tMean Rainfall:    " + result.mean.toFixed(2) + "cm");
    console.log("\tMedian Rainfall:  " + result.median.toFixed(2) + "cm");
    console.log("\tStandard Dev.:    " + result.standard_deviation.toFixed(2) + "cm");
    console.log("\tNumber Samples:   " + result.n);
    console.log();
    i++;
})
```

When you run this with `node rainfall` you'll get no output, only because the C++ function is returning an empty array at this point.  Try putting a `console.log(results)` in, you should see `[]` print out.

## Extracting the Array in C++
Now lets skip back to our `CalculateResults` C++ function.  We've been given the function callback arguments object, and our first step is to cast it to a V8 array.

```cpp
void CalculateResults(const v8::FunctionCallbackInfo<v8::Value>&args) {
    Isolate* isolate = args.GetIsolate();
    ... (see above)...
    Local<Array> input = Local<Array>::Cast(args[0]);
    unsigned int num_locations = input->Length();
```

With the V8 array `input`, we'll now loop through and actually create a POCO `location` object using the `unpack_location` function we saw in [Part 2](http://blog.scottfrees.com/c-processing-from-node-js-part-2).  The return value from `unpack_location` is pushed onto a standard C++ vector.

```cpp
for (unsigned int i = 0; i < num_locations; i++) {
  locations.push_back(
       unpack_location(isolate, Local<Object>::Cast(input->Get(i)))
  );
}
```

Of course, now that we have a standard vector of `location` objects, we can call our existing `calc_rain_stats` function on each one and build up a vector of `rain_result` objects.

```cpp
results.resize(locations.size());
std::transform(
     locations.begin(), 
     locations.end(), 
     results.begin(), 
     calc_rain_stats);
```

# Building an Array to return back from C++
Our next step is to move the data we've created into the V8 objects that we'll return.  First, we create a new V8 Array:

```cpp
Local<Array> result_list = Array::New(isolate);
```
We can now iterate through our `rain_result` vector and use the `pack_rain_result` function from [Part 2](http://blog.scottfrees.com/c-processing-from-node-js-part-2) to create a new V8 object and add it to the `result_list` array.

```cpp
for (unsigned int i = 0; i < results.size(); i++ ) {
      Local<Object> result = Object::New(isolate);
      pack_rain_result(isolate, result, results[i]);
      result_list->Set(i, result);
    }
```

And... we're all set.  Here's the complete code for the `CalculateResult` function:

```cpp
void CalculateResults(const v8::FunctionCallbackInfo<v8::Value>&args) {
    Isolate* isolate = args.GetIsolate();
    std::vector<location> locations;
    std::vector<rain_result> results;
    
    // extract each location (its a list)
    Local<Array> input = Local<Array>::Cast(args[0]);
    unsigned int num_locations = input->Length();
    for (unsigned int i = 0; i < num_locations; i++) {
      locations.push_back(
             unpack_location(isolate, Local<Object>::Cast(input->Get(i))));
    }

    // Build vector of rain_results
    results.resize(locations.size());
    std::transform(
          locations.begin(), 
          locations.end(), 
          results.begin(), 
          calc_rain_stats);


    // Convert the rain_results into Objects for return
    Local<Array> result_list = Array::New(isolate);
    for (unsigned int i = 0; i < results.size(); i++ ) {
      Local<Object> result = Object::New(isolate);
      pack_rain_result(isolate, result, results[i]);
      result_list->Set(i, result);
    }

    // Return the list
    args.GetReturnValue().Set(result_list);
}
```

Do another `node-gyp configure build` and re-run ` node rainfall.js` and you'll see the fully populated output results from C++.

```
Result for Location 0
--------------------------
	Latitude:         145.45
	Longitude:        7.46
	Mean Rainfall:    1.59cm
	Median Rainfall:  1.65cm
	Standard Dev.:    0.64cm
	Number Samples:   4

Result for Location 1
--------------------------
	Latitude:         25.32
	Longitude:        98.64
	Mean Rainfall:    1.17cm
	Median Rainfall:  1.24cm
	Standard Dev.:    0.62cm
	Number Samples:   4
....

```

## About efficiency
You might be wondering, aren't we wasting a lot of memory by creating POCO copies of all the V8 data?  Its a good point, for all the data being passed into the C++ Addon, the V8 objects (which take up memory) are being moved into new C++ objects.  Those C++ (and their derivatives) are then copied into new V8 objects to be returned... we're doubling memory consumption and its also costing us processing time to do all this!

For most use cases I end up working with, the overhead of memory copying (both time and space) is dwarfed by the actual execution time of the algorithm and processing that I'm doing in C++.  If I'm going through the trouble of calling C++ from Node, its because the actual compute task is *significant*!  

For situations where the cost of copying input/output isn't dwarfed by your actual processing time, it would probably make more sense to use V8 object wrapping API instead.

# Next up... asynchronous execution
Now that we've seen how to move primitives, objects, and lists between Node and C++, we'll look at how to execute the bulk of our C++ work asynchronously in a separate thread using [libuv](http://libuv.org/) so JavaScript can just give the add-on a callback and continue on its way.

# Part 4
This article is Part 4 of a series of posts on moving data back and forth between Node.js and C++. In [Part 1](http://blog.scottfrees.com/c-processing-from-node-js), I built up an example of processing rainfall accumulation data in C++ and returning a simple statistic (average) back to JavaScript. In Parts [2](http://blog.scottfrees.com/c-processing-from-node-js-part-2) and [3](http://blog.scottfrees.com/c-processing-from-node-js-part-3-arrays), I covered more complex use cases involving moving lists and objects.  This post covers **asynchronous** C++ addons - which are probably the most useful.

# Why Asynchronous?
If you are dropping into C++ to do some calculations, chances are good that you are doing it for speed.  If you are going though all this trouble for speed, then you probably have a hefty calculation to do - and its going to take time (even in C++!).

Unfortunately, if you make a call from JavaScript to a C++ addon that executes synchronously, your JavaScript application will be **blocked** - waiting for the C++ addon to return control.  This basically goes against *ALL best-practices*, your event loop is stalled!

# Asynchronous addon API
If you haven't read parts 1-3 of this series, make sure you do - so you understand the data structures being passed back and forth.  We're sending in lists of lat/longitude positions with rainfall sample data, and C++ is returning back statistics on that data.  Its a contrived example, but its working so far.

So the idea is to change our API from this:

```js
// require the C++ addon
var rainfall = require("./cpp/build/Release/rainfall");
...
// call into the addon, blocking until it returns
var results = rainfall.calculate_results(locations);
print_rain_results(results);
```

.. to something like this:

```js
// pass a callback into the addon, and have the addon return immediately
rainfall.calculate_results_async(locations, function(results) {
  print_rain_results(results);
});
// we can continue here, before the callback is invoked.
```

... or just ...

```js
rainfall.calculate_results_async(locations, print_rain_results);
// we can continue here, before the callback is invoked.
```

# The C++ addon code
Our first step is to create yet another C++ function, and register it with our module.  This is basically the same as in the previous posts.  Make sure you take a look at the full code [here](https://github.com/freezer333/nodecpp-demo)

```cpp
// in node_rainfall.cc
void CalculateResultsAsync(const v8::FunctionCallbackInfo<v8::Value>&args) {
    Isolate* isolate = args.GetIsolate();

     // we'll start a worker thread to do the job 
     // and call the callback here...

    args.GetReturnValue().Set(Undefined(isolate));
}
...
void init(Handle <Object> exports, Handle<Object> module) {
  ...
   NODE_SET_METHOD(exports, "calculate_results_async", CalculateResultsAsync); 
}
```

The `CalculateResultsAsync` function is where we'll end up kicking off a worker thread using libuv  - but notice what it does right away:  it *returns*!  Nothing we fill into this function will be long running, all the real work will be done in the worker thread.

## Worker thread model
Lets do a quick overview of how worker threads should work in V8.  In our model, there are **two threads**.  

The first thread is the *event loop thread* - its the thread that our JavaScript code is executing in, and its the thread that we are **still in** when we cross over into C++ through the `calculate_results_async` function call.  This is the thread that we *don't* want to stall by doing heavy calculations!

The second thread (to be created) will be a worker thread managed by libuv, the library that supports asynchronous I/O in Node.  

Hopefully you're pretty familiar with threads - the key point here is that each thread has it's own stack - you can't share stack variables between the event loop thread and the worker thread!  Threads do share the same heap though - so that's where we are going to put our input and output data, along with state information.

On the C++ side of things, we're going to utilize **three functions** and a **struct** to coordinate everything:

1.  `Worker Data` (struct) - will store plain old C++ input (locations) and output (rain_results) and the callback function that can be invoked when work is complete
1.  `CalculateResultsAsync` - executes in event-loop thread, extracts input and stores it on the heap in *worker data*.
1.  `WorkAsync` - the function that the worker thread will execute.  We'll launch this thread from `CalculateResultsAsync` using the libuv API
1.  `WorkAsyncComplete` - the function that libuv will invoke when the worker thread is finished.  This function is executed on the *event loop thread*, **not** the worker thread.

I like pictures:
![Node and Libuv Worker Thread](http://scottfrees.com/node-worker-c.png)
<img src="https://docs.google.com/drawings/d/1DD6FajO_vGmOHpk1kM2KTvApSIjc7JFF7HlwBBwX8jw/pub?w=960&h=720">

Lets look at the C++ code, starting with our Work Data structure:

```cpp
struct Work {
  uv_work_t  request;
  Persistent<Function> callback;

  std::vector<location> * locations;
  std::vector<rain_result> * results;
};
```

The vector pointers are going to store our input and output, which will be allocated on the heap.  The request object is a handle that will actually loop back to the work object - the libuv API accepts pointers of type `uv_work_t` when starting worker threads. The `callback` variable is going to store the JavaScript callback.  Importantly, its `Persistent<>`, meaning it will be stored on the heap so we can call it when the worker is complete.  This seems confusing (at least to me), since the callback will be executed in the event-loop thread, but the reason we need to put into the heap is because when we initially return to JavaScript, all V8 locals are destroyed.  A new Local context is created when we are about to call the JavaScript callback after the worker thread completes.


Now lets look at the `CalculateResultsAsync` function

```cpp
void CalculateResultsAsync(const v8::FunctionCallbackInfo<v8::Value>&args) {
    Isolate* isolate = args.GetIsolate();
    
    Work * work = new Work();
    work->request.data = work;
```

Notice that the Work struct is created on the heap.  Remember, local variables (and V8 Local objects) will be destroyed when this function returns - even though our worker thread will still be active.  Here we also set the `uv_work_t` data pointer to point right back to the `work` struct so libuv will pass it back to us on the other side.

```cpp
    ...
    // extract each location (its a list) and store it in the work package
    // locations is on the heap, accessible in the libuv threads
    work->locations = new std::vector<location>();
    Local<Array> input = Local<Array>::Cast(args[0]);
    unsigned int num_locations = input->Length();
    for (unsigned int i = 0; i < num_locations; i++) {
      work->locations->push_back(
          unpack_location(isolate, Local<Object>::Cast(input->Get(i)))
      );
    }
```

The code above is really the same as from [Part 3](http://blog.scottfrees.com/c-processing-from-node-js-part-3-arrays). The key part is that we are extracting the arguments sent from JavaScript and putting them in a locations vector stored on the heap, within the `work` struct.

Where earlier we now just went ahead and processed the rainfall data, now we'll kick off a worker thread using libuv.  First we store the callback sent to use from JavaScript, and then we're off.  Notice as soon as we call `uv_queue_work`, we return - the worker is executing in its own thread (`uv_queue_work` returns immediately).  

```cpp

    // store the callback from JS in the work package so we can 
    // invoke it later
    Local<Function> callback = Local<Function>::Cast(args[1]);
    work->callback.Reset(isolate, callback);

    // kick of the worker thread
    uv_queue_work(uv_default_loop(),&work->request,
        WorkAsync,WorkAsyncComplete);
    
    args.GetReturnValue().Set(Undefined(isolate));

}
```

Notice the arguments to `uv_queue_work` - its the work->request we setup at the top of the function, and the two functions we have seen yet - the function to start the thread in (`WorkAsync`) and the function to call when it's complete (`WorkAsyncComplete`).

At this point, control is passed back to Node (JavaScript).  If we had further JavaScript to execute, it would execute now.  Basically, from the JavaScript side, our addon is acting the same as any other asynchronous call we typically make (like reading from files).

## The worker thread
The worker thread code is actually really simple. We just need to process the data - and since its already extracted out of the V8 objects, its pretty vanilla C++ code.  Its largely explained in [Part 3](http://blog.scottfrees.com/c-processing-from-node-js-part-3-arrays), with the exception of the cast of the `work` data.  Notice our function has been called with the libuv work request parameter.  We set this up above to point to our actual work data.

```cpp
static void WorkAsync(uv_work_t *req)
{
    Work *work = static_cast<Work *>(req->data);

    // this is the worker thread, lets build up the results
    // allocated results from the heap because we'll need
    // to access in the event loop later to send back
    work->results = new std::vector<rain_result>();
    work->results->resize(work->locations->size());
    std::transform(work->locations->begin(), 
             work->locations->end(), 
             work->results->begin(), 
             calc_rain_stats);

    // that wasn't really that long of an operation, 
    // so lets pretend it took longer...

    sleep(3);
}
```

Note - the code above also sleeps for extra effect, since the rainfall data isn't really that large in my demo.  You can remove it, or substitute it with Sleep(3000) on Windows (and replace `#include <unistd.h>` with `#include <windows.h>`).

## When the worker completes...
Once the worker thread completes, libuv handles calling our `WorkAsyncComplete` function - passing in the work request object again - so we can use it!

```cpp
// called by libuv in event loop when async function completes
static void WorkAsyncComplete(uv_work_t *req,int status)
{
    Isolate * isolate = Isolate::GetCurrent();
    Work *work = static_cast<Work *>(req->data);

    // the work has been done, and now we pack the results
    // vector into a Local array on the event-thread's stack.
    Local<Array> result_list = Array::New(isolate);
    for (unsigned int i = 0; i < work->results->size(); i++ ) {
      Local<Object> result = Object::New(isolate);
      pack_rain_result(isolate, result, (*(work->results))[i]);
      result_list->Set(i, result);
    }

    ...

```

The first part of the function above is pretty standard - we get the work data, and we package up the results into V8 objects rather than C++ vectors.  Once again, this was all discussed in more detail in [Part 3](http://blog.scottfrees.com/c-processing-from-node-js-part-3-arrays).

Next, we need to invoke the JavaScript callback that was originally passed to the addon.  **Note, this part is a lot different in Node 0.11 than it was in previous versions of Node because of recent V8 API changes.** If you are looking for ways to be a little less dependent on V8, take a look at [Nan](https://github.com/nodejs/nan).

```cpp
    // set up return arguments
    Handle<Value> argv[] = { result_list };
    
    // execute the callback
    Local<Function>::New(isolate, work->callback)->
      Call(isolate->GetCurrentContext()->Global(), 1, argv);
    
    delete work;
}
```

Once you call the callback, you're back in JavaScript!  The `print_rain_results` function will be called...

```js
rainfall.calculate_results_async(locations, function(results) {
  print_rain_results(results);
});
```

