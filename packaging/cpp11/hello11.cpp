#include <nan.h>
using namespace Nan;
using namespace v8;

NAN_METHOD(Add) {
    auto sum = [](int x, int y) { return x + y; };
    
    int a = To<int>(info[0]).FromJust();
    int b = To<int>(info[1]).FromJust();
    
    Local<Number> retval = Nan::New(sum(a, b));
    info.GetReturnValue().Set(retval);    
}

NAN_MODULE_INIT(Init) {
   Nan::Set(target, New<String>("add").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(Add)).ToLocalChecked());
}       

NODE_MODULE(cpp11, Init)