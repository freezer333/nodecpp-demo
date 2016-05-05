#include <iostream>
#include <chrono>
#include <thread>
#include "addon-streams.h"
#include "json.hpp"  //https://github.com/nlohmann/json

using namespace std;
using json = nlohmann::json;


class Sensor : public StreamingWorker {
  public:
    Sensor(Callback *data, Callback *complete, Callback *error_callback,  v8::Local<v8::Object> & options) 
          : StreamingWorker(data, complete, error_callback){

      name = "default sensor";
      if (options->IsObject() ) {
        v8::Local<v8::Value> name_ = options->Get(New<v8::String>("name").ToLocalChecked());
        if ( name_->IsString() ) {
          v8::String::Utf8Value s(name_);
          name = *s;
        }
      }
    }
     
    void send_sample(const AsyncProgressWorker::ExecutionProgress& progress, double x, double y, double z) {
        json sample;
        sample["sensor"] = name;
        sample["position"]["x"] = x;
        sample["position"]["y"] = x;
        sample["position"]["z"] = x;
        Message tosend("position_sample", sample.dump());
        writeToNode(progress, tosend);
    }

    void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
      std::random_device rd;
      std::uniform_real_distribution<double> pos_dist(-1.0, 1.0);
      std::uniform_int_distribution<int> sample_dist(50, 200);
      int sample_rate = sample_dist(rd);
      while (!closed()) {
        send_sample(progress, pos_dist(rd), pos_dist(rd), pos_dist(rd));
        std::this_thread::sleep_for(chrono::milliseconds(sample_rate));
      }
    }
  private:
    string name;
};

// Important:  You MUST include this function, and you cannot alter
//             the signature at all.  The base wrapper class calls this
//             to build your particular worker.  The prototype for this
//             function is defined in addon-streams.h
StreamingWorker * create_worker(Callback *data
    , Callback *complete
    , Callback *error_callback, v8::Local<v8::Object> & options) {
 return new Sensor(data, complete, error_callback, options);
}

// Don't forget this!  You can change the name of your module, 
// but the second parameter should always be as shown.
NODE_MODULE(factorization, StreamWorkerWrapper::Init)
