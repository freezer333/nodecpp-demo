#include <node.h>
using namespace v8;


void CallThis(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Function> cb = Local<Function>::Cast(args[0]);
  cb->Call(Null(isolate), 0, nullptr);
}
void CallThisWithThis(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Function> cb = Local<Function>::Cast(args[0]);
  Local<Value> argv[1] = {args[1]};
  cb->Call(Null(isolate), 1, argv);
}


// Called when addon is require'd from JS
void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "callthis", CallThis);  // we'll create Add in a moment...
    NODE_SET_METHOD(exports, "callthis_withthis", CallThisWithThis); 
}

// The addon's name is cmath, this tells V8 what to call when it's require'd
NODE_MODULE(callback, Init)