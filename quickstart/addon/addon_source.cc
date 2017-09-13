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
    if ( info.Length() < 1 ) {
        return;
    }
    if ( !info[0]->IsString()) {
        return;
    }
    v8::String::Utf8Value val(info[0]->ToString());
    
    std::string str (*val, val.length());
    std::reverse(str.begin(), str.end());
    
    info.GetReturnValue().Set(Nan::New<String>(str.c_str()).ToLocalChecked()); 
}

NAN_METHOD(PassObject) {
    if ( info.Length() > 0 ) {
        Local<Object> input = info[0]->ToObject();
        
        // Make property names to access the input object
        Local<String> x_prop = Nan::New<String>("x").ToLocalChecked();
        Local<String> y_prop = Nan::New<String>("y").ToLocalChecked();
        Local<String> sum_prop = Nan::New<String>("sum").ToLocalChecked();
        Local<String> product_prop = Nan::New<String>("product").ToLocalChecked();

        // create the return object
        Local<Object> retval = Nan::New<Object>();
        
        // pull x and y out of the input.  We'll get NaN if these weren't set, 
        // or if x / y aren't able to be converted to numbers.
        double x = Nan::Get(input, x_prop).ToLocalChecked()->NumberValue();
        double y = Nan::Get(input, y_prop).ToLocalChecked()->NumberValue();
      
        // set the properties on the return object
        Nan::Set(retval, sum_prop, Nan::New<Number>(x+y));
        Nan::Set(retval, product_prop, Nan::New<Number>(x*y));
        
        info.GetReturnValue().Set(retval);
    }
}

// Increment each value in the array parameter, 
// Return a new array with the squares of the original
// array and a 'sum_of_squares' property.
NAN_METHOD(IncrementArray) {
    Local<Array> array = Local<Array>::Cast(info[0]);
    
    Local<String> ss_prop = Nan::New<String>("sum_of_squares").ToLocalChecked();
    Local<Array> squares = New<v8::Array>(array->Length());
    double ss = 0;

    for (unsigned int i = 0; i < array->Length(); i++ ) {
      if (Nan::Has(array, i).FromJust()) {
        // get data from a particular index
        double value = Nan::Get(array, i).ToLocalChecked()->NumberValue();
        
        // set a particular index - note the array parameter
        // is mutable
        Nan::Set(array, i, Nan::New<Number>(value + 1));
        Nan::Set(squares, i, Nan::New<Number>(value * value));
        ss += value*value;
      }
    }
    // set a non index property on the returned array.
    Nan::Set(squares, ss_prop, Nan::New<Number>(ss));
    info.GetReturnValue().Set(squares);
}

NAN_METHOD(AddArray) {
    if ( info.Length() > 0 ) {
        Local<Object> input = info[0]->ToObject();
        Local<String> a_prop = Nan::New<String>("a").ToLocalChecked();
        Local<String> b_prop = Nan::New<String>("b").ToLocalChecked();
        
        double a = Nan::Get(input, a_prop).ToLocalChecked()->NumberValue();
        Local<Array> b = Local<Array>::Cast(Nan::Get(input, b_prop).ToLocalChecked());
        
        for (unsigned int i = 0; i < b->Length(); i++ ) {
            if (Nan::Has(b, i).FromJust()) {
                // get data from a particular index
                double value = Nan::Get(b, i).ToLocalChecked()->NumberValue();
                
                // set a particular index - note the array parameter
                // is mutable
                Nan::Set(b, i, Nan::New<Number>(value + a));
            }
        }
    }
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
   
    Nan::Set(target, New<String>("add_array").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(AddArray)).ToLocalChecked());
}

NODE_MODULE(my_addon, Init)