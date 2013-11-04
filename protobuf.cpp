#include <v8.h>

#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>

#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/service.h>

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string> 

using namespace google::protobuf;
using namespace v8;
using node::Buffer;

bool preserve_int64;

class Protobuf : public node::ObjectWrap {
public:
  static void Init(Handle<Object> exports);

private:
  Protobuf(DescriptorPool *pool);
  ~Protobuf();

  DescriptorPool *pool;

  static Handle<v8::Value> New(const Arguments &args);
  static Handle<Value> Serialize(const Arguments &args);
  static Handle<Value> Parse(const Arguments &args);
};

Protobuf::Protobuf(DescriptorPool *pool) {
  this->pool = pool;
}

Protobuf::~Protobuf() {
  delete this->pool;
}

void Protobuf::Init(Handle<Object> exports) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Protobuf"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("Serialize"), FunctionTemplate::New(Serialize)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("Parse"), FunctionTemplate::New(Parse)->GetFunction());

  Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("Protobuf"), constructor);
}

Handle<Value> Protobuf::New(const Arguments& args) {
  HandleScope scope;

  Local<Object> buffer_obj = args[0]->ToObject();
  char *buffer_data = Buffer::Data(buffer_obj);
  size_t buffer_length = Buffer::Length(buffer_obj);

  FileDescriptorSet descriptors;
  if (!descriptors.ParseFromArray(buffer_data, buffer_length)) {
    return v8::ThrowException(
        v8::Exception::Error(String::New("Malformed descriptor")));
  }

  DescriptorPool* pool = new DescriptorPool;
  for (int i = 0; i < descriptors.file_size(); i++) {
    pool->BuildFile(descriptors.file(i));
  }

  if (args.Length() > 1) {
    preserve_int64 = args[1]->ToBoolean()->Value();
  }

  Protobuf* obj = new Protobuf(pool);
  obj->Wrap(args.This());

  return args.This();
}

