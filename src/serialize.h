#ifndef SERIALIZE_H
#define SERIALIZE_H

void SerializeField(google::protobuf::Message *message, const Reflection *r, const FieldDescriptor *field, Handle<Value> val);
int SerializePart(google::protobuf::Message *message, Handle<Object> subj);

#endif