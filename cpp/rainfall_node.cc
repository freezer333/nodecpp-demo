#include <node.h>
#include <v8.h>
#include "rainfall.h"

using namespace v8;

Handle<Value> AvgRainfall(const Arguments & args) {
  HandleScope scope;
  Local<Number> retval = v8::Number::New(0);
  return scope.Close(retval);
}

void init(Handle<Object> target) {
  target->Set(String::NewSymbol("avg_rainfall"),
      FunctionTemplate::New(AvgRainfall)->GetFunction());
}
NODE_MODULE(rainfall, init)
