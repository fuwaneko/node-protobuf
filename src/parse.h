#ifndef PARSE_H
#define PARSE_H

#include "common.h"

Local<Value> ParseField(Isolate *isolate,
                        const google::protobuf::Message &message,
                        const Reflection *r, const FieldDescriptor *field,
                        int index, bool preserve_int64);

Local<Object> ParsePart(Isolate *isolate,
                        const google::protobuf::Message &message,
                        bool preserve_int64);

Local<Object> ParsePart(Isolate *isolate,
                        const google::protobuf::Message &message,
                        bool preserve_int64,
                        bool use_typed_array);

Local<Object> ParsePartWithUnknown(Isolate *isolate,
                                   const google::protobuf::Message &message,
                                   bool preserve_int64);
                                   
 Local<Object> ParsePartWithUnknown(Isolate *isolate,
                                    const google::protobuf::Message &message,
                                    bool preserve_int64,
                                    bool use_typed_array);

#endif
