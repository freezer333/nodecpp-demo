# nodecpp-demo

This repository contains full code listings that help guide you through developing Node.js addons.

The content is fully explained on my blog - [blog.scottfrees.com](http://blog.scottfrees.com) and is the subject of my ebook - [C++ and Node.js Integration](http://scottfrees.com/ebooks/nodecpp).

## Basics
Some simple "hello world" type programs, discussed in the first chapter of the [ebook](http://scottfrees.com/ebooks/nodecpp).

## Conversions
Code "cheat sheet" for passing different data types between C++ and JavaScript.  See [here](http://blog.scottfrees.com/type-conversions-from-javascript-to-c-in-v8) for associated blog post.  This is also covered in Chapter 2 of the [ebook](http://scottfrees.com/ebooks/nodecpp).

## Conversions_NAN
The same "cheat sheet" as Conversions, but using the NAN API rather than native V8.  Covered in Chapter 6 of the [ebook](http://scottfrees.com/ebooks/nodecpp).

## Rainfall
Contains a full example of using objects, arrays, and asynchronous callbacks to exchange data between JavaScript and C++ library for doing some (trivial) number crunching.  This supports a 4-part blog series, which starts [here](http://blog.scottfrees.com/c-processing-from-node-js).  Covered in Chapters 3 and 4 of the [ebook](http://scottfrees.com/ebooks/nodecpp).

## Primes
A set of example addon functions demonstrating NAN's AsyncWorker and AsyncProgressWorker.  Covered in Chapter 6 of the [ebook](http://scottfrees.com/ebooks/nodecpp).

## ObjectWrap
An `ObjectWrap` example developing a C++ polynomial class.  Covered in Chapter 5 of the [ebook](http://scottfrees.com/ebooks/nodecpp).

## ObjectWrap_NAN
The same Polynomial class, but using NAN instead of direct Node.js API.  Covered in Chapter 6 of the [ebook](http://scottfrees.com/ebooks/nodecpp).

## Streaming
A complete SDK and set of examples for using NAN and asynchronous callbacks to stream data between C++ and JavaScript.  This is the subject of Chapter 7 of the [ebook](http://scottfrees.com/ebooks/nodecpp).

## Buffers
An example of using buffers to move data between JavaScript and a simple image processing C++ addon (converting PNG to BMP using [LodePNG](http://lodev.org/lodepng/)).  This is covered in Appendix B of the [ebook](http://scottfrees.com/ebooks/nodecpp).

## Packaging
A set of examples for building and distributing reusable addons.  Covered in Chapter 8 of the [ebook](http://scottfrees.com/ebooks/nodecpp).
