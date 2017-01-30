#include <nan.h>
using namespace Nan;
using namespace v8;

#include "native-rt.h"

NAN_METHOD(now) {
    double time_now = native_now();
    Local<Number> retval = Nan::New(time_now);
    info.GetReturnValue().Set(retval);    
}

NAN_MODULE_INIT(Init) {
   Nan::Set(target, New<String>("now").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(now)).ToLocalChecked());
}

NODE_MODULE(native_rt, Init)