void SerializeField(google::protobuf::Message *message, const Reflection *r, const FieldDescriptor *field, Handle<Value> val);
void SerializePart(google::protobuf::Message *message, Handle<Object> subj);
NAN_METHOD(Serialize);