void SerializePart(google::protobuf::Message *message, Handle<Object> subj);
void SerializeField(google::protobuf::Message *message, const Reflection *r, const FieldDescriptor *field, Handle<Value> val) {
  const EnumValueDescriptor *enumValue = NULL;
  bool repeated = field->is_repeated();

  if (*val != NULL) {
    switch (field->cpp_type()) {
      case FieldDescriptor::CPPTYPE_INT32: {
        if (repeated)
          r->AddInt32(message, field, val->Int32Value());
        else
          r->SetInt32(message, field, val->Int32Value());
        break;
      }
      case FieldDescriptor::CPPTYPE_INT64:
        if (repeated)
          if (preserve_int64 && val->IsArray()) {
            Local<Object> n64_array = val->ToObject();
            uint64 n64;
            uint32 hi = n64_array->Get(0)->Uint32Value(), lo = n64_array->Get(1)->Uint32Value();
            n64 = ((uint64)hi << 32) + (uint64)lo;
            r->AddInt64(message, field, n64);
          } else
            r->AddInt64(message, field, val->NumberValue());
        else
          if (preserve_int64 && val->IsArray()) {
            Local<Object> n64_array = val->ToObject();
            uint64 n64;
            uint32 hi = n64_array->Get(0)->Uint32Value(), lo = n64_array->Get(1)->Uint32Value();
            n64 = ((uint64)hi << 32) + (uint64)lo;
            r->SetInt64(message, field, n64);
          } else
            r->SetInt64(message, field, val->NumberValue());
        break;
      case FieldDescriptor::CPPTYPE_UINT32:
        if (repeated)
          r->AddUInt32(message, field, val->Uint32Value());
        else
          r->SetUInt32(message, field, val->Uint32Value());
        break;
      case FieldDescriptor::CPPTYPE_UINT64:
        if (repeated)
          if (preserve_int64 && val->IsArray()) {
            Local<Object> n64_array = val->ToObject();
            uint64 n64;
            uint32 hi = n64_array->Get(0)->Uint32Value(), lo = n64_array->Get(1)->Uint32Value();
            n64 = ((uint64)hi << 32) + (uint64)lo;
            r->AddUInt64(message, field, n64);
          } else
            r->AddUInt64(message, field, val->NumberValue());
        else
          if (preserve_int64 && val->IsArray()) {
            Local<Object> n64_array = val->ToObject();
            uint64 n64;
            uint32 hi = n64_array->Get(0)->Uint32Value(), lo = n64_array->Get(1)->Uint32Value();
            n64 = ((uint64)hi << 32) + (uint64)lo;
            r->SetUInt64(message, field, n64);
          } else
            r->SetUInt64(message, field, val->NumberValue());
        break;
      case FieldDescriptor::CPPTYPE_DOUBLE:
        if (repeated)
          r->AddDouble(message, field, val->NumberValue());
        else
          r->SetDouble(message, field, val->NumberValue());
        break;
      case FieldDescriptor::CPPTYPE_FLOAT:
        if (repeated)
          r->AddFloat(message, field, val->NumberValue());
        else
          r->SetFloat(message, field, val->NumberValue());
        break;
      case FieldDescriptor::CPPTYPE_BOOL:
        if (repeated)
          r->AddBool(message, field, val->BooleanValue());
        else
          r->SetBool(message, field, val->BooleanValue());
        break;
      case FieldDescriptor::CPPTYPE_ENUM:
        enumValue =
          val->IsNumber() ?
            field->enum_type()->FindValueByNumber(val->Int32Value()) :
            field->enum_type()->FindValueByName(*String::AsciiValue(val));

        if (enumValue != NULL) {
          if (repeated)
            r->AddEnum(message, field, enumValue);
          else
            r->SetEnum(message, field, enumValue);
        }
        break;
      case FieldDescriptor::CPPTYPE_MESSAGE:
        if (val->IsObject()) {
          if (repeated)
            SerializePart(r->AddMessage(message, field), val.As<Object>());
          else
            SerializePart(r->MutableMessage(message, field), val.As<Object>());
        }
        break;
      case FieldDescriptor::CPPTYPE_STRING:
        if( Buffer::HasInstance(val)){
          Local<Object> buf = val->ToObject();
          r->SetString( message, field, std::string(Buffer::Data(buf), Buffer::Length(buf)));
          break;
        }

        String::Utf8Value temp(val->ToString());
        std::string value = std::string(*temp);
        if (repeated)
          r->AddString(message, field, value);
        else
          r->SetString(message, field, value);
        break;
    }
  }
}

void SerializePart(google::protobuf::Message *message, Handle<Object> subj) {
  // get a reflection
  const Reflection *r = message->GetReflection();
  const Descriptor *d = message->GetDescriptor();

  // build a reflection
  // get properties of passed object
  Local<Array> properties = subj->GetPropertyNames();
  uint32_t len = properties->Length();

  for (uint32_t i = 0; i < len; i++) {
    // TODO: iterate over descriptor properties?
    // TODO: Yes, we must process required fields
    Local<Value> property = properties->Get(i);
    Local<String> property_s = property->ToString();

    if (*property_s == NULL)
      continue;

    String::Utf8Value temp(property);
    std::string propertyName = std::string(*temp);

    const FieldDescriptor *field = d->FindFieldByName(propertyName);
    if (field == NULL) continue;

    Local<Value> val = subj->Get(property);

    if (field->is_repeated()) {
      if (!val->IsArray())
        continue;

      Handle<Array> array = val.As<Array>();
      int len = array->Length();

      for (int i = 0; i < len; i++)
        SerializeField(message, r, field, array->Get(i));

    } else {
      SerializeField(message, r, field, val);
    }
  }
}

