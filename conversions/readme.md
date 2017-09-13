# Type conversions from JavaScript to C++ in V8
Learning how to pass information from JavaScript to C++ addons can be tricky.  Confusion stems from the extreme differences between the JavaScript and C++ type system.  While C++ is strongly typed ("42" is not an integer, but a string, and only a string!), JavaScript is very eager to convert datatypes for us.  

JavaScript primitives include Strings, Numbers, and Booleans, along with others like `null` and `undefined`.  V8 uses an inheritance hierarchy where `Primitive` extends `Value`, and all the individual primitives subclass `Primitive`.  In addition to the standard JavaScript primitives, V8 also supports integers (`Int32` and `Uint32`).  You can see all the rest of the Value types [here](https://v8docs.nodesource.com/io.js-3.0/dc/d0a/classv8_1_1_value.html).

In this post, I'll show you some basics for working with numerics, and I'll point you to a **cheat sheet** I've used to learn how to do the [rest of the conversions](https://github.com/freezer333/nodecpp-demo/tree/master/conversions).  I also cover this topic in a lot more detail in my upcoming ebook on [Node.js C++ Integration](http://scottfrees.com/ebooks/nodecpp/).  

All references to JavaScript values are held in C++ through a Handle object - in most cases a `Local`.  The handles point to V8 storage cells within JavaScript runtime.  You can learn a lot more about storage cells in my previous post on [memory issues in V8](http://blog.scottfrees.com/how-not-to-access-node-js-from-c-worker-threads).

As you go through the API for working with primitives, you'll notice there is no assignment of `Local` objects - which at first may seem odd!  It makes a lot of sense, however, for three reasons:

1. JavaScript primitives are *immutable*. Variables "containing" primitives are just pointing to unchanging V8 storage cells.  Assignment of a `var x = 5;` makes `x` point to a storage cell with 5 in it - reassigning `x = 6` does not change this storage cell. Instead, it simply makes `x` point to another storage cell that contains 6.  If `x` and `y` are both assigned the value of `10`, they both point to the same storage cell.  
2. Function calls are pass-by-value. Whenever JavaScript calls a C++ addon with a primitive parameter, the addon always receives a distinct *copy*. This means that changing its value in C++ has no visible effect to the caller.
3. Handles (`Local<Value>`) are references to *storage cells*.  Thus, given #1 above, it doesn't make sense to allow handle values to change - since primitives don't change!

Hopefully that makes some sense. However it's still likely you'll need to *modify* V8 variables. We'll just need to do this by *creating* new ones and assigning the new value to them.

## Number Example
Now let's look at the Number primitive type and what happens when we construct a C++ addon to accept them from JavaScript.  I've built an addon that exposes the following C++ function as an export called `pass_number`:

```c++
void PassNumber(const FunctionCallbackInfo<Value>& args) {
    Isolate * isolate = args.GetIsolate();

    double value = args[0]->NumberValue();

    value += 42;

    Local<Number> retval = Number::New(isolate, value);

    args.GetReturnValue().Set(retval);
}
```

The complete addon code is [here](https://github.com/freezer333/nodecpp-demo/blob/master/conversions/loose/loose_type_demo.cpp).

The addon does virtually nothing to ensure that the argument passed into the function is a Number, or if it even exists!  Here's the associated mocha tests - we can see how V8 handles numbers, and more importantly, other input that can or cannot be converted to numbers in JavaScript.

```javascript

describe('pass_number()', function () {
  it('return input + 42 when given a valid number', function () {
    assert.equal(23+42, loose.pass_number(23));
    assert.equal(0.5+42, loose.pass_number(0.5));
  });

  it('return input + 42 when given numeric as a string', function () {
    assert.equal(23+42, loose.pass_number('23'));
    assert.equal(0.5+42, loose.pass_number('0.5'));
  });

  it('return 42 when given null (null converts to 0)', function () {
    assert.equal(42, loose.pass_number(null));
  });

  it('return NaN when given undefined', function () {
    assert(isNaN(loose.pass_number()));
    assert(isNaN(loose.pass_number(undefined)));
  });

  it('return NaN when given a non-number string', function () {
    assert(isNaN(loose.pass_number('this is not a number')));
  });

```


## Complete type conversion cheat sheet
I've created a [repository](https://github.com/freezer333/nodecpp-demo/tree/master/conversions) which has a type conversion cheat sheet that I think is pretty useful.  To get it:

```
> git clone https://github.com/freezer333/nodecpp-demo.git
```

To build both addons, go into the `loose` and `strict` directories and issue a `node-gyp configure build` command in each.  You'll need to have installed `node-gyp` globally first.  If you're completely new to this, [check this out](http://blog.scottfrees.com/c-processing-from-node-js).

```
> cd nodecpp-demo/conversion/loose
> node-gyp configure build
...
> cd ../strict
> node-gyp configure build
```

The two addons (loose and strict) expose a series of functions that accept different types - Numbers, Integers, Strings, Booleans, Objects, and Arrays - and perform (somewhat silly) operations on them before returning a value.  I've included a JavaScript test program that shows you the expected outputs of each function - but the real learning value is in the addons' C++ code ([strict](https://github.com/freezer333/nodecpp-demo/blob/master/conversions/loose/loose_type_demo.cpp)/[loose](https://github.com/freezer333/nodecpp-demo/blob/master/conversions/strict/strict_type_demo.cpp)).

To run the tests (you'll need `mocha` installed), go into the `conversions` directory (with `index.js`):

```
> npm test
```

The 'loose' addon has very little type checking. It's basically mimicking how a pure JavaScript function would work.  For example, the `pass_string` function accepts anything that could be converted to a string in JavaScript and returns the reverse of it:

```javascript

describe('pass_string()', function () {
  var str = "The truth is out there";

  it('reverse a proper string', function () {
    assert.equal(reverse(str), loose.pass_string(str));
  });
  it('reverse a numeric/boolean since numbers are turned into strings', function () {
    assert.equal('24', loose.pass_string(42));
    assert.equal('eurt', loose.pass_string(true));
  });
  it('return 'llun' when given null - null is turned into 'null'', function () {
    assert.equal('llun', loose.pass_string(null));
  });
  it('return 'denifednu' when given undefined', function () {
    assert.equal(reverse('undefined'), loose.pass_string(undefined));
  });
  it('return reverse of object serialized to string', function () {
    assert.equal(reverse('[object Object]'), loose.pass_string({x: 5}));
  });
  it('return reverse of array serialized to string', function () {
    assert.equal(reverse('9,0'), loose.pass_string([9, 0]));
  });
});
```

Here's the C++ code for the loose conversions of string input:

```c++
void PassString(const FunctionCallbackInfo<Value>& args) {
    Isolate * isolate = args.GetIsolate();

    v8::String::Utf8Value s(args[0]);
    std::string str(*s, s.length());
    std::reverse(str.begin(), str.end());    

    Local<String> retval = String::NewFromUtf8(isolate, str.c_str());
    args.GetReturnValue().Set(retval);
}
```

The 'strict' addon performs full type and error checking, behaving more like a C++ function than a JavaScript function. For all of the strict addon methods, `undefined` is always returned if the input to the function isn't *exactly* what was expected. For example, the pass_string function behaves quite differently than the loose interpretation:

```javascript
describe('pass_string()', function () {

  it('return reverse a proper string', function () {
    var str = 'The truth is out there';
    it('reverse a proper string', function () {
      assert.equal(reverse(str), strict.pass_string(str));
    });
  });

  it('return undefined for non-strings', function () {
    assert.equal(undefined, strict.pass_string(42));
    assert.equal(undefined, strict.pass_string(true));
    assert.equal(undefined, strict.pass_string(null));
    assert.equal(undefined, strict.pass_string(undefined));
    assert.equal(undefined, strict.pass_string({x: 5}));
    assert.equal(undefined, strict.pass_string([9, 0]));
  });
});
```

```c++
void PassString(const FunctionCallbackInfo<Value>& args) {
    Isolate * isolate = args.GetIsolate();

    if ( args.Length() < 1 ) {
        return;
    }
    else if ( args[0]->IsNull() ) {
        return;
    }
    else if ( args[0]->IsUndefined() ) {
        return;
    }
    else if (!args[0]->IsString()) {
        // This clause would catch IsNull and IsUndefined too...
        return ;
    }

    v8::String::Utf8Value s(args[0]);
    std::string str(*s, s.length());
    std::reverse(str.begin(), str.end());    

    Local<String> retval = String::NewFromUtf8(isolate, str.c_str());
    args.GetReturnValue().Set(retval);
}
```

Go [ahead and download](https://github.com/freezer333/nodecpp-demo) the complete source and take a look.  The code is in the `/conversions` directory.  You'll see examples using Integers, Booleans, Objects, and Arrays.

## Looking for more info?
You'll encounter similar code when using NAN, so reviewing the NAN project's writeups should shed some light on the topic.  However, this post is actually a small excerpt from my recent book, which covers these and related issues in much gorier detail.  If you are interested, click [here](http://scottfrees.com/ebooks/nodecpp/) for the table of contents and info on how to get your copy. 
