// hello.cpp
#include <node.h>

using namespace v8;

void Method(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<String>  retval = String::NewFromUtf8(isolate, "world");
  args.GetReturnValue().Set(retval);
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "hello", Method);
}

NODE_MODULE(hello_addon, init)

