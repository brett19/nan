/**********************************************************************************
 * NAN - Native Abstractions for Node.js
 *
 * Copyright (c) 2014 NAN contributors
 *
 * MIT +no-false-attribs License <https://github.com/rvagg/nan/blob/master/LICENSE>
 **********************************************************************************/

#ifndef _WIN32
#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif
#include <nan.h>

class ProgressWorker : public NanAsyncWorker {
 public:
  ProgressWorker(
      NanCallback *callback
    , NanCallback *progress
    , int milliseconds
    , int iters)
    : NanAsyncWorker(callback), progress(progress)
    , milliseconds(milliseconds), iters(iters) {}
  ~ProgressWorker() {}

  void Execute () {
    for (int i = 0; i < iters; ++i) {
      SendProgress(reinterpret_cast<const char*>(&i), sizeof(int));
      Sleep(milliseconds);
    }
  }

  void HandleProgressCallback(const char *data, size_t size) {
    NanScope();

    v8::Local<v8::Value> argv[] = {
        NanNew<v8::Integer>(*reinterpret_cast<int*>(const_cast<char*>(data)))
    };
    progress->Call(1, argv);
  }

 private:
  NanCallback *progress;
  int milliseconds;
  int iters;
};

NAN_METHOD(DoProgress) {
  NanScope();
  NanCallback *progress = new NanCallback(args[2].As<v8::Function>());
  NanCallback *callback = new NanCallback(args[3].As<v8::Function>());
  NanAsyncQueueWorker(new ProgressWorker(
      callback
    , progress
    , args[0]->Uint32Value()
    , args[1]->Uint32Value()));
  NanReturnUndefined();
}

void Init(v8::Handle<v8::Object> exports) {
  exports->Set(
      NanNew<v8::String>("a")
    , NanNew<v8::FunctionTemplate>(DoProgress)->GetFunction());
}

NODE_MODULE(asyncworkerprogress, Init)
