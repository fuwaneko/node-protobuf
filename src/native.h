#ifndef NATIVE_H
#define NATIVE_H

#include "common.h"

class NativeProtobuf : public Nan::ObjectWrap {
public:
  static void Init(Local<Object> exports);
  NativeProtobuf(FileDescriptorSet *descriptors, bool preserve_int64);

  DescriptorPool pool;
  std::vector<std::string> info;
  bool preserve_int64;

private:
  DynamicMessageFactory factory;
  static NAN_METHOD(New);
  static NAN_METHOD(Parse);
  static NAN_METHOD(ParseWithUnknown);
  static NAN_METHOD(Serialize);
  static NAN_METHOD(Info);
};

#endif
