#ifndef COMMON_H
#define COMMON_H

#include <v8.h>

#include <nan.h>
#include <node.h>

#include <stdio.h>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/service.h>
#include <string>

using namespace google::protobuf;
using namespace v8;
using namespace node;

#endif
