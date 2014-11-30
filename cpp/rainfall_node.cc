#include <node.h>
#include <v8.h>
#include "rainfall.h"
#include <string>
#include <iostream>

using namespace v8;

void extract_sample(sample &sample, const Handle<Object> sample_obj) {
  Handle<Value> date_Value = sample_obj->Get(String::New("date"));
  Handle<Value> rainfall_Value = sample_obj->Get(String::New("rainfall"));

  // Translate the V8 string to a regularl C++ string
  v8::String::AsciiValue av(date_Value);
  sample.date = std::string(*av);

  // Unpack the numeric rainfall amount directly from V8 value
  sample.rainfall = rainfall_Value->NumberValue();
}

void extract_location(location & loc, const Arguments & args) {
  Handle<Object> location_obj = Handle<Object>::Cast(args[0]);
  Handle<Value> lat_Value = location_obj->Get(String::New("latitude"));
  Handle<Value> lon_Value = location_obj->Get(String::New("longitude"));
  loc.latitude = lat_Value->NumberValue();
  loc.longitude = lon_Value->NumberValue();

  Handle<Array> array = Handle<Array>::Cast(location_obj->Get(String::New("samples")));
  int sample_count = array->Length();
  for ( int i = 0; i < sample_count; i++ ) {
    sample s;
    extract_sample(s, Handle<Object>::Cast(array->Get(i)));
    loc.samples.push_back(s);
  }
}

Handle<Value> AvgRainfall(const Arguments & args) {
  HandleScope scope;
  location loc;
  extract_location(loc, args);
  double avg = avg_rainfall(loc);

  Local<Number> retval = v8::Number::New(avg);
  return scope.Close(retval);
}

void init(Handle<Object> target) {
  target->Set(String::NewSymbol("avg_rainfall"),
      FunctionTemplate::New(AvgRainfall)->GetFunction());
}
NODE_MODULE(rainfall, init)
