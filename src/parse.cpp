#include "common.h"
#include "parse.h"

Handle<Value> ParseField(const google::protobuf::Message &message, const Reflection *r, const FieldDescriptor *field, int index) {
	Handle<Value> v;

	switch (field->cpp_type()) {
		case FieldDescriptor::CPPTYPE_INT32: {
			int32 value;
			if (index >= 0)
				value = r->GetRepeatedInt32(message, field, index);
			else
				value = r->GetInt32(message, field);
			v = NanNew<Number>(value);
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
				Local<Array> t = NanNew<Array>(2);
				t->Set(0, NanNew<Number>(hi));
				t->Set(1, NanNew<Number>(lo));
				v = t;
			} else
				v = NanNew<Number>(value);
			break;
		}
		case FieldDescriptor::CPPTYPE_UINT32: {
			uint32 value;
			if (index >= 0)
				value = r->GetRepeatedUInt32(message, field, index);
			else
				value = r->GetUInt32(message, field);
			v = NanNew<Number>(value);
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
				Local<Array> t = NanNew<Array>(2);
				t->Set(0, NanNew<Number>(hi));
				t->Set(1, NanNew<Number>(lo));
				v = t;
			} else
				v = NanNew<Number>(value);
			break;
		}
		case FieldDescriptor::CPPTYPE_DOUBLE: {
			double value;
			if (index >= 0)
				value = r->GetRepeatedDouble(message, field, index);
			else
				value = r->GetDouble(message, field);
			v = NanNew<Number>(value);
			break;
		}
		case FieldDescriptor::CPPTYPE_FLOAT: {
			float value;
			if (index >= 0)
				value = r->GetRepeatedFloat(message, field, index);
			else
				value = r->GetFloat(message, field);
			v = NanNew<Number>(value);
			break;
		}
		case FieldDescriptor::CPPTYPE_BOOL: {
			bool value;
			if (index >= 0)
				value = r->GetRepeatedBool(message, field, index);
			else
				value = r->GetBool(message, field);
			v = NanNew<Boolean>(value);
			break;
		}
		case FieldDescriptor::CPPTYPE_ENUM: {
			if (index >= 0)
				v = NanNew<String>(r->GetRepeatedEnum(message, field, index)->name().c_str());
			else
				v = NanNew<String>(r->GetEnum(message, field)->name().c_str());
			break;
		}
		case FieldDescriptor::CPPTYPE_MESSAGE: {
			if (field->is_optional() && !r->HasField(message, field))
				v = NanNull();
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
				v = NanNewBufferHandle(const_cast<char *>(value.data()), value.length());
			else
				v = NanNew<String>(value.c_str());
			break;
		}
	}

	return v;
}

Handle<Object> ParsePart(const google::protobuf::Message &message) {
	Handle<Object> ret = NanNew<Object>();
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
				Handle<Array> array = NanNew<Array>(size);
				for (int i = 0; i < size; i++) {
					array->Set(i, ParseField(message, r, field, i));
				}
				v = array;
			} else {
				v = ParseField(message, r, field, -1);
			}

			if (field->is_optional() && (v->IsNull() || !r->HasField(message, field)))
				continue;
			
			ret->Set(NanNew<String>(field->name().c_str()), v);
		}
	}

	return ret;
}
