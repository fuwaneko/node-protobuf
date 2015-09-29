#ifndef PARSE_H
#define PARSE_H

Handle<Value> ParseField(const google::protobuf::Message &message, const Reflection *r, const FieldDescriptor *field, int index, bool preserve_int64);
Local<Object> ParsePart(const google::protobuf::Message &message, bool preserve_int64);

#endif
