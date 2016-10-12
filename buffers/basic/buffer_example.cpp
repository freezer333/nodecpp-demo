#include <nan.h>
using namespace Nan;
using namespace v8;

NAN_METHOD(rotate) {
    char* buffer = (char*) node::Buffer::Data(info[0]->ToObject());
    unsigned int size = info[1]->Uint32Value();
    unsigned int rot = info[2]->Uint32Value();
   
    char * retval = new char[size];
    for(unsigned int i = 0; i < size; i++ ) {
        retval[i] = buffer[i] - rot;
        buffer[i] += rot;
    }   
    
    info.GetReturnValue().Set(Nan::NewBuffer(retval, size).ToLocalChecked());
}

NAN_MODULE_INIT(Init) {
   Nan::Set(target, New<String>("rotate").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(rotate)).ToLocalChecked());
}

NODE_MODULE(buffer_example, Init)