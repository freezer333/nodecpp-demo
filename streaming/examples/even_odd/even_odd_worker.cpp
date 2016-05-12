#include <nan.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include "streaming-worker.h"

using namespace std;

class EvenOdd : public StreamingWorker {
  public:
    EvenOdd(Callback *data
      , Callback *complete
      , Callback *error_callback, 
      v8::Local<v8::Object> & options) : StreamingWorker(data, complete, error_callback){
        
      start = 0;
      if (options->IsObject() ) {
        v8::Local<v8::Value> start_ = options->Get(New<v8::String>("start").ToLocalChecked());
        if ( start_->IsNumber() ) {
          start = start_->NumberValue();
        }
      }
    }
    ~EvenOdd(){}
    
    void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
      int max ;
      do {
        Message m = fromNode.read();
        max = std::stoi(m.data);
        for (int i = start; i <= max; ++i) {
          string event = (i % 2 == 0 ? "even_event" : "odd_event");
          Message tosend(event, std::to_string(i));
          writeToNode(progress, tosend);
          std::this_thread::sleep_for(chrono::milliseconds(100));
        }
      } while (max >= 0);
    }
  private:
    int start;
};

// Important:  You MUST include this function, and you cannot alter
//             the signature at all.  The base wrapper class calls this
//             to build your particular worker.  The prototype for this
//             function is defined in addon-streams.h
StreamingWorker * create_worker(Callback *data
    , Callback *complete
    , Callback *error_callback, v8::Local<v8::Object> & options) {
 return new EvenOdd(data, complete, error_callback, options);
}

// Don't forget this!  You can change the name of your module, 
// but the second parameter should always be as shown.
NODE_MODULE(even_odd_worker, StreamWorkerWrapper::Init)
