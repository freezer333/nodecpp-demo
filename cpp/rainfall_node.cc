#include <node.h>
#include <v8.h>
#include "rainfall.h"
#include <string>
#include <iostream>

using namespace v8;

location unpack_location(Isolate * , const v8::FunctionCallbackInfo<v8::Value>& );
sample unpack_sample(Isolate * , const Handle<Object> );


///////////////////////////////////////////////////////////////////
// Part 2 - Returning objects
///////////////////////////////////////////////////////////////////
void pack_rain_result(v8::Isolate* isolate, v8::Local<v8::Object> & target, rain_result & result){
  target->Set(String::NewFromUtf8(isolate, "mean"), Number::New(isolate, result.mean));
  target->Set(String::NewFromUtf8(isolate, "median"), Number::New(isolate, result.median));
  target->Set(String::NewFromUtf8(isolate, "standard_deviation"), Number::New(isolate, result.standard_deviation));
  target->Set(String::NewFromUtf8(isolate, "n"), Integer::New(isolate, result.n));
}

void data_rainfall(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  
  location loc = unpack_location(isolate, args);
  rain_result result = calc_rain_stats(loc);

  Local<Object> obj = Object::New(isolate);
  pack_rain_result(isolate, obj, result);

  args.GetReturnValue().Set(obj);
}


///////////////////////////////////////////////////////////////////
// Part 1 - Receiving JSON objects
///////////////////////////////////////////////////////////////////

sample unpack_sample(Isolate * isolate, const Handle<Object> sample_obj) {
  sample s;
  Handle<Value> date_Value = sample_obj->Get(String::NewFromUtf8(isolate, "date"));
  Handle<Value> rainfall_Value = sample_obj->Get(String::NewFromUtf8(isolate, "rainfall"));

  v8::String::Utf8Value utfValue(date_Value);
  s.date = std::string(*utfValue);
 
  // Unpack the numeric rainfall amount directly from V8 value
  s.rainfall = rainfall_Value->NumberValue();
  return s;
}



location unpack_location(Isolate * isolate, const v8::FunctionCallbackInfo<v8::Value>& args) {
  location loc;
  Handle<Object> location_obj = Handle<Object>::Cast(args[0]);
  Handle<Value> lat_Value = location_obj->Get(String::NewFromUtf8(isolate,"latitude"));
  Handle<Value> lon_Value = location_obj->Get(String::NewFromUtf8(isolate,"longitude"));
  loc.latitude = lat_Value->NumberValue();
  loc.longitude = lon_Value->NumberValue();

  Handle<Array> array = Handle<Array>::Cast(location_obj->Get(String::NewFromUtf8(isolate,"samples")));
  int sample_count = array->Length();
  for ( int i = 0; i < sample_count; i++ ) {
    sample s = unpack_sample(isolate, Handle<Object>::Cast(array->Get(i)));
    loc.samples.push_back(s);
  }
  return loc;
}

void avg_rainfall(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  
  location loc = unpack_location(isolate, args);
  double avg = avg_rainfall(loc);

  Local<Number> retval = v8::Number::New(isolate, avg);
  args.GetReturnValue().Set(retval);
}



void init(Handle <Object> exports, Handle<Object> module) {
  NODE_SET_METHOD(exports, "avg_rainfall", avg_rainfall);
  NODE_SET_METHOD(exports, "data_rainfall", data_rainfall);
}

NODE_MODULE(rainfall, init)
