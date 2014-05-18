#include <v8.h>

#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>

#include <nan.h>

#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/service.h>

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string> 

using namespace google::protobuf;
using namespace v8;
using node::Buffer;

bool preserve_int64;

class Protobuf : public node::ObjectWrap {
public:
  static void Init(Handle<Object> exports);

private:
  Protobuf(DescriptorPool *pool);
  ~Protobuf();

  DescriptorPool *pool;

  static Handle<v8::Value> New(const Arguments &args);
  static Handle<Value> Serialize(const Arguments &args);
  static Handle<Value> Parse(const Arguments &args);
};

Protobuf::Protobuf(DescriptorPool *pool) {
  this->pool = pool;
}

Protobuf::~Protobuf() {
  delete this->pool;
}

void Protobuf::Init(Handle<Object> exports) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Protobuf"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("Serialize"), FunctionTemplate::New(Serialize)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("Parse"), FunctionTemplate::New(Parse)->GetFunction());

  Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("Protobuf"), constructor);
}

Handle<Value> Protobuf::New(const Arguments& args) {
  HandleScope scope;

  Local<Object> buffer_obj = args[0]->ToObject();
  char *buffer_data = Buffer::Data(buffer_obj);
  size_t buffer_length = Buffer::Length(buffer_obj);

  FileDescriptorSet descriptors;
  if (!descriptors.ParseFromArray(buffer_data, buffer_length)) {
    return v8::ThrowException(
        v8::Exception::Error(String::New("Malformed descriptor")));
  }

  DescriptorPool* pool = new DescriptorPool;
  for (int i = 0; i < descriptors.file_size(); i++) {
    pool->BuildFile(descriptors.file(i));
  }

  if (args.Length() > 1) {
    preserve_int64 = args[1]->ToBoolean()->Value();
  }

  Protobuf* obj = new Protobuf(pool);
  obj->Wrap(args.This());

  return args.This();
}

void init(Handle<Object> exports) {
  exports->Set(NanSymbol("libraryInit"), NanNew<FunctionTemplate>(libraryInit)->GetFunction());
}

NODE_MODULE(protobuf, init)
