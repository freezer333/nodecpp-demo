#include <node.h>
using namespace v8;


void Add(const FunctionCallbackInfo<Value> & args) {
	Isolate * isolate = args.GetIsolate();

	if (args.Length() < 2) {
        return;
    }
    
	// value is created on the C++ runtime stack, not as part of 
	// the JavaScript execution context...
	double value= args[0]->NumberValue() + args[1]->NumberValue();

	// Now we create it in the JS execution context so we can return it
	Local<Number> num = Number::New(isolate, value);
	args.GetReturnValue().Set(num);
}

// Called when addon is require'd from JS
void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "add", Add);  // we'll create Add in a moment...
}

// The addon's name is cmath, this tells V8 what to call when it's require'd
NODE_MODULE(cmath, Init)