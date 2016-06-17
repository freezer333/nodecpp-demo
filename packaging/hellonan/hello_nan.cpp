#include <nan.h>
using namespace Nan;
using namespace v8;

NAN_METHOD(Method) {
  info.GetReturnValue().Set(New<String>("nan hello").ToLocalChecked());
}

NAN_MODULE_INIT(Init) {
   Nan::Set(target, New<String>("hello").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(Method)).ToLocalChecked());
}       

NODE_MODULE(hello_nan_addon, Init)

