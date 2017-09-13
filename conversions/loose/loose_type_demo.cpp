#include <node.h>
#include <string>
#include <algorithm>
#include <iostream>

using namespace v8;

/* Demonstrates converstions of v8:Value's passed as arguments.
   See https://v8docs.nodesource.com/node-0.8/dc/d0a/classv8_1_1_value.html
   for more types and conversion methods
*/
Local<Value> make_return(Isolate * isolate, const Local<Object> input ) ;


void PassNumber(const FunctionCallbackInfo<Value>& args) {
    Isolate * isolate = args.GetIsolate();

    double value = args[0]->NumberValue();

    Local<Number> retval = Number::New(isolate, value + 42);
    args.GetReturnValue().Set(retval);
}

void PassInteger(const FunctionCallbackInfo<Value>& args) {
    Isolate * isolate = args.GetIsolate();

    int value = args[0]->Int32Value();

    Local<Number> retval = Int32::New(isolate, value + 42);
    args.GetReturnValue().Set(retval);
}

void PassBoolean(const FunctionCallbackInfo<Value>& args) {
    Isolate * isolate = args.GetIsolate();
    Local<Boolean> target = args[0]->ToBoolean();
    bool value = target->BooleanValue();

    Local<Boolean> retval = Boolean::New(isolate, !value);
    args.GetReturnValue().Set(retval);
}

void PassString(const FunctionCallbackInfo<Value>& args) {
    Isolate * isolate = args.GetIsolate();
    
    v8::String::Utf8Value s(args[0]);
    std::string str(*s, s.length());
    std::reverse(str.begin(), str.end());    
    
    Local<String> retval = String::NewFromUtf8(isolate, str.c_str());
    args.GetReturnValue().Set(retval);
}

void PassObject(const FunctionCallbackInfo<Value>& args) {
    Isolate * isolate = args.GetIsolate();
    Local<Object> target = args[0]->ToObject();
    
    Local<Value> obj = make_return(isolate, target);

    args.GetReturnValue().Set(obj);
}

void PassArray(const FunctionCallbackInfo<Value>& args) {
    Isolate * isolate = args.GetIsolate();
    Local<Array> array = Local<Array>::Cast(args[0]);
    
    for (unsigned int i = 0; i < array->Length(); i++ ) {
      if (array->Has(i)) {
        double value = array->Get(i)->NumberValue();
        array->Set(i, Number::New(isolate, value + 1));
      }
    }

    Local<String> prop = String::NewFromUtf8(isolate, "not_index");
    Local<Array> a = Array::New(isolate);
    a->Set(0, array->Get(0));
    a->Set(1, array->Get(prop));
    a->Set(2, array->Get(2));

    args.GetReturnValue().Set(a);
}



Local<Value> make_return(Isolate * isolate, const Local<Object> input ) {
    Local<String> x_prop = String::NewFromUtf8(isolate, "x");
    Local<String> y_prop = String::NewFromUtf8(isolate, "y");
    Local<String> sum_prop = String::NewFromUtf8(isolate, "sum");
    Local<String> product_prop = String::NewFromUtf8(isolate, "product");

    double x = input->Get(x_prop)->NumberValue();
    double y = input->Get(y_prop)->NumberValue();

    Local<Object> obj = Object::New(isolate);
    obj->Set(sum_prop, Number::New(isolate, x + y));
    obj->Set(product_prop, Number::New(isolate, x * y));
    return obj;
}




void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "pass_number", PassNumber);
  NODE_SET_METHOD(exports, "pass_integer", PassInteger);
  NODE_SET_METHOD(exports, "pass_string", PassString);
  NODE_SET_METHOD(exports, "pass_boolean", PassBoolean);
  NODE_SET_METHOD(exports, "pass_object", PassObject);
  NODE_SET_METHOD(exports, "pass_array", PassArray);
}

NODE_MODULE(loose_type_demo, init)

