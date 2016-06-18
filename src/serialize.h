#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "common.h"

void SerializeField(google::protobuf::Message *message, const Reflection *r,
                    const FieldDescriptor *field, Local<Value> val,
                    bool preserve_int64);
int SerializePart(google::protobuf::Message *message, Local<Object> subj,
                  bool preserve_int64);

#endif
