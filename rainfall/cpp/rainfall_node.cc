#include <node.h>
#include <v8.h>
#include <uv.h>
#include "rainfall.h"
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>

using namespace v8;

location unpack_location(Isolate * , const Handle<Object> sample_obj);
sample unpack_sample(Isolate * , const Handle<Object> );
void pack_rain_result(v8::Isolate* isolate, v8::Local<v8::Object> & target, rain_result & result);


///////////////////////////////////////////////////////////////////
// Part 4 - Callbacks
///////////////////////////////////////////////////////////////////

struct Work {
  uv_work_t  request;
  Persistent<Function> callback;

  std::vector<location> locations;
  std::vector<rain_result> results;
};

// called by libuv worker in separate thread
static void WorkAsync(uv_work_t *req)
{
    Work *work = static_cast<Work *>(req->data);

    // this is the worker thread, lets build up the results
    // allocated results from the heap because we'll need
    // to access in the event loop later to send back
    work->results.resize(work->locations.size());
    std::transform(work->locations.begin(), work->locations.end(), work->results.begin(), calc_rain_stats);


    // that wasn't really that long of an operation, so lets pretend it took longer...
    std::this_thread::sleep_for(chrono::seconds(3));
}

// called by libuv in event loop when async function completes
static void WorkAsyncComplete(uv_work_t *req,int status)
{
    Isolate * isolate = Isolate::GetCurrent();

    // Fix for Node 4.x - thanks to https://github.com/nwjs/blink/commit/ecda32d117aca108c44f38c8eb2cb2d0810dfdeb
    v8::HandleScope handleScope(isolate);

    Local<Array> result_list = Array::New(isolate);
    Work *work = static_cast<Work *>(req->data);

    // the work has been done, and now we pack the results
    // vector into a Local array on the event-thread's stack.

    for (unsigned int i = 0; i < work->results.size(); i++ ) {
      Local<Object> result = Object::New(isolate);
      pack_rain_result(isolate, result, work->results[i]);
      result_list->Set(i, result);
    }

    // set up return arguments
    Handle<Value> argv[] = { Null(isolate) , result_list };

    // execute the callback
    // https://stackoverflow.com/questions/13826803/calling-javascript-function-from-a-c-callback-in-v8/28554065#28554065
    Local<Function>::New(isolate, work->callback)->Call(isolate->GetCurrentContext()->Global(), 2, argv);

    // Free up the persistent function callback
    work->callback.Reset();
    delete work;

}

void CalculateResultsAsync(const v8::FunctionCallbackInfo<v8::Value>&args) {
    Isolate* isolate = args.GetIsolate();

    Work * work = new Work();
    work->request.data = work;

    // extract each location (its a list) and store it in the work package
    // locations is on the heap, accessible in the libuv threads
    Local<Array> input = Local<Array>::Cast(args[0]);
    unsigned int num_locations = input->Length();
    for (unsigned int i = 0; i < num_locations; i++) {
      work->locations.push_back(unpack_location(isolate, Local<Object>::Cast(input->Get(i))));
    }

    // store the callback from JS in the work package so we can
    // invoke it later
    Local<Function> callback = Local<Function>::Cast(args[1]);
    work->callback.Reset(isolate, callback);

    // kick of the worker thread
    uv_queue_work(uv_default_loop(),&work->request,WorkAsync,WorkAsyncComplete);


    args.GetReturnValue().Set(Undefined(isolate));

}

