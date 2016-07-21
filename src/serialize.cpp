#include "common.h"

#include "serialize.h"

void SerializeField(google::protobuf::Message *message, const Reflection *r,
                    const FieldDescriptor *field, Local<Value> val,
                    bool preserve_int64) {
  const EnumValueDescriptor *enumValue = NULL;
  bool repeated = field->is_repeated();

  if (*val != NULL) {
    if (field->is_optional() && (val->IsNull() || val->IsUndefined()))
      return;

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
          uint32 hi = n64_array->Get(0)->Uint32Value(),
                 lo = n64_array->Get(1)->Uint32Value();
          n64 = ((uint64)hi << 32) + (uint64)lo;
          r->AddInt64(message, field, n64);
        } else if (preserve_int64 && val->IsString()) {
          String::Utf8Value temp(val->ToString());
          std::string value = std::string(*temp);
          r->AddInt64(message, field, std::stoll(value, nullptr, 10));
        } else
          r->AddInt64(message, field, val->NumberValue());
      else if (preserve_int64 && val->IsArray()) {
        Local<Object> n64_array = val->ToObject();
        uint64 n64;
        uint32 hi = n64_array->Get(0)->Uint32Value(),
               lo = n64_array->Get(1)->Uint32Value();
        n64 = ((uint64)hi << 32) + (uint64)lo;
        r->SetInt64(message, field, n64);
      } else if (preserve_int64 && val->IsString()) {
        String::Utf8Value temp(val->ToString());
        std::string value = std::string(*temp);
        r->SetInt64(message, field, std::stoll(value, nullptr, 10));
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
          uint32 hi = n64_array->Get(0)->Uint32Value(),
                 lo = n64_array->Get(1)->Uint32Value();
          n64 = ((uint64)hi << 32) + (uint64)lo;
          r->AddUInt64(message, field, n64);
        } else if (preserve_int64 && val->IsString()) {
          String::Utf8Value temp(val->ToString());
          std::string value = std::string(*temp);
          r->AddUInt64(message, field, std::stoull(value, nullptr, 10));
        } else
          r->AddUInt64(message, field, val->NumberValue());
      else if (preserve_int64 && val->IsArray()) {
        Local<Object> n64_array = val->ToObject();
        uint64 n64;
        uint32 hi = n64_array->Get(0)->Uint32Value(),
               lo = n64_array->Get(1)->Uint32Value();
        n64 = ((uint64)hi << 32) + (uint64)lo;
        r->SetUInt64(message, field, n64);
      } else if (preserve_int64 && val->IsString()) {
        String::Utf8Value temp(val->ToString());
        std::string value = std::string(*temp);
        r->SetUInt64(message, field, std::stoull(value, nullptr, 10));
      } else {
        r->SetUInt64(message, field, val->NumberValue());
      }
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
      // TODO: possible memory leak?
      enumValue =
          val->IsNumber()
              ? field->enum_type()->FindValueByNumber(val->Int32Value())
              : field->enum_type()->FindValueByName(*String::Utf8Value(val));

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
          SerializePart(r->AddMessage(message, field), val.As<Object>(),
                        preserve_int64);
        else
          SerializePart(r->MutableMessage(message, field), val.As<Object>(),
                        preserve_int64);
      }
      break;
    case FieldDescriptor::CPPTYPE_STRING:
      if (Buffer::HasInstance(val)) {
        Local<Object> buf = val->ToObject();
        if (repeated)
          r->AddString(message, field,
                       std::string(Buffer::Data(buf), Buffer::Length(buf)));
        else
          r->SetString(message, field,
                       std::string(Buffer::Data(buf), Buffer::Length(buf)));
        break;
      }

      if (val->IsObject()) {
        Local<Object> val2 = val->ToObject();
        Local<Value> converter =
            val2->Get(Nan::New<String>("toProtobuf").ToLocalChecked());
        if (converter->IsFunction()) {
          Local<Function> toProtobuf = Local<Function>::Cast(converter);
          Local<Value> ret = toProtobuf->Call(val2, 0, NULL);
          if (Buffer::HasInstance(ret)) {
            Local<Object> buf = ret->ToObject();
            if (repeated)
              r->AddString(message, field,
                           std::string(Buffer::Data(buf), Buffer::Length(buf)));
            else
              r->SetString(message, field,
                           std::string(Buffer::Data(buf), Buffer::Length(buf)));
            break;
          }
        }
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

int SerializePart(google::protobuf::Message *message, Local<Object> subj,
                  bool preserve_int64) {
  // get a reflection
  const Reflection *r = message->GetReflection();
  const Descriptor *d = message->GetDescriptor();

  for (int i = 0; i < d->field_count(); i++) {
    const FieldDescriptor *field = d->field(i);

    Local<String> fieldKey =
        Nan::New<String>(field->name().c_str()).ToLocalChecked();

    Nan::MaybeLocal<Value> maybeVal = Nan::Get(subj, fieldKey);

    if (maybeVal.IsEmpty() && field->is_required()) {
      return -1;
    }

    Local<Value> val = maybeVal.ToLocalChecked();

    if (val->IsUndefined() || val->IsNull()) {
      if (field->is_required()) {
        return -1;
      } else {
        continue;
      }
    }

    if (field->is_repeated()) {
      uint32_t len;
      Local<Object> array;
      if (val->IsArray()) {
        len = val.As<Array>()->Length();
#if NODE_MAJOR_VERSION > 0 || NODE_MINOR_VERSION > 11
      } else if (val->IsTypedArray()) {
        len = val.As<TypedArray>()->Length();
#endif
      } else {
        continue;
      }
      array = val.As<Object>();
      for (uint32_t i = 0; i < len; i++)
        SerializeField(message, r, field, array->Get(i), preserve_int64);
    } else {
      SerializeField(message, r, field, val, preserve_int64);
    }

  }

  return 0;
}
