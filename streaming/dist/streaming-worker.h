#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>
#include <thread>
#include <deque>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <nan.h>

using namespace Nan;
using namespace std;

template <typename Data>
class PCQueue
{
public:
  void write(Data data)
  {
    std::unique_lock<std::mutex> locker(mu);
    buffer_.push_back(data);
    locker.unlock();
    cond.notify_all();
    return;
  }
  Data read()
  {
    std::unique_lock<std::mutex> locker(mu);
    cond.wait(locker, [this]() { return buffer_.size() > 0; });
    Data back = buffer_.front();
    buffer_.pop_front();
    locker.unlock();
    cond.notify_all();
    return back;
  }
  void readAll(std::deque<Data> &target)
  {
    std::unique_lock<std::mutex> locker(mu);
    std::copy(buffer_.begin(), buffer_.end(), std::back_inserter(target));
    buffer_.clear();
    locker.unlock();
  }
  PCQueue() {}

private:
  std::mutex mu;
  std::condition_variable cond;
  std::deque<Data> buffer_;
};

class Message
{
public:
  string name;
  string data;
  Message(string name, string data) : name(name), data(data) {}
};

class StreamingWorker : public AsyncProgressWorker
{
public:
  StreamingWorker(
      Callback *progress,
      Callback *callback,
      Callback *error_callback)
      : AsyncProgressWorker(callback), progress(progress), error_callback(error_callback)
  {
    input_closed = false;
  }
  ~StreamingWorker()
  {
    delete progress;
    delete error_callback;
  }

  void HandleErrorCallback()
  {
    HandleScope scope;

    v8::Local<v8::Value> argv[] = {
        v8::Exception::Error(New<v8::String>(ErrorMessage()).ToLocalChecked())};
    error_callback->Call(1, argv);
  }

  void HandleOKCallback()
  {
    drainQueue();
    callback->Call(0, NULL);
  }

  void HandleProgressCallback(const char *data, size_t size)
  {
    drainQueue();
  }

  void close()
  {
    input_closed = true;
  }

  PCQueue<Message> fromNode;

protected:
  void writeToNode(const AsyncProgressWorker::ExecutionProgress &progress, Message &msg)
  {
    toNode.write(msg);
    progress.Send(reinterpret_cast<const char *>(&toNode), sizeof(toNode));
  }

  bool closed()
  {
    return input_closed;
  }

  Callback *progress;
  Callback *error_callback;
  PCQueue<Message> toNode;
  bool input_closed;

private:
  void drainQueue()
  {
    HandleScope scope;

    // drain the queue - since we might only get called once for many writes
    std::deque<Message> contents;
    toNode.readAll(contents);

    for (Message &msg : contents)
    {
      v8::Local<v8::Value> argv[] = {
          New<v8::String>(msg.name.c_str()).ToLocalChecked(),
          New<v8::String>(msg.data.c_str()).ToLocalChecked()};
      progress->Call(2, argv);
    }
  }
};

StreamingWorker *create_worker(Callback *, Callback *, Callback *, v8::Local<v8::Object> &);

class StreamWorkerWrapper : public Nan::ObjectWrap
{
public:
  static NAN_MODULE_INIT(Init)
  {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("StreamingWorker").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(2);

    SetPrototypeMethod(tpl, "sendToAddon", sendToAddon);
    SetPrototypeMethod(tpl, "closeInput", closeInput);

    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("StreamingWorker").ToLocalChecked(),
             Nan::GetFunction(tpl).ToLocalChecked());
  }

private:
  explicit StreamWorkerWrapper(StreamingWorker *worker) : _worker(worker) {}
  ~StreamWorkerWrapper() {}

  static NAN_METHOD(New)
  {
    if (info.IsConstructCall())
    {
      Callback *data_callback = new Callback(info[0].As<v8::Function>());
      Callback *complete_callback = new Callback(info[1].As<v8::Function>());
      Callback *error_callback = new Callback(info[2].As<v8::Function>());
      v8::Local<v8::Object> options = info[3].As<v8::Object>();

      StreamWorkerWrapper *obj = new StreamWorkerWrapper(
          create_worker(
              data_callback,
              complete_callback,
              error_callback, options));

      obj->Wrap(info.This());
      info.GetReturnValue().Set(info.This());

      // start the worker
      AsyncQueueWorker(obj->_worker);
    }
    else
    {
      const int argc = 3;
      v8::Local<v8::Value> argv[argc] = {info[0], info[1], info[2]};
      v8::Local<v8::Function> cons = Nan::New(constructor());
      info.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
  }

  static NAN_METHOD(sendToAddon)
  {
    v8::String::Utf8Value name(info[0]->ToString());
    v8::String::Utf8Value data(info[1]->ToString());
    StreamWorkerWrapper *obj = Nan::ObjectWrap::Unwrap<StreamWorkerWrapper>(info.Holder());
    obj->_worker->fromNode.write(Message(*name, *data));
  }

  static NAN_METHOD(closeInput)
  {
    StreamWorkerWrapper *obj = Nan::ObjectWrap::Unwrap<StreamWorkerWrapper>(info.Holder());
    obj->_worker->close();
  }

  static inline Nan::Persistent<v8::Function> &constructor()
  {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }

  StreamingWorker *_worker;
};
