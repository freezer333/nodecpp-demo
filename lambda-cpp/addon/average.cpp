#include <node.h>

using namespace v8;

void Average(const FunctionCallbackInfo<Value>& args) {
    Isolate * isolate = args.GetIsolate();
    double sum = 0;
    int count = 0;
    
    for (int i = 0; i < args.Length(); i++){
    	if ( args[i]->IsNumber()) {
    		sum += args[i]->NumberValue();
    		count++;
    	}
    }
    
    Local<Number> retval = Number::New(isolate, sum / count);
    args.GetReturnValue().Set(retval);
}


void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "average", Average);
}

NODE_MODULE(average_addon, init)