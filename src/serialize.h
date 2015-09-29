#ifndef SERIALIZE_H
#define SERIALIZE_H

void SerializeField(google::protobuf::Message *message, const Reflection *r, const FieldDescriptor *field, Handle<Value> val, bool preserve_int64);
int SerializePart(google::protobuf::Message *message, Handle<Object> subj, bool preserve_int64);

#endif
