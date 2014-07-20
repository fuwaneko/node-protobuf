#include "common.h"
#include "native.h"

void init(Handle<Object> exports) {
	NativeProtobuf::Init(exports);
}

NODE_MODULE(protobuf, init)