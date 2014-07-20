#ifndef PARSE_H
#define PARSE_H

Handle<Value> ParseField(const google::protobuf::Message &message, const Reflection *r, const FieldDescriptor *field, int index);
Handle<Object> ParsePart(const google::protobuf::Message &message);

#endif