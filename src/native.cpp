#include "native.h"
#include "parse.h"
#include "serialize.h"

Nan::Persistent<Function> constructor;

NativeProtobuf::NativeProtobuf(FileDescriptorSet *descriptors,
  bool preserve_int64) : preserve_int64(preserve_int64) {

  for (int i = 0; i < descriptors->file_size(); i++) {
    const FileDescriptor *f = pool.BuildFile(descriptors->file(i));
    for (int i = 0; i < f->message_type_count(); i++) {
      const Descriptor *d = f->message_type(i);
      const std::string name = d->full_name();

      info.push_back(name);
    }
  }

  // Initialization steps have been taken
}

void NativeProtobuf::Init(Local<Object> exports) {

  // constructor
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);

  tpl->SetClassName(Nan::New<String>("NativeProtobuf").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // prototype
  Nan::SetPrototypeMethod(tpl, "parse", NativeProtobuf::Parse);
  Nan::SetPrototypeMethod(tpl, "parseWithUnknown",
                          NativeProtobuf::ParseWithUnknown);
  Nan::SetPrototypeMethod(tpl, "serialize", NativeProtobuf::Serialize);
  Nan::SetPrototypeMethod(tpl, "info", NativeProtobuf::Info);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New<String>("native").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(NativeProtobuf::New) {

  Local<Object> buffer_obj = info[0]->ToObject();
  char *buffer_data = Buffer::Data(buffer_obj);
  size_t buffer_length = Buffer::Length(buffer_obj);

  FileDescriptorSet descriptors;
  if (!descriptors.ParseFromArray(buffer_data, buffer_length)) {
    Nan::ThrowError("Malformed descriptor");
    info.GetReturnValue().Set(Nan::Undefined());
    return;
  }

  bool preserve_int64 = info[1]->BooleanValue();

  NativeProtobuf *proto = new NativeProtobuf(&descriptors, preserve_int64);
  proto->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NativeProtobuf::Serialize) {

  NativeProtobuf *self = Nan::ObjectWrap::Unwrap<NativeProtobuf>(info.This());

  // get object to serialize and name of schema
  Local<Object> subj = info[0]->ToObject();
  String::Utf8Value schemaName(info[1]->ToString());
  std::string schema_name = std::string(*schemaName);

  // create a message based on schema
  const Descriptor *descriptor = self->pool.FindMessageTypeByName(schema_name);
  if (descriptor == NULL) {
    Nan::ThrowError(("Unknown schema name: " + schema_name).c_str());
    info.GetReturnValue().Set(Nan::Undefined());
    return;
  }

  google::protobuf::Message *message = self->factory.GetPrototype(descriptor)->New();

  if (SerializePart(message, subj, self->preserve_int64) < 0) {
    // required field not present!
    info.GetReturnValue().Set(Nan::Null());
    return;
  } else {
    // make JS Buffer instead of SlowBuffer
    int size = message->ByteSize();
    Local<Object> buffer = Nan::NewBuffer(size).ToLocalChecked();
    char *buf = Buffer::Data(buffer);
    bool result = message->SerializeToArray(buf, size);

    if (!result) {
      Nan::ThrowError("Can't serialize");
      info.GetReturnValue().Set(Nan::Undefined());
    } else {
      info.GetReturnValue().Set(buffer);
    }

  }

  delete message;
}

NAN_METHOD(NativeProtobuf::Parse) {

  NativeProtobuf *self = Nan::ObjectWrap::Unwrap<NativeProtobuf>(info.This());

  Local<Object> buffer_obj = info[0]->ToObject();
  char *buffer_data = Buffer::Data(buffer_obj);
  size_t buffer_length = Buffer::Length(buffer_obj);

  String::Utf8Value schemaName(info[1]->ToString());
  std::string schema_name = std::string(*schemaName);

  // create a message based on schema
  const Descriptor *descriptor = self->pool.FindMessageTypeByName(schema_name);
  if (descriptor == NULL) {
    Nan::ThrowError(("Unknown schema name: " + schema_name).c_str());
    info.GetReturnValue().Set(Nan::Null());
    return;
  }
  google::protobuf::Message *message = self->factory.GetPrototype(descriptor)->New();

  google::protobuf::io::ArrayInputStream array_stream(buffer_data,
                                                      buffer_length);
  google::protobuf::io::CodedInputStream coded_stream(&array_stream);
  size_t max = info[2]->Uint32Value();
  size_t warn = info[3]->Uint32Value();
  if (max) {
    coded_stream.SetTotalBytesLimit(max, warn ? warn : max);
  }
  bool parseResult = message->ParseFromCodedStream(&coded_stream);

  // Check if we want to use a typed array for our numeric fields.
  // Default to true

  bool use_typed_array;

  if (info.Length() < 5) {
    use_typed_array = true;
  } else {
    use_typed_array = info[4]->BooleanValue();
  }

  if (parseResult) {
    Local<Object> ret =
        ParsePart(Isolate::GetCurrent(), *message, self->preserve_int64,
          use_typed_array);
    info.GetReturnValue().Set(ret);
  } else {
    Nan::ThrowError("Malformed protocol buffer");
    info.GetReturnValue().Set(Nan::Null());
  }

  delete message;
}

NAN_METHOD(NativeProtobuf::ParseWithUnknown) {

  NativeProtobuf *self = Nan::ObjectWrap::Unwrap<NativeProtobuf>(info.This());

  Local<Object> buffer_obj = info[0]->ToObject();
  char *buffer_data = Buffer::Data(buffer_obj);
  size_t buffer_length = Buffer::Length(buffer_obj);

  String::Utf8Value schemaName(info[1]->ToString());
  std::string schema_name = std::string(*schemaName);

  // create a message based on schema
  const Descriptor *descriptor = self->pool.FindMessageTypeByName(schema_name);
  if (descriptor == NULL) {
    Nan::ThrowError(("Unknown schema name: " + schema_name).c_str());
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  google::protobuf::Message *message = self->factory.GetPrototype(descriptor)->New();

  google::protobuf::io::ArrayInputStream array_stream(buffer_data,
                                                      buffer_length);
  google::protobuf::io::CodedInputStream coded_stream(&array_stream);
  size_t max = info[2]->Uint32Value();
  size_t warn = info[3]->Uint32Value();
  if (max) {
    coded_stream.SetTotalBytesLimit(max, warn ? warn : max);
  }
  bool parseResult = message->ParseFromCodedStream(&coded_stream);

  // Check if we want to use a typed array for our numeric fields.
  // Default to true

  bool use_typed_array;

  if (info.Length() < 5) {
    use_typed_array = true;
  } else {
    use_typed_array = info[4]->BooleanValue();
  }

  if (parseResult) {
    Local<Object> ret = ParsePartWithUnknown(Isolate::GetCurrent(), *message,
                                             self->preserve_int64, use_typed_array);
    info.GetReturnValue().Set(ret);
  } else {
    Nan::ThrowError("Malformed protocol buffer");
    info.GetReturnValue().Set(Nan::Null());
  }

  delete message;
}

NAN_METHOD(NativeProtobuf::Info) {

  NativeProtobuf *self = Nan::ObjectWrap::Unwrap<NativeProtobuf>(info.This());
  Local<Array> array = Nan::New<Array>();

  for (unsigned long i = 0; i < self->info.size(); i++)
    array->Set(i, Nan::New<String>(self->info.at(i).c_str()).ToLocalChecked());

  info.GetReturnValue().Set(array);
}
