// hello.cpp
#include <node.h>

using namespace v8;

void Method(const FunctionCallbackInfo<Value>& args) {
  args.GetReturnValue().Set(args.Holder());
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "hello", Method);
}

NODE_MODULE(hello_addon, init)