void CalculateResultsSync(const v8::FunctionCallbackInfo<v8::Value>&args) {
    Isolate* isolate = args.GetIsolate();
    std::vector<location> locations;
    std::vector<rain_result> results;
    
    // extract each location (its a list)
    Local<Array> input = Local<Array>::Cast(args[0]);
    unsigned int num_locations = input->Length();
    for (unsigned int i = 0; i < num_locations; i++) {
      locations.push_back(unpack_location(isolate, Local<Object>::Cast(input->Get(i))));
    }

    // Build vector of rain_results
    results.resize(locations.size());
    std::transform(locations.begin(), locations.end(), results.begin(), calc_rain_stats);


    // Convert the rain_results into Objects for return
    Local<Array> result_list = Array::New(isolate);
    for (unsigned int i = 0; i < results.size(); i++ ) {
      Local<Object> result = Object::New(isolate);
      pack_rain_result(isolate, result, results[i]);
      result_list->Set(i, result);
    }


    Local<Function> callback = Local<Function>::Cast(args[1]);
    Handle<Value> argv[] = { result_list };
    callback->Call(isolate->GetCurrentContext()->Global(), 1, argv);
        
    std::cerr << "Returning from C++ now" << std::endl;
    
    args.GetReturnValue().Set(Undefined(isolate));
}

///////////////////////////////////////////////////////////////////
// Part 3 - Lists and Nested Objects
///////////////////////////////////////////////////////////////////
void CalculateResults(const v8::FunctionCallbackInfo<v8::Value>&args) {
    Isolate* isolate = args.GetIsolate();
    std::vector<location> locations;
    std::vector<rain_result> results;

    // extract each location (its a list)
    Local<Array> input = Local<Array>::Cast(args[0]);
    unsigned int num_locations = input->Length();
    for (unsigned int i = 0; i < num_locations; i++) {
      locations.push_back(unpack_location(isolate, Local<Object>::Cast(input->Get(i))));
    }

    // Build vector of rain_results
    results.resize(locations.size());
    std::transform(locations.begin(), locations.end(), results.begin(), calc_rain_stats);


    // Convert the rain_results into Objects for return
    Local<Array> result_list = Array::New(isolate);
    for (unsigned int i = 0; i < results.size(); i++ ) {
      Local<Object> result = Object::New(isolate);
      pack_rain_result(isolate, result, results[i]);
      result_list->Set(i, result);
    }

    // Return the list
    args.GetReturnValue().Set(result_list);
}


///////////////////////////////////////////////////////////////////
// Part 2 - Returning objects
///////////////////////////////////////////////////////////////////
void pack_rain_result(v8::Isolate* isolate, v8::Local<v8::Object> & target, rain_result & result){
  target->Set(String::NewFromUtf8(isolate, "mean"), Number::New(isolate, result.mean));
  target->Set(String::NewFromUtf8(isolate, "median"), Number::New(isolate, result.median));
  target->Set(String::NewFromUtf8(isolate, "standard_deviation"), Number::New(isolate, result.standard_deviation));
  target->Set(String::NewFromUtf8(isolate, "n"), Integer::New(isolate, result.n));
}

void RainfallData(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  location loc = unpack_location(isolate, Handle<Object>::Cast(args[0]));
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
  s.date = std::string(*utfValue, utfValue.length());

  // Unpack the numeric rainfall amount directly from V8 value
  s.rainfall = rainfall_Value->NumberValue();
  return s;
}



location unpack_location(Isolate * isolate, const Handle<Object> location_obj) {
  location loc;

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

void AvgRainfall(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  location loc = unpack_location(isolate, Handle<Object>::Cast(args[0]));
  double avg = avg_rainfall(loc);

  Local<Number> retval = v8::Number::New(isolate, avg);
  args.GetReturnValue().Set(retval);
}



void init(Handle <Object> exports, Handle<Object> module) {
  NODE_SET_METHOD(exports, "avg_rainfall", AvgRainfall);
  NODE_SET_METHOD(exports, "data_rainfall", RainfallData);
  NODE_SET_METHOD(exports, "calculate_results", CalculateResults);
  NODE_SET_METHOD(exports, "calculate_results_sync", CalculateResultsSync);
  NODE_SET_METHOD(exports, "calculate_results_async", CalculateResultsAsync);

}

NODE_MODULE(rainfall, init)
