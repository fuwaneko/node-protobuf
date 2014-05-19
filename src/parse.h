Handle<Value> ParseField(const google::protobuf::Message &message, const Reflection *r, const FieldDescriptor *field, int index);
Handle<Object> ParsePart(const google::protobuf::Message &message);
NAN_METHOD(Parse);
NAN_METHOD(lookupMessage);
