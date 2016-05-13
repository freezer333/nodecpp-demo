#include <nan.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include "streaming-worker.h"

using namespace std;

class Accumulate : public StreamingWorker {
  public:
    Accumulate(Callback *data
      , Callback *complete
      , Callback *error_callback, 
      v8::Local<v8::Object> & options) : StreamingWorker(data, complete, error_callback){

        sum = 0;
        filter = "";
        if (options->IsObject() ) {
          v8::Local<v8::Value> filter_ = options->Get(New<v8::String>("filter").ToLocalChecked());
          if ( filter_->IsString() ) {
            v8::String::Utf8Value s(filter_);
            filter = *s;
          }
        }
    }
    ~Accumulate(){}
    
    bool filter_by_name(string name) {
      return ( filter.empty() || name == filter);
    }

    void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
      int value ;
      do {
        Message m = fromNode.read();
        value = std::stoi(m.data);
        if ( filter_by_name(m.name) || value <= 0) {
          if ( value > 0 ){
            sum += value;
          }
          else {
            Message tosend("sum", std::to_string(sum));
            writeToNode(progress, tosend);
          }
        }
      } while (value > 0);
    }
  private:
    int sum;
    string filter;
};

StreamingWorker * create_worker(Callback *data
    , Callback *complete
    , Callback *error_callback, v8::Local<v8::Object> & options) {
 return new Accumulate(data, complete, error_callback, options);
}

NODE_MODULE(accumulate, StreamWorkerWrapper::Init)