Handle<Value> Protobuf::Serialize(const Arguments &args) {
  HandleScope scope;

  Protobuf* obj = ObjectWrap::Unwrap<Protobuf>(args.This());

  // get object to serialize and name of schema
  Local<Object> subj = args[0]->ToObject();
  String::Utf8Value schemaName(args[1]->ToString());
  std::string schema_name = std::string(*schemaName);

  // create a message based on schema
  DynamicMessageFactory factory;
  const Descriptor *descriptor = obj->pool->FindMessageTypeByName(schema_name);
  if (descriptor == NULL) {
    std::string error = "Unknown schema name: " + schema_name;
    ThrowException(Exception::Error(String::New(error.c_str())));
    return scope.Close(Undefined());
  }
  google::protobuf::Message *message = factory.GetPrototype(descriptor)->New();

  SerializePart(message, subj);

  // make JS Buffer instead of SlowBuffer
  int size = message->ByteSize();
  Buffer *buf = Buffer::New(size);
  bool result = message->SerializeToArray(Buffer::Data(buf), size);

  if (!result) {
    return v8::ThrowException(
        v8::Exception::Error(String::New("Can't serialize")));
  }

  // obtain Node.js Buffer constructor
  Local<Object> gScope = Context::GetCurrent()->Global();
  Local<Function> bConstructor = Local<Function>::Cast(gScope->Get(String::New("Buffer")));

  // construct Node.js Buffer
  Handle<Value> constructorArgs[3] = { buf->handle_, Integer::New(size), Integer::New(0) };
  Local<Object> actualBuffer = bConstructor->NewInstance(3, constructorArgs);

  delete message;

  return scope.Close(actualBuffer);
}

Handle<Object> ParsePart(const google::protobuf::Message &message);
Handle<Value> ParseField(const google::protobuf::Message &message, const Reflection *r, const FieldDescriptor *field, int index) {
  HandleScope scope;
  Handle<Value> v;

  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32: {
      int32 value;
      if (index >= 0)
        value = r->GetRepeatedInt32(message, field, index);
      else
        value = r->GetInt32(message, field);
      v = Number::New(value);
      break;
    }
    case FieldDescriptor::CPPTYPE_INT64: {
      int64 value;
      if (index >= 0)
        value = r->GetRepeatedInt64(message, field, index);
      else
        value = r->GetInt64(message, field);

      // to retain exact value if preserve_int64 flag was passed to constructor
      // extract int64 as two int32
      if (preserve_int64) {
        uint32 hi, lo;
        hi = (uint32) ((uint64)value >> 32);
        lo = (uint32) value;
        Local<Array> t = Array::New(2);
        t->Set(0, Number::New(hi));
        t->Set(1, Number::New(lo));
        v = t;
      } else
        v = Number::New(value);
      break;
    }
    case FieldDescriptor::CPPTYPE_UINT32: {
      uint32 value;
      if (index >= 0)
        value = r->GetRepeatedUInt32(message, field, index);
      else
        value = r->GetUInt32(message, field);
      v = Number::New(value);
      break;
    }
    case FieldDescriptor::CPPTYPE_UINT64: {
      uint64 value;
      if (index >= 0)
        value = r->GetRepeatedUInt64(message, field, index);
      else
        value = r->GetUInt64(message, field);
      if (preserve_int64) {
        uint32 hi, lo;
        hi = (uint32) (value >> 32);
        lo = (uint32) (value);
        Local<Array> t = Array::New(2);
        t->Set(0, Number::New(hi));
        t->Set(1, Number::New(lo));
        v = t;
      } else
        v = Number::New(value);
      break;
    }
    case FieldDescriptor::CPPTYPE_DOUBLE: {
      double value;
      if (index >= 0)
        value = r->GetRepeatedDouble(message, field, index);
      else
        value = r->GetDouble(message, field);
      v = Number::New(value);
      break;
    }
    case FieldDescriptor::CPPTYPE_FLOAT: {
      float value;
      if (index >= 0)
        value = r->GetRepeatedFloat(message, field, index);
      else
        value = r->GetFloat(message, field);
      v = Number::New(value);
      break;
    }
    case FieldDescriptor::CPPTYPE_BOOL: {
      bool value;
      if (index >= 0)
        value = r->GetRepeatedBool(message, field, index);
      else
        value = r->GetBool(message, field);
      v = Boolean::New(value);
      break;
    }
    case FieldDescriptor::CPPTYPE_ENUM: {
      if (index >= 0)
        v = String::New(r->GetRepeatedEnum(message, field, index)->name().c_str());
      else
        v = String::New(r->GetEnum(message, field)->name().c_str());
      break;
    }
    case FieldDescriptor::CPPTYPE_MESSAGE: {
      if (field->is_optional() && !r->HasField(message, field))
        v = Null();
      else {
        if (index >= 0)
          v = ParsePart(r->GetRepeatedMessage(message, field, index));
        else
          v = ParsePart(r->GetMessage(message, field));
      }
      break;
    }
    case FieldDescriptor::CPPTYPE_STRING: {
      std::string value;
      if (index >= 0)
        value = r->GetRepeatedString(message, field, index);
      else
        value = r->GetString(message, field);
      if (field->type() == FieldDescriptor::TYPE_BYTES)
        v = Buffer::New(const_cast<char *>(value.data()), value.length())->handle_;
      else
        v = String::New(value.c_str());
      break;
    }
  }

  return scope.Close(v);
}

