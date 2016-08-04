#include "parse.h"
#include "common.h"

bool NewTypedArray(Local<Object> &v, Isolate *isolate,
                   const FieldDescriptor *field, int size) {
#if NODE_MAJOR_VERSION > 0 || NODE_MINOR_VERSION > 11
  Local<ArrayBuffer> a;
  switch (field->cpp_type()) {
  case FieldDescriptor::CPPTYPE_INT32:
    a = ArrayBuffer::New(isolate, sizeof(int32) * size);
    v = Int32Array::New(a, 0, size);
    return true;
  case FieldDescriptor::CPPTYPE_UINT32:
    a = ArrayBuffer::New(isolate, sizeof(uint32) * size);
    v = Uint32Array::New(a, 0, size);
    return true;
  case FieldDescriptor::CPPTYPE_FLOAT:
    a = ArrayBuffer::New(isolate, sizeof(float) * size);
    v = Float32Array::New(a, 0, size);
    return true;
  case FieldDescriptor::CPPTYPE_DOUBLE:
    a = ArrayBuffer::New(isolate, sizeof(double) * size);
    v = Float64Array::New(a, 0, size);
    return true;
  default:
    break;
  }
#endif
  return false;
}

Local<Value> ParseField(Isolate *isolate,
                        const google::protobuf::Message &message,
                        const Reflection *r, const FieldDescriptor *field,
                        int index, bool preserve_int64) {
  Local<Value> v;

  switch (field->cpp_type()) {
  case FieldDescriptor::CPPTYPE_INT32: {
    int32 value;
    if (index >= 0)
      value = r->GetRepeatedInt32(message, field, index);
    else
      value = r->GetInt32(message, field);
    v = Nan::New<Number>(value);
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
      v = Nan::New<String>(std::to_string(value).c_str()).ToLocalChecked();
    } else {
      v = Nan::New<Number>(value);
    }

    break;
  }
  case FieldDescriptor::CPPTYPE_UINT32: {
    uint32 value;
    if (index >= 0)
      value = r->GetRepeatedUInt32(message, field, index);
    else
      value = r->GetUInt32(message, field);
    v = Nan::New<Number>(value);
    break;
  }
  case FieldDescriptor::CPPTYPE_UINT64: {
    uint64 value;
    if (index >= 0)
      value = r->GetRepeatedUInt64(message, field, index);
    else
      value = r->GetUInt64(message, field);
    if (preserve_int64) {
      v = Nan::New<String>(std::to_string(value).c_str()).ToLocalChecked();
    } else {
      v = Nan::New<Number>(value);
    }

    break;
  }
  case FieldDescriptor::CPPTYPE_DOUBLE: {
    double value;
    if (index >= 0)
      value = r->GetRepeatedDouble(message, field, index);
    else
      value = r->GetDouble(message, field);
    v = Nan::New<Number>(value);
    break;
  }
  case FieldDescriptor::CPPTYPE_FLOAT: {
    float value;
    if (index >= 0)
      value = r->GetRepeatedFloat(message, field, index);
    else
      value = r->GetFloat(message, field);
    v = Nan::New<Number>(value);
    break;
  }
  case FieldDescriptor::CPPTYPE_BOOL: {
    bool value;
    if (index >= 0)
      value = r->GetRepeatedBool(message, field, index);
    else
      value = r->GetBool(message, field);
    v = Nan::New<Boolean>(value);
    break;
  }
  case FieldDescriptor::CPPTYPE_ENUM: {
    if (index >= 0)
      v = Nan::New<String>(
              r->GetRepeatedEnum(message, field, index)->name().c_str())
              .ToLocalChecked();
    else
      v = Nan::New<String>(r->GetEnum(message, field)->name().c_str())
              .ToLocalChecked();
    break;
  }
  case FieldDescriptor::CPPTYPE_MESSAGE: {
    if (field->is_optional() && !r->HasField(message, field))
      v = Nan::Null();
    else {
      if (index >= 0)
        v = ParsePart(isolate, r->GetRepeatedMessage(message, field, index),
                      preserve_int64);
      else
        v = ParsePart(isolate, r->GetMessage(message, field), preserve_int64);
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
      v = Nan::CopyBuffer(const_cast<char *>(value.data()), value.length())
              .ToLocalChecked();
    else
      v = Nan::New<String>(value.c_str()).ToLocalChecked();
    break;
  }
  }

  return v;
}

