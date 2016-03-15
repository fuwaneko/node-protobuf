#include "common.h"
#include "native.h"

void init(Local<Object> exports) { NativeProtobuf::Init(exports); }

NODE_MODULE(protobuf, init)
