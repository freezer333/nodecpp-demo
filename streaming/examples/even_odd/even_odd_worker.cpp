#include <nan.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include "addon-streams.h"

using namespace std;

/*

NEXT STEP:  Support options.
            
*/



///////////////////////////////
// User
//////////////////////////////
class EvenOdd : public StreamingWorker {
  public:
    EvenOdd(Callback *data
    , Callback *complete
    , Callback *error_callback) : StreamingWorker(data, complete, error_callback){

  }
  ~EvenOdd(){}
  
  void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
    int max ;
    do {
      //std::cerr << "Waiting for input:  ";
      Message m = fromNode.read();
     // std:: cerr << m.data << " received" << endl;
      max = std::stoi(m.data);
      for (int i = 0; i <= max; ++i) {
        string event = (i % 2 == 0 ? "even_event" : "odd_event");
        Message tosend(event, std::to_string(i));
        writeToNode(progress, tosend);
        std::this_thread::sleep_for(chrono::milliseconds(100));
      }
    } while (max >= 0);
  }
};

// Important:  You MUST include this function, and you cannot alter
//             the signature at all.  The base wrapper class calls this
//             to build your particular worker.  The prototype for this
//             function is defined in addon-streams.h
StreamingWorker * create_worker(Callback *data
    , Callback *complete
    , Callback *error_callback) {
 return new EvenOdd(data, complete, error_callback);
}

// Don't forget this!  You can change the name of your module, 
// but the second parameter should always be as shown.
NODE_MODULE(even_odd_worker, StreamWorkerWrapper::Init)
