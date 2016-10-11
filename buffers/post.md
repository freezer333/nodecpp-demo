# Using Buffers to share data between Node.js and C++
One of the really nice things about developing with Node.js is the ability to move fairly seamlessly between JavaScript and native C++ code - thanks to the V8's addon API.  The ability to move into C++ is sometimes driven by processing speed; but more often it's because we already have C++ code and we just want to be able to use it from JavaScript.  We can categorize the different use cases for addons over (at least) two axes - (1) amount of processing time we'll spend in the C++ code, and (2) the amount of data flowing between C++ and JavaScript.

![CPU vs. Data quadrant](https://scottfrees.com/quadrant.png)

Most articles discussing C++ addons for Node.js focus on the differences between the left and right quadrants.  If you are in the left quadrants (short processing time), your addon can possibly be synchronous - meaning the C++ code that executes is run directly in the Node.js event loop when called.  In this situation, your addon function blocks - the calling JavaScript code simply waits for the return value from the addon.  In the right quadrants, you would almost certainly design the addon using the asynchronous pattern.  In an asynchronous addon function, the calling JavaScript code returns immediately.  The calling code passes a callback function to the addon, and the addon does it's work in a separate worker thread. This avoids locking up the Node.js event loop, as the addon function does not block.  

The difference between the top and bottom quadrants is often overlooked however - but can be just as important.  This post is going to focus on this topic - and we'll wind up presenting Node.js Buffers as a powerful solution to the data problems we are about to describe.  

## V8 vs. C++ memory and data
If you are new to writing native addons, one of the first things you must master is the differences between V8-owned data (which you **can** access from C++ addons) and normal C++ memory allocations.  When we say "V8-owned", we are referring to the storage cells that hold JavaScript data.  These storage cells are accessibly through V8's C++ API - but they aren't ordinary C++ variables, they can be accessed in only limited ways.  While your addon *could* restrict itself to ONLY using V8 data, it's more than likely it will also create it's own variables - in plain old C++.  These could be stack or heap variables, and of course are completely independent of V8.

In JavaScript, primitives (numbers, strings, booleans, etc.) are *immutable* - storage cells associated with primitive JavaScript variables cannot be altered by a C++ addon.  The primitive JavaScript variables can be reassigned to *new storage cells* created by C++ - but this means changing data always results in *new* memory allocation.  In the upper quadrant (low data transfer), this really isn't a big deal - if you are designing an addon that doesn't have a lot of data exchange, then the overhead of all the new memory allocation in your addon probably doesn't mean much.  As your addons moves closer to the lower quadrant however, the allocation / copying starts to cost you.  For one, It costs you in terms of peak memory usage, and **it also costs you in performance**!  The time cost of copying all this data between JavaScript (V8 storage cells) to C++ (and back) usually kills the performance benefits you might be getting from running C++ in the first place!  For addons in the lower left quadrant (low processing, high data usage), the latency associated with data copying can push your addon towards the right - forcing you to consider an asynchronous design.  

## V8 memory and asynchronous addons  
In asynchronous addons, we execute the bulk of our C++ processing code a worker thread.  If you are unfamiliar with asynchronous callbacks, you might want to checkout a few tutorials (like [here](http://blog.scottfrees.com/building-an-asynchronous-c-addon-for-node-js-using-nan) and [here](http://blog.scottfrees.com/c-processing-from-node-js-part-4-asynchronous-addons)).   

A central tenant of asynchronous addons is that *you can't access V8 (JavaScript) memory outside the event-loop's thread*.  This leads us to our next problem - if we have lots of data, that data must be copied out of V8 memory and into your addon's native address space *from the event loop's thread*, before the worker thread starts.  Likewise, any data produced or modified by the worker thread must be copied back into V8 by code executing in the event loop (in the callback).  If you are interested in high throughput Node.js applications, you should hate spending lots of time in the event loop copying data!

![Creating copies for input and output for a C++ worker thread](https://raw.githubusercontent.com/freezer333/node-v8-workers/master/imgs/copying.gif)

Ideally, we'd prefer a way to do this:

![Accessing V8 data directly from C++ worker thread](https://raw.githubusercontent.com/freezer333/node-v8-workers/master/imgs/inplace.gif)

## Buffers to the rescue
So, we have two somewhat related problems.  When working with synchronous addons, unless we aren't changing/producing data, it's likely we'll need to spend a lot of time moving our data between V8 storage cells and plain old C++ variables - which costs us.  If we are working with asynchronous addons - where ideally we should spend as little time in the event loop as possible, we are still "up a creek" - since we *must* do our data copying in the event loop's thread due to V8's multi-threaded restrictions.  This is where an often overlooked features of Node.js helps us with addon development - the `Buffer`.  Quoting the[Node.js official documentation](https://nodejs.org/api/buffer.html), 

> Instances of the Buffer class are similar to arrays of integers but correspond to fixed-sized, raw memory allocations outside the V8 heap. 

This is exactly what we are looking for - because the data inside a Buffer is *not stored in a V8 storage cell*, it is not subject to the multi-threading rules of V8.  This means we can interact with it **in place** from a C++ worker thread started by an asynchronous addon.

### How Buffers work
Buffers store raw binary data, they are found in the Node.js API for reading files and other I/O devices.  

Borrowing again from some examples in the Node.js documentation, we can create unitialized buffers of a specified size, buffers pre-set with a specified value, buffers from arrays of bytes, and buffers from strings.

```
// buffer with size 10 bytes
const buf1 = Buffer.alloc(10);       

// buffer filled with 1's (10 bytes)
const buf2 = Buffer.alloc(10, 1);    

//buffer containing [0x1, 0x2, 0x3]
const buf3 = Buffer.from([1, 2, 3]); 

// buffer containing ASCII bytes [0x74, 0x65, 0x73, 0x74].
const buf4 = Buffer.from('test');  

// buffer containing bytes from a file
const buf5 = fs.readFileSync("some file");
``` 

Buffers can be turned back into traditional JavaScript data (strings) or written back out to files, databases, or other I/O devices.

### How to access Buffers in C++
When building an addon for Node.js, the best place to start is by making use of the NAN (Native Abstractions for Node.js) API rather than directly using the V8 API - which can be a moving target.  There are many tutorials on the web for getting started with NAN addons - including [examples](https://github.com/nodejs/nan#example) in NAN's codebase itself.


> Present Buffer API in more depth.  Remove original synchronous version.  Remove original file-based version too.

In this section, we'll look at how `Buffer` objects can be passed to and from C++ addons using NAN.  NAN is used because the `Buffer` object API has actually undergone some significant changes recently, and NAN will shield us from these issues.  We'll look at `Buffer` objects through the lens of an image converter - specifically converting binary png image data into bitmap formatted binary data.

All of the code for this section is available in full in the `nodecpp-demo` repository at [https://github.com/freezer333/nodecpp-demo](https://github.com/freezer333/nodecpp-demo), under the "Buffers" section.


## Example:  PNG and BMP Image Processing
Image processing, in general, is anything that manipulates/transforms an image.  An image of course is a big chunk of binary data - in it's most basic state an integer (or 3 or 4) can be used to represent each pixel in an image, and those integers can be stored in a file or held in a contiguously allocated array.  Typically image data is not held in *raw* data form though, it's compressed/encoded into a image format standard such as png, gif, bmp, jpeg, and others.

Image processing is a good candidate for C++ addons, as image processing can often be time consuming, CPU intensive, and some processing technique have parallelism that C++ can exploit.  For the example we'll look at now, we'll simply convert png formatted data into bmp formatted data[^jspng].  There are a good number of existing, open source C++ libraries that can help us with this task, I'm going to use LodePNG as it is dependency free and quite simple to use.  LodePNG can be found at [http://lodev.org/lodepng/](http://lodev.org/lodepng/), and it's source code is at [https://github.com/lvandeve/lodepng](https://github.com/lvandeve/lodepng).  Many thanks to the developer, Lode Vandevenne for providing such an easy to use library!

[^jspng]: Converting from png to bmp is *not* particularly time consuming, it's probably overkill for an addon - but it's good for demonstration purposes.  If you are looking for a pure JavaScript implementation of image processing (including much more than png to bmp conversion), take a look at JIMP at [https://www.npmjs.com/package/jimp]()https://www.npmjs.com/package/jimp.

## Setting up the addon
For this addon, we'll create the following directory structure, which includes source code downloaded from [https://github.com/lvandeve/lodepng](https://github.com/lvandeve/lodepng), namely `lodepng.h` and `lodepng.cpp`.

~~~~~~~~~~{#binding-hello}
/png2bmp
 | 
 |--- binding.gyp
 |--- package.json
 |--- png2bmp.cpp  # the addon
 |--- index.js     # program to test the addon
 |--- sample.png   # input (will be converted to bmp)
 |--- lodepng.h    # from lodepng distribution
 |--- lodepng.cpp  # From loadpng distribution
~~~~~~~~~~

To download the complete addon, head over to [https://github.com/freezer333/nodecpp-demo](https://github.com/freezer333/nodecpp-demo), this particular example is in the `buffers` directory.

`lodepng.cpp` contains all the necessary code for doing image processing, and I will not discuss it's working in detail.  In addition, the lodepng distribution contains sample code that allows you to specifically convert between png and bmp - I've adapted it slightly, and will put it in the addon source code file `png2bmp.cpp` which we will take a look at shortly.  Let's first look at what the actual JavaScript program looks like though - before diving into the addon code itself:

~~~~~~~~~~{#binding-hello .javascript}
'use strict';
const fs = require('fs');
const path = require('path');
const png2bmp = require('./build/Release/png2bmp');
var png_file = process.argv[2];
var bmp_file = path.basename(png_file, '.png') + ".bmp";
var png_buffer = fs.readFileSync(png_file);

png2bmp.saveBMP(bmp_file, png_buffer, png_buffer.length);
~~~~~~~~~~

This program simply requires `fs`, `path`, and our addon, which will be located at `./build/Releast/png2bmp`.  The program grabs an input (png) filename from the command line arguments and **reads the png into a `Buffer`**.  It then sends the `Buffer` into the addon, which saves the converted BMP to the filename as specified.  Thus, the addon is converting png to BMP and saving the results to a file - returning nothing.

Here's the `package.json`, which is setting up `npm start` to invoke the `index.js` program with a command line argument of `sample.png`. It's a pretty generic image:

~~~~~~~~~~{#binding-hello .javascript}
{
  "name": "png2bmp",
  "version": "0.0.1",
  "private": true,
  "gypfile": true,
  "scripts": {
    "start": "node index.js sample.png"
  },
  "dependencies": {
  	"nan": "*"
  }
}
~~~~~~~~~~

![sample.png](imgs/sample.png)

Finally, let's take a look at the `binding.gyp` file - which is fairly standard, other than the presence of a few compiler flags needed to compile lodepng.  It also includes the requisite references to NAN.

~~~~~~~~~~{#binding-hello .javascript}
{
  "targets": [
    {
      "target_name": "png2bmp",
      "sources": [ "png2bmp.cpp", "lodepng.cpp" ], 
      "cflags": ["-Wall", "-Wextra", "-pedantic", "-ansi", "-O3"],
      "include_dirs" : ["<!(node -e \"require('nan')\")"]
    }
  ]
}
~~~~~~~~~~

`png2bmp.cpp` will mostly contain V8/NAN code, however it does have one image processing utility function - `do_convert`, adapted from lodepng's png to bmp example code.  The function accepts a `vector<unsigned char>` containing input data (png format) and a `vector<unsigned char>` to put it's output (bmp format) data into.  That function in turn calls `encodeBMP`, which is straight from the lodepng examples.  Here is the full code listing of these two functions.  The details are not important to understanding addon `Buffer` objects, but are included here for completeness.  Our addon entry point(s) will call `do_convert`. 

~~~~~~~~~~{#binding-hello .cpp}
/*
ALL LodePNG code in this file is adapted from lodepng's 
examples, found at the following URL:
https://github.com/lvandeve/lodepng/blob/
master/examples/example_bmp2png.cpp'
*/

void encodeBMP(std::vector<unsigned char>& bmp, 
  const unsigned char* image, int w, int h)
{
  //3 bytes per pixel used for both input and output.
  int inputChannels = 3;
  int outputChannels = 3;
  
  //bytes 0-13
  bmp.push_back('B'); bmp.push_back('M'); //0: bfType
  bmp.push_back(0); bmp.push_back(0); 
  bmp.push_back(0); bmp.push_back(0); 
  bmp.push_back(0); bmp.push_back(0); //6: bfReserved1
  bmp.push_back(0); bmp.push_back(0); //8: bfReserved2
  bmp.push_back(54 % 256); 
  bmp.push_back(54 / 256); 
  bmp.push_back(0); bmp.push_back(0); 

  //bytes 14-53
  bmp.push_back(40); bmp.push_back(0); 
  bmp.push_back(0); bmp.push_back(0);  //14: biSize
  bmp.push_back(w % 256); 
  bmp.push_back(w / 256); 
  bmp.push_back(0); bmp.push_back(0); //18: biWidth
  bmp.push_back(h % 256); 
  bmp.push_back(h / 256); 
  bmp.push_back(0); bmp.push_back(0); //22: biHeight
  bmp.push_back(1); bmp.push_back(0); //26: biPlanes
  bmp.push_back(outputChannels * 8); 
  bmp.push_back(0); //28: biBitCount
  bmp.push_back(0); bmp.push_back(0); 
  bmp.push_back(0); bmp.push_back(0);  //30: biCompression
  bmp.push_back(0); bmp.push_back(0); 
  bmp.push_back(0); bmp.push_back(0);  //34: biSizeImage
  bmp.push_back(0); bmp.push_back(0); 
  bmp.push_back(0); bmp.push_back(0);  //38: biXPelsPerMeter
  bmp.push_back(0); bmp.push_back(0); 
  bmp.push_back(0); bmp.push_back(0);  //42: biYPelsPerMeter
  bmp.push_back(0); bmp.push_back(0); 
  bmp.push_back(0); bmp.push_back(0);  //46: biClrUsed
  bmp.push_back(0); bmp.push_back(0); 
  bmp.push_back(0); bmp.push_back(0);  //50: biClrImportant
 
  int imagerowbytes = outputChannels * w;
  //must be multiple of 4
  imagerowbytes = imagerowbytes % 4 == 0 ? imagerowbytes : 
            imagerowbytes + (4 - imagerowbytes % 4); 
  
  for(int y = h - 1; y >= 0; y--) 
  {
    int c = 0;
    for(int x = 0; x < imagerowbytes; x++)
    {
      if(x < w * outputChannels)
      {
        int inc = c;
        //Convert RGB(A) into BGR(A)
        if(c == 0) inc = 2;
        else if(c == 2) inc = 0;
        bmp.push_back(image[inputChannels 
            * (w * y + x / outputChannels) + inc]);
      }
      else bmp.push_back(0);
      c++;
      if(c >= outputChannels) c = 0;
    }
  }

  // Fill in the size
  bmp[2] = bmp.size() % 256;
  bmp[3] = (bmp.size() / 256) % 256;
  bmp[4] = (bmp.size() / 65536) % 256;
  bmp[5] = bmp.size() / 16777216;
}


bool do_convert(
  std::vector<unsigned char> & input_data, 
  std::vector<unsigned char> & bmp)
{
  std::vector<unsigned char> image; //the raw pixels
  unsigned width, height;
  unsigned error = lodepng::decode(image, width, 
    height, input_data, LCT_RGB, 8);
  if(error) {
    std::cout << "error " << error << ": " 
              << lodepng_error_text(error) 
              << std::endl;
    return false;
  }
  encodeBMP(bmp, &image[0], width, height);
  return true;
}
~~~~~~~~~~

Sorry... that listing was long, but it's important to see what's actually going on!  Let's get to work bridging all this code to JavaScript.

## Passing buffers to an addon
Our first task is to create the `saveBMP` addon function, which accepts a filename (destination BMP) along with the png data.  The png image data is actually read when we are in JavaScript, so it's passed in as a Node.js `Buffer`.  The first rule to recognize is that `Buffer` is unknown to V8, it's a Node.js construct.  We'll use NAN to access the buffer itself (and later create new `Buffer` objects).  Whenever `Buffer` objects are passed to C++ addons, it is necessary to specify it's length as an added parameter, as it's difficult to ascertain the actual data length of a `Buffer` from C++.

Let's set up the first function call in NAN:

~~~~~~~~~~{#binding-hello .cpp}

NAN_METHOD(SaveBMP) {
    v8::String::Utf8Value val(info[0]->ToString());
    std::string outfile (*val);
    
    ....
}

NAN_MODULE_INIT(Init) {
   Nan::Set(target, 
    New<String>("saveBMP").ToLocalChecked(),
    GetFunction(New<FunctionTemplate>(SaveBMP))
      .ToLocalChecked());
}

NODE_MODULE(basic_nan, Init)
~~~~~~~~~~

In `SaveBMP`, the first step we do is a simple extraction of the output filename.  Next, we must extract the binary data (which will be represented by `unsigned char` data).

~~~~~~~~~~{#binding-hello .cpp}
unsigned char* buffer = 
  (unsigned char*) node::Buffer::Data(info[1]->ToObject());

unsigned int size = info[2]->Uint32Value();
~~~~~~~~~~

Notice just how easy this is... Node.js provides a static `Data` method on the `Buffer` class that accepts a standard `v8::Object` handle and returns an `unsigned char` pointer to the underlying data.  This pointer **does NOT** point to data managed by V8 recall - it's on the normal C++ heap and can be worked with as such.  We also extract the size from the third argument to the addon function.

Often in C++ we prefer to deal with STL containers rather than raw memory arrays, so we can easily create a `vector` from this pointer - which we need to do in order to call `do_convert`.  Below is the full code listing - which converts the buffer's data pointer to a vector, calls `do_convert` which works it's magic to fill in bmp data into the vector we give it, and finally saves it to the desired output file (using a lodepng utility call - `save_file`).

~~~~~~~~~~{#binding-hello .cpp}
NAN_METHOD(SaveBMP) {
  v8::String::Utf8Value val(info[0]->ToString());
  std::string outfile (*val);
    
  // Convert the Node.js Buffer into a C++ Vector
  unsigned char*buffer = 
    (unsigned char*) node::Buffer::Data(info[1]->ToObject());

  unsigned int size = info[2]->Uint32Value();
  std::vector<unsigned char> png_data(buffer, buffer + size);

  // Convert to bmp, stored in another vector.    
  std::vector<unsigned char> bmp; 
  if ( do_convert(png_data, bmp)) {
      info.GetReturnValue().Set(Nan::New(false)); 
  }
  else {
      lodepng::save_file(bmp, outfile);    
      info.GetReturnValue().Set(Nan::New(true));
  }
}
~~~~~~~~~~

Run this program by doing an `npm install` and then an `npm start` and you'll see a `sample.bmp` generated that looks eerily similar to `sample.png`, just a whole lot bigger (bmp compression is far less efficient than png).

## Returning buffers from addon
This addon would be a lot more flexible if we simply returned the bitmap image data, rather than needing to save it to a file while within C++.  To do this, we must learn how to return `Buffer` objects.  This concept, on the surface, seems easy enough - you can look at examples on NAN's website to see new `Buffer`s being created in C++ and returned to JavaScript.  Upon closer look though, there are some issues we must be careful with, which we'll tackle here.

Let's create a new addon entry point - `getBMP` - which would be called from JavaScript like so:

~~~~~~~~~~{#binding-hello .javascript}
...
var png_buffer = fs.readFileSync(png_file);

bmp_buffer = png2bmp.getBMP(png_buffer, png_buffer.length);
fs.writeFileSync(bmp_file, bmp_buffer);    
~~~~~~~~~~

In the original C++ function, we called `do_convert` which put the bitmap data into a `vector<unsigned int>` which we wrote to a file.  Now we must *return* that data, by constructing a new `Buffer` object.  NAN's `NewBuffer` call aptly does the trick here - let's look at a first draft of the addon function:

~~~~~~~~~~{#binding-hello .cpp}

void buffer_delete_callback(char* data, void* hint) {
    free(data);
}

NAN_METHOD(GetBMP) {
  unsigned char*buffer = 
    (unsigned char*) node::Buffer::Data(info[0]->ToObject());

  unsigned int size = info[1]->Uint32Value();
   
  std::vector<unsigned char> png_data(buffer, buffer + size);
  std::vector<unsigned char> bmp = vector<unsigned char>();
     
  if ( do_convert(png_data, bmp)) {
      info.GetReturnValue().Set(
          NewBuffer((char *)bmp.data(), 
            bmp.size(), buffer_delete_callback, 0)
            .ToLocalChecked());
    }
}  
~~~~~~~~~~

The code example above follows what most tutorials online advocate.  We call `NewBuffer` with a char * (which we grab from the `bmp` vector using the `data` method), the size of the amount of memory we are creating the buffer out of, and then 2 additional parameters that might raise your curiosity.  The 3rd parameter to `NewBuffer` is a callback - which ends up being called when the `Buffer` you are creating gets garbage collected by V8.  Recall, `Buffer`s are JavaScript objects, whose data is stored outside V8 - but the object itself is under V8's control.  From this perspective, it should make sense that a callback would be handy - when V8 destroys the buffer, we need some way of freeing up the data we have created - which is passed into the callback as it's first parameter.  The signature of the callback is defined by NAN - `Nan::FreeCallback()`.  The seconds parameter is a hint to aid in deallocation, we can use it however we want.  It will be helpful soon, but for now we just pass null (0).

So - *here is the problem* with this code:  The data contained in the buffer we return is likely deleted before our JavaScript gets to use it.  Why?  If you understand C++ well, you likely already see the problem:  the `bmp` vector is going to go out of scope as our `GetBMP` function returns.  C++ vector semantics hold that when the vector goes out of scope, the vector's destructor will delete all data within the vector - in this case, our bmp data!  This is a huge problem, since the `Buffer` we send back to JavaScript will have it's data deleted out from under it.  You might get away with this (race conditions are fun right?), but it wil eventually cause your program to crash.

How do we get around this?  One method is to create a `Buffer` containing a *copy* of the `bmp` vector's data.  We could do this like so:

~~~~~~~~~~{#binding-hello .cpp}
if ( do_convert(png_data, bmp)) {
        info.GetReturnValue().Set(
            CopyBuffer(
              (char *)bmp.data(), 
              bmp.size()).ToLocalChecked());
}
~~~~~~~~~~

This indeed is safe, but it involves creating a copy of the data - slow and memory wasting...  One way to avoid this whole mess is not to use a vector, and store the bitmap data in a dynamically allocated `char *` array - however that makes the bitmap conversion code a lot more cumbersome.  Thankfully, the answer to this problem, which allows us to still use vectors, is suggested by the the `Nan::FreeCallback` call signature - namely the `hint` parameter.  Since our problem is that the vector containing bitmap data goes out of scope, we can instead *dynamically* allocate the vector itself, and pass it into the free callback, where it can be properly deleted when the `Buffer` has been garbage collected.  Below is the completed solution - take careful note now that we are utilizing the `hint` parameter in our callback, and that we are using a dynamically allocated (heap) vector instead of a stack variable.

~~~~~~~~~~{#binding-hello .cpp}
void buffer_delete_callback(char* data, void* the_vector) {
  delete reinterpret_cast<vector<unsigned char> *> (the_vector);
}

NAN_METHOD(GetBMP) {
    
  unsigned char*buffer = 
    (unsigned char*) node::Buffer::Data(info[0]->ToObject());
  unsigned int size = info[1]->Uint32Value();
   
  std::vector<unsigned char> png_data(buffer, buffer + size);
    
  // allocate the vector on the heap because we 
  // are building a buffer out of it's data to 
  // return to Node - and don't want to allow
  // it to go out of scope until the buffer 
  // does (see buffer_delete_callback).
  
  std::vector<unsigned char> * bmp = new vector<unsigned char>();
     
  if ( do_convert(png_data, *bmp)) {
      info.GetReturnValue().Set(
          NewBuffer((char *)bmp->data(), 
            bmp->size(), buffer_delete_callback, bmp)
            .ToLocalChecked());
  }
}
~~~~~~~~~~

When you run the program, JavaScript will now safely be able to operate on the returned `Buffer` without needing to worry about `vector` deleting the memory.

## Buffers as a solution to the worker thread copying problem
Reading the segment above, you might recall a discussion in Chapter 4 regarding V8 memory and worker threads.  We had a significant issue when using asynchronous addons, in that C++ threads created to do the asynchronous work could *never* access V8 data directly.  There was no real solution to this problem, other than creating a copy of the data in C++ heap space.  For lots of addons, this if fine - however as was suggested at the time, when moving large amounts of data between JavaScript and C++ this is a real issue.  Now we have a glimpse at a possible solution - allocating data as `Buffer` objects!

Let's develop an asynchronous version of the png to bitmap converter.  We'll perform the actual conversion in a C++ worker thread, using `Nan::AsyncWorker`.  Through the use of `Buffer` objects however, we will be no need to create a copy of the png data - we will only need to hold a pointer to the underlying data such that our worker thread can access it.  Likewise, the data produced by the worker thread (the `bmp` vector can be used to create a new `Buffer` without copying data, as shown above.  Since we've worked with `AsyncWorker` a lot already in this book, I'll simply show you the code below - it's pretty straightforward:

~~~~~~~~~~{#binding-hello .cpp}
 class PngToBmpWorker : public AsyncWorker {
    public:
    PngToBmpWorker(Callback * callback, 
        v8::Local<v8::Object> &pngBuffer, int size) 
        : AsyncWorker(callback) {
        unsigned char*buffer = 
          (unsigned char*) node::Buffer::Data(pngBuffer);

        std::vector<unsigned char> tmp(
          buffer, 
          buffer +  (unsigned int) size);

        png_data = tmp;
    }
    void Execute() {
       bmp = new vector<unsigned char>();
       do_convert(png_data, *bmp);
    }
    void HandleOKCallback () {
        Local<Object> bmpData = 
               NewBuffer((char *)bmp->data(), 
               bmp->size(), buffer_delete_callback, 
               bmp).ToLocalChecked();
        Local<Value> argv[] = { bmpData };
        callback->Call(1, argv);
    }

    private:
        vector<unsigned char> png_data;
        std::vector<unsigned char> * bmp;
};

NAN_METHOD(GetBMPAsync) {
    int size = To<int>(info[1]).FromJust();
    v8::Local<v8::Object> pngBuffer = 
      info[0]->ToObject();

    Callback *callback = 
      new Callback(info[2].As<Function>());

    AsyncQueueWorker(
      new PngToBmpWorker(callback, pngBuffer , size));
}

~~~~~~~~~~

Now we've got an asynchronous function to get the bitmap encoded data too - without any copying of data unnecessarily.

~~~~~~~~~~{#binding-hello .javascript}
png2bmp.getBMPAsync(png_buffer, 
  png_buffer.length,
  function(bmp_buffer) {
    fs.writeFileSync(bmp_file, bmp_buffer);
}); 
~~~~~~~~~~