#include <iostream>
#include <chrono>
#include <thread>
#include "streaming-worker.h"

using namespace std;

class Factorization : public StreamingWorker {
  public:
    Factorization(Callback *data, Callback *complete, Callback *error_callback,  v8::Local<v8::Object> & options) 
          : StreamingWorker(data, complete, error_callback){

        N = -1;
        if (options->IsObject() ) {
          v8::Local<v8::Value> n_ = options->Get(New<v8::String>("n").ToLocalChecked());
          if ( n_->IsNumber() ) {
            N = n_->NumberValue();
          }
        }

        if ( N < 0 ) {
          SetErrorMessage("Cannot compute prime factorization of negative numbers (overflowed long long?)!");
        }
    }
     
    void send_factor(const AsyncProgressWorker::ExecutionProgress& progress, long long factor) {
        Message tosend("factor", std::to_string(factor));
        writeToNode(progress, tosend);
    }

    void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
      long long n = N;
      while (n%2 == 0)
      {
        send_factor(progress, 2);
        n = n/2;
      }
 
      for (long long i = 3; i <= n; i = i+2) {
          while (n%i == 0) {
            send_factor(progress, i);
            n = n/i;
          }
      }
    }
  private:
    long long N;
};

// Important:  You MUST include this function, and you cannot alter
//             the signature at all.  The base wrapper class calls this
//             to build your particular worker.  The prototype for this
//             function is defined in addon-streams.h
StreamingWorker * create_worker(Callback *data
    , Callback *complete
    , Callback *error_callback, v8::Local<v8::Object> & options) {
 return new Factorization(data, complete, error_callback, options);
}

// Don't forget this!  You can change the name of your module, 
// but the second parameter should always be as shown.
NODE_MODULE(factorization, StreamWorkerWrapper::Init)
