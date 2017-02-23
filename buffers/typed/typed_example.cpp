#include <nan.h>
using namespace Nan;
using namespace v8;

NAN_METHOD(increment) {
   Local<Array> array = Local<Array>::Cast(info[0]);
   double inc = Nan::To<double>(info[1]).FromJust(); 
   TypedArrayContents<double> typed(array);
   double* data = *typed;
   for (u_int32_t i = 0; i < typed.length(); i++ ) {
    data[i] += inc;
   }
}

NAN_MODULE_INIT(Init) {
   Nan::Set(target, New<String>("increment").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(increment)).ToLocalChecked());
}

NODE_MODULE(typed_example, Init)