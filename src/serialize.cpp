#include "common.h"

#include "serialize.h"

void SerializeField(google::protobuf::Message *message, const Reflection *r, const FieldDescriptor *field, Handle<Value> val, bool preserve_int64) {
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
						Handle<Object> n64_array = val->ToObject();
						uint64 n64;
						uint32 hi = n64_array->Get(0)->Uint32Value(), lo = n64_array->Get(1)->Uint32Value();
						n64 = ((uint64)hi << 32) + (uint64)lo;
						r->AddInt64(message, field, n64);
					} else if (preserve_int64 && val->IsString()) {
						String::Utf8Value temp(val->ToString());
						std::string value = std::string(*temp);
						r->AddInt64(message, field, std::stoll(value, nullptr, 10));
					} else
						r->AddInt64(message, field, val->NumberValue());
				else
					if (preserve_int64 && val->IsArray()) {
						Handle<Object> n64_array = val->ToObject();
						uint64 n64;
						uint32 hi = n64_array->Get(0)->Uint32Value(), lo = n64_array->Get(1)->Uint32Value();
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
						Handle<Object> n64_array = val->ToObject();
						uint64 n64;
						uint32 hi = n64_array->Get(0)->Uint32Value(), lo = n64_array->Get(1)->Uint32Value();
						n64 = ((uint64)hi << 32) + (uint64)lo;
						r->AddUInt64(message, field, n64);
					} else if (preserve_int64 && val->IsString()) {
						String::Utf8Value temp(val->ToString());
						std::string value = std::string(*temp);
						r->AddUInt64(message, field, std::stoull(value, nullptr, 10));
					} else
						r->AddUInt64(message, field, val->NumberValue());
				else
					if (preserve_int64 && val->IsArray()) {
						Handle<Object> n64_array = val->ToObject();
						uint64 n64;
						uint32 hi = n64_array->Get(0)->Uint32Value(), lo = n64_array->Get(1)->Uint32Value();
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
					val->IsNumber() ?
						field->enum_type()->FindValueByNumber(val->Int32Value()) :
						field->enum_type()->FindValueByName(*String::Utf8Value(val));

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
						SerializePart(r->AddMessage(message, field), val.As<Object>(), preserve_int64);
					else
						SerializePart(r->MutableMessage(message, field), val.As<Object>(), preserve_int64);
				}
				break;
			case FieldDescriptor::CPPTYPE_STRING:
				if (Buffer::HasInstance(val)) {
					Handle<Object> buf = val->ToObject();
					if (repeated)
						r->AddString(message, field, std::string(Buffer::Data(buf), Buffer::Length(buf)));
					else
						r->SetString(message, field, std::string(Buffer::Data(buf), Buffer::Length(buf)));
					break;
				}

				if (val->IsObject()) {
					Handle<Object> val2 = val->ToObject();
					Handle<Value> converter = val2->Get(Nan::New<String>("toProtobuf").ToLocalChecked());
					if (converter->IsFunction()) {
						Handle<Function> toProtobuf = Handle<Function>::Cast(converter);
						Handle<Value> ret = toProtobuf->Call(val2,0,NULL);
						if (Buffer::HasInstance(ret)) {
							Handle<Object> buf = ret->ToObject();
							if (repeated)
								r->AddString(message, field, std::string(Buffer::Data(buf), Buffer::Length(buf)));
							else
								r->SetString(message, field, std::string(Buffer::Data(buf), Buffer::Length(buf)));
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

int SerializePart(google::protobuf::Message *message, Handle<Object> subj, bool preserve_int64) {
	// get a reflection
	const Reflection *r = message->GetReflection();
	const Descriptor *d = message->GetDescriptor();

	// build a list of required properties
	vector<string> required;
	for (int i = 0; i < d->field_count(); i++) {
		const FieldDescriptor *field = d->field(i);
		if (field->is_required())
			required.push_back(field->name());
	}

	// build a reflection
	// get properties of passed object
	Handle<Array> properties = subj->GetPropertyNames();
	uint32_t len = properties->Length();

	// check that all required properties are present
	for (uint32_t i = 0; i < required.size(); i++) {
		Handle<String> key = Nan::New<String>(required.at(i).c_str()).ToLocalChecked();
		if (!subj->Has(key))
			return -1;
	}

	for (uint32_t i = 0; i < len; i++) {
		Handle<Value> property = properties->Get(i);
		Handle<String> property_s = property->ToString();

		if (*property_s == NULL)
			continue;

		String::Utf8Value temp(property);
		std::string propertyName = std::string(*temp);

		const FieldDescriptor *field = d->FindFieldByName(propertyName);
		if (field == NULL) continue;

		Handle<Value> val = subj->Get(property);

		if (field->is_repeated()) {
			if (!val->IsArray())
				continue;

			Handle<Array> array = val.As<Array>();
			int len = array->Length();

			for (int i = 0; i < len; i++)
				SerializeField(message, r, field, array->Get(i), preserve_int64);

		} else {
			SerializeField(message, r, field, val, preserve_int64);
		}
	}

	return 0;
}
