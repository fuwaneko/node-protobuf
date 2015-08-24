#ifndef COMMON_H
#define COMMON_H

#include <v8.h>

#include <node.h>
#include <nan.h>

#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/service.h>
#include <string>

using namespace google::protobuf;
using namespace v8;
using namespace node;

static bool preserve_int64 = true;

#endif
