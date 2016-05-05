#include <nan.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include "addon-streams.h"

using namespace std;

class Accumulate : public StreamingWorker {
  public:
    Accumulate(Callback *data
      , Callback *complete
      , Callback *error_callback, 
      v8::Local<v8::Object> & options) : StreamingWorker(data, complete, error_callback){

        sum = 0;
    }
    ~Accumulate(){}
    
    void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
      int value ;
      do {
        Message m = fromNode.read();
        value = std::stoi(m.data);
        if ( value > 0  ){
          sum += value;
        }
        else {
          Message tosend("sum", std::to_string(sum));
          writeToNode(progress, tosend);
        }
      } while (value > 0);
    }
  private:
    int sum;
};

// Important:  You MUST include this function, and you cannot alter
//             the signature at all.  The base wrapper class calls this
//             to build your particular worker.  The prototype for this
//             function is defined in addon-streams.h
StreamingWorker * create_worker(Callback *data
    , Callback *complete
    , Callback *error_callback, v8::Local<v8::Object> & options) {
 return new Accumulate(data, complete, error_callback, options);
}

// Don't forget this!  You can change the name of your module, 
// but the second parameter should always be as shown.
NODE_MODULE(even_odd_worker, StreamWorkerWrapper::Init)
