#include <nan.h>
#include <iostream>
#include <string>
using namespace std;
using namespace Nan;
using namespace v8;

NAN_METHOD(PassNumber) {
    Nan::Maybe<double> value = Nan::To<double>(info[0]); 
    Local<Number> retval = Nan::New(value.FromJust() + 42);
    info.GetReturnValue().Set(retval);    
}

NAN_METHOD(PassInteger) {
    if ( info.Length() < 1 ) {
        return;
    }
    if ( !info[0]->IsInt32()) {
        return;
    }
    int value = info[0]->IntegerValue();
    Local<Integer> retval = Nan::New(value + 42);
    info.GetReturnValue().Set(retval); 
}

NAN_METHOD(PassBoolean) {
    if ( info.Length() < 1 ) {
        return;
    }
    if ( !info[0]->IsBoolean()) {
        return;
    }
    bool value = info[0]->BooleanValue();
    Local<Boolean> retval = Nan::New(!value);
    info.GetReturnValue().Set(retval); 
}


NAN_METHOD(PassString) {
    v8::String::Utf8Value val(info[0]->ToString());
    
    std::string str(*val, val.length());
    std::reverse(str.begin(), str.end());
    
    info.GetReturnValue().Set(Nan::New<String>(str.c_str()).ToLocalChecked()); 
}

Local<Value> make_return(const Local<Object> input ) {
    Local<String> x_prop = Nan::New<String>("x").ToLocalChecked();
    Local<String> y_prop = Nan::New<String>("y").ToLocalChecked();
    Local<String> sum_prop = Nan::New<String>("sum").ToLocalChecked();
    Local<String> product_prop = Nan::New<String>("product").ToLocalChecked();

    Local<Object> obj = Nan::New<Object>();
    double x = Nan::Get(input, x_prop).ToLocalChecked()->NumberValue();
    double y = Nan::Get(input, y_prop).ToLocalChecked()->NumberValue();
      
    Nan::Set(obj, sum_prop, Nan::New<Number>(x+y));
    Nan::Set(obj, product_prop, Nan::New<Number>(x*y));
    return obj;
}

NAN_METHOD(PassObject) {
    if ( info.Length() > 0 ) {
        Local<Object> target = info[0]->ToObject();
        Local<Value> obj = make_return(target);
        info.GetReturnValue().Set(obj);
    }
}

NAN_METHOD(IncrementArray) {
    Local<Array> array = Local<Array>::Cast(info[0]);
    
    for (unsigned int i = 0; i < array->Length(); i++ ) {
      if (Nan::Has(array, i).FromJust()) {
        double value = Nan::Get(array, i).ToLocalChecked()->NumberValue();
        Nan::Set(array, i, Nan::New<Number>(value + 1));
      }
    }

    Local<String> prop = Nan::New<String>("not_index").ToLocalChecked();
    Local<Array> a = New<v8::Array>(3);
    Nan::Set(a, 0, Nan::Get(array, 0).ToLocalChecked());
    Nan::Set(a, 1, Nan::Get(array, prop).ToLocalChecked());
    Nan::Set(a, 2, Nan::Get(array, 2).ToLocalChecked());

    info.GetReturnValue().Set(a);
}


NAN_MODULE_INIT(Init) {
   Nan::Set(target, New<String>("pass_number").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(PassNumber)).ToLocalChecked());
   Nan::Set(target, New<String>("pass_integer").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(PassInteger)).ToLocalChecked());
   Nan::Set(target, New<String>("pass_boolean").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(PassBoolean)).ToLocalChecked());
   Nan::Set(target, New<String>("pass_string").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(PassString)).ToLocalChecked());
        
   Nan::Set(target, New<String>("pass_object").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(PassObject)).ToLocalChecked());
        
   Nan::Set(target, New<String>("pass_array").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(IncrementArray)).ToLocalChecked());
}

NODE_MODULE(basic_nan, Init)