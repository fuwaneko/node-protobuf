#include "common.h"
#include "parse.h"

Handle<Value> ParseField(const google::protobuf::Message &message, const Reflection *r, const FieldDescriptor *field, int index) {
	NanScope();

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

	NanReturnValue(v);
}

Handle<Object> ParsePart(const google::protobuf::Message &message) {
	NanScope();

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

	NanReturnValue(ret);
}

NAN_METHOD(Parse) {
	NanScope();

	Local<Value> p_pool = args[0]->ToObject()->GetInternalField(0);
	DescriptorPool *pool = static_cast<DescriptorPool*>(External::Unwrap(p_pool));

	Local<Object> buffer_obj = args[1]->ToObject();
	char *buffer_data = Buffer::Data(buffer_obj);
	size_t buffer_length = Buffer::Length(buffer_obj);

	String::Utf8Value schemaName(args[2]->ToString());
	std::string schema_name = std::string(*schemaName);

	// create a message based on schema
	DynamicMessageFactory factory;
	const Descriptor *descriptor = pool->FindMessageTypeByName(schema_name);
	if (descriptor == NULL) {
		NanThrowError(("Unknown schema name: " + schema_name).c_str());
		NanReturnNull();
	}
	google::protobuf::Message *message = factory.GetPrototype(descriptor)->New();

	bool parseResult = message->ParseFromArray(buffer_data, buffer_length);

	if (parseResult) {
		Handle<Object> ret = ParsePart(*message);
		delete message;
		NanReturnValue(ret);
	} else {
		NanThrowError("Malformed protocol buffer");
		NanReturnNull();
	}
}