Handle<Object> ParsePart(const google::protobuf::Message &message) {
  HandleScope scope;

  Handle<Object> ret = Object::New();
  // get a reflection
  const Reflection *r = message.GetReflection();
  const Descriptor *d = message.GetDescriptor();

  // get fields of descriptor
  uint32_t count = d->field_count();
  for (uint32_t i = 0; i < count; i++) {
    const FieldDescriptor *field = d->field(i);

    if (field != NULL) {
      Handle<Value> v;

      if (field->is_repeated()) {
        int size = r->FieldSize(message, field);
        Handle<Array> array = Array::New(size);
        for (int i = 0; i < size; i++) {
          array->Set(i, ParseField(message, r, field, i));
        }
        v = array;
      } else {
        v = ParseField(message, r, field, -1);
      }

      if (field->is_optional() && (v->IsNull() || !r->HasField(message, field)))
        continue;
      
      ret->Set(String::NewSymbol(field->name().c_str()), v);
    }
  }

  return scope.Close(ret);
}

Handle<Value> Protobuf::Parse(const Arguments &args) {
  HandleScope scope;

  Protobuf* obj = ObjectWrap::Unwrap<Protobuf>(args.This());

  if (!Buffer::HasInstance(args[0])) {
    ThrowException(Exception::Error(String::New("First argument must be a Buffer")));
    return scope.Close(Undefined());
  }

  Local<Object> buffer_obj = args[0]->ToObject();
  char *buffer_data = Buffer::Data(buffer_obj);
  size_t buffer_length = Buffer::Length(buffer_obj);

  String::Utf8Value schemaName(args[1]->ToString());
  std::string schema_name = std::string(*schemaName);

  // create a message based on schema
  DynamicMessageFactory factory;
  const Descriptor *descriptor = obj->pool->FindMessageTypeByName(schema_name);
  if (descriptor == NULL) {
    std::string error = "Unknown schema name: " + schema_name;
    ThrowException(Exception::Error(String::New(error.c_str())));
    return scope.Close(Undefined());
  }
  google::protobuf::Message *message = factory.GetPrototype(descriptor)->New();

  bool parseResult = message->ParseFromArray(buffer_data, buffer_length);

  if (parseResult) {
    Handle<Object> ret = ParsePart(*message);
    delete message;
    return scope.Close(ret);
  } else {
    ThrowException(Exception::Error(String::New("Malformed protocol buffer")));
    return scope.Close(Undefined());
  }
}

void init(Handle<Object> exports) {
  Protobuf::Init(exports);
}

NODE_MODULE(protobuf, init)
