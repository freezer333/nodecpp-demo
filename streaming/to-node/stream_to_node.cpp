#include <nan.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>

using namespace Nan;
using namespace std;
//using namespace v8;


// Node to init stream
// - Starts the worker thread
// - Worker thread will call the emit function passed into init
//   - For demonstration purposes, spews out a random lat/long
// Node to close stream
// - Shuts down worker thread
// - Emits the close event

// NEED TO HANDLE ERRORS TOO


class ProgressWorker : public AsyncProgressWorker {
 public:
  ProgressWorker(
      Callback *callback
    , Callback *progress
    )
    : AsyncProgressWorker(callback), progress(progress)
    {}
  ~ProgressWorker() {}

  void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
    for (int i = 0; i < 100; ++i) {
      progress.Send(reinterpret_cast<const char*>(&i), sizeof(int));
      std::this_thread::sleep_for(chrono::milliseconds(100));
    }
    
  }

  void HandleProgressCallback(const char *data, size_t size) {
    HandleScope scope;

    int value = *reinterpret_cast<int*>(const_cast<char*>(data));
    string event;
    if ( value %2 == 0) {
        event = "even_event";
    }
    else {
        event = "odd_event";
    }


    v8::Local<v8::Value> argv[] = {New<v8::String>(event.c_str()).ToLocalChecked(), 
        New<v8::Integer>(*reinterpret_cast<int*>(const_cast<char*>(data)))
    };
    progress->Call(2, argv);
  }

 private:
  Callback *progress;
};

NAN_METHOD(Start) {
  Callback *progress = new Callback(info[1].As<v8::Function>());
  Callback *callback = new Callback(info[2].As<v8::Function>());
  AsyncQueueWorker(new ProgressWorker(
      callback
    , progress));
}

NAN_METHOD(Stop) {
  cout << "Stopping." << endl;
}

NAN_MODULE_INIT(Init) {
  Set(target
    , New<v8::String>("start").ToLocalChecked()
    , New<v8::FunctionTemplate>(Start)->GetFunction());
  Set(target
    , New<v8::String>("stop").ToLocalChecked()
    , New<v8::FunctionTemplate>(Stop)->GetFunction());
}

NODE_MODULE(stream_to_node, Init)