Local<Object>
ParseUnknownFieldset(const google::protobuf::UnknownFieldSet &fs) {
  Local<Array> ret = Nan::New<Array>();

  const int field_count = fs.field_count();

  for (int i = 0; i < field_count; i++) {
    Local<Object> obj = Nan::New<Object>();
    const google::protobuf::UnknownField uf = fs.field(i);

    const google::protobuf::UnknownField::Type type = uf.type();
    const int tag = uf.number();

    obj->Set(Nan::New<String>("number").ToLocalChecked(),
             Nan::New<Number>(tag));

    Local<String> type_key = Nan::New<String>("type").ToLocalChecked();

    switch (type) {
    case google::protobuf::UnknownField::TYPE_VARINT:
      obj->Set(type_key, Nan::New<String>("TYPE_VARINT").ToLocalChecked());
      break;
    case google::protobuf::UnknownField::TYPE_FIXED32:
      obj->Set(type_key, Nan::New<String>("TYPE_FIXED32").ToLocalChecked());
      break;
    case google::protobuf::UnknownField::TYPE_FIXED64:
      obj->Set(type_key, Nan::New<String>("TYPE_FIXED64").ToLocalChecked());
      break;
    case google::protobuf::UnknownField::TYPE_LENGTH_DELIMITED:
      obj->Set(type_key,
               Nan::New<String>("TYPE_LENGTH_DELIMITED").ToLocalChecked());
      break;
    case google::protobuf::UnknownField::TYPE_GROUP:
      obj->Set(type_key, Nan::New<String>("TYPE_GROUP").ToLocalChecked());
      break;
    default:
      obj->Set(type_key, Nan::New<String>("UNKNOWN").ToLocalChecked());
    }

    ret->Set(i, obj);
  }

  return ret;
}

Local<Object> ParsePartWithUnknown(Isolate *isolate,
                               const google::protobuf::Message &message,
                               bool preserve_int64) {
  // reset whitespace
  return ParsePartWithUnknown(isolate, message, preserve_int64, true);
}

Local<Object> ParsePartWithUnknown(Isolate *isolate,
                                   const google::protobuf::Message &message,
                                   bool preserve_int64, bool use_typed_array) {
  // reset whitespace
  Local<Object> ret = Nan::New<Object>();

  const Reflection *r = message.GetReflection();
  const Descriptor *d = message.GetDescriptor();

  // get fields of descriptor
  uint32_t count = d->field_count();
  for (uint32_t i = 0; i < count; i++) {
    const FieldDescriptor *field = d->field(i);

    if (field != NULL) {

      if (field->is_optional() && !r->HasField(message, field))
        continue;

      Local<Value> v;

      if (field->is_repeated()) {
        int size = r->FieldSize(message, field);
        Local<Object> typedArray;
        if (size > 0 && use_typed_array && NewTypedArray(typedArray, isolate, field, size)) {
          for (int i = 0; i < size; i++)
            typedArray->Set(
                i, ParseField(isolate, message, r, field, i, preserve_int64));
          v = typedArray;
        } else {
          Local<Array> array = Nan::New<Array>(size);
          for (int i = 0; i < size; i++)
            array->Set(
                i, ParseField(isolate, message, r, field, i, preserve_int64));
          v = array;
        }
      } else {
        v = ParseField(isolate, message, r, field, -1, preserve_int64);
      }

      ret->Set(Nan::New<String>(field->name().c_str()).ToLocalChecked(), v);
    }
  }

  const google::protobuf::UnknownFieldSet &unknownFields =
      r->GetUnknownFields(message);
  ret->Set(Nan::New<String>("$unknownFields").ToLocalChecked(),
           ParseUnknownFieldset(unknownFields));

  return ret;
}

Local<Object> ParsePart(Isolate *isolate,
                        const google::protobuf::Message &message,
                        bool preserve_int64) {
  // reset whitespace
  return ParsePart(isolate, message, preserve_int64, true);
}

Local<Object> ParsePart(Isolate *isolate,
                        const google::protobuf::Message &message,
                        bool preserve_int64,
                        bool use_typed_array) {
  // reset whitespace
  Local<Object> ret = Nan::New<Object>();
  // get a reflection
  const Reflection *r = message.GetReflection();
  const Descriptor *d = message.GetDescriptor();

  // get fields of descriptor
  uint32_t count = d->field_count();
  for (uint32_t i = 0; i < count; i++) {
    const FieldDescriptor *field = d->field(i);

    if (field != NULL) {

      if (field->is_optional() && !r->HasField(message, field))
        continue;

      Local<Value> v;

      if (field->is_repeated()) {
        int size = r->FieldSize(message, field);
        Local<Object> typedArray;
        if (size > 0 && use_typed_array && NewTypedArray(typedArray, isolate, field, size)) {
          for (int i = 0; i < size; i++)
            typedArray->Set(
                i, ParseField(isolate, message, r, field, i, preserve_int64));
          v = typedArray;
        } else {
          Local<Array> array = Nan::New<Array>(size);
          for (int i = 0; i < size; i++)
            array->Set(
                i, ParseField(isolate, message, r, field, i, preserve_int64));
          v = array;
        }
      } else {
        v = ParseField(isolate, message, r, field, -1, preserve_int64);
      }

      ret->Set(Nan::New<String>(field->name().c_str()).ToLocalChecked(), v);
    }
  }

  return ret;
}
