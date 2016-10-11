#include <nan.h>
using namespace Nan;
using namespace v8;

NAN_METHOD(rotate) {
    
    unsigned char*buffer = (unsigned char*) node::Buffer::Data(info[0]->ToObject());
    unsigned int size = info[1]->Uint32Value();
   
    for(int i = 0; i < size; i++ ) {
        buffer[i] += 13;
    }   
}

NAN_MODULE_INIT(Init) {
         
   Nan::Set(target, New<String>("rotate").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(rotate)).ToLocalChecked());
}

NODE_MODULE(buffer_example, Init)