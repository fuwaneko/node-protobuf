#include "common.h"

#include "serialize.h"

void SerializeField(google::protobuf::Message *message, const Reflection *r, const FieldDescriptor *field, Handle<Value> val) {
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
				if (Buffer::HasInstance(val)) {
					Local<Object> buf = val->ToObject();
					if (repeated)
						r->AddString(message, field, std::string(Buffer::Data(buf), Buffer::Length(buf)));
					else
						r->SetString(message, field, std::string(Buffer::Data(buf), Buffer::Length(buf)));
					break;
				}

				if (val->IsObject()) {
					Local<Object> val2 = val->ToObject();
					Local<Value> converter = val2->Get(String::NewSymbol("toProtobuf"));
					if (converter->IsFunction()) {
						Local<Function> toProtobuf = Local<Function>::Cast(converter);
						Local<Value> ret = toProtobuf->Call(val2,0,NULL);
						if (Buffer::HasInstance(ret)) {
							Local<Object> buf = ret->ToObject();
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

int SerializePart(google::protobuf::Message *message, Handle<Object> subj) {
	NanScope();

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
	Local<Array> properties = subj->GetPropertyNames();
	uint32_t len = properties->Length();
	
	// check that all required properties are present
	for (uint32_t i = 0; i < required.size(); i++) {
		Handle<String> key = String::New(required.at(i).c_str());
		if (!subj->Has(key))
			return -1;
	}

	for (uint32_t i = 0; i < len; i++) {
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
	
	return 0;
}

NAN_METHOD(Serialize) {
	NanScope();

	Local<Value> p_pool = args[0]->ToObject()->GetInternalField(0);
	DescriptorPool *pool = static_cast<DescriptorPool*>(External::Unwrap(p_pool));

	// get object to serialize and name of schema
	Local<Object> subj = args[1]->ToObject();
	String::Utf8Value schemaName(args[2]->ToString());
	std::string schema_name = std::string(*schemaName);

	// create a message based on schema
	DynamicMessageFactory factory;
	const Descriptor *descriptor = pool->FindMessageTypeByName(schema_name);
	if (descriptor == NULL) {
		NanThrowError(("Unknown schema name: " + schema_name).c_str());
		NanReturnUndefined();
	}

	google::protobuf::Message *message = factory.GetPrototype(descriptor)->New();

	if (SerializePart(message, subj) < 0) {
		// required field not present!
		NanReturnNull();
	}

	// make JS Buffer instead of SlowBuffer
	int size = message->ByteSize();
	char *buf = new char[size];
	bool result = message->SerializeToArray(buf, size);

	if (!result) {
		return NanThrowError("Can't serialize");
	}

	Local<Object> buffer = NanNewBufferHandle(buf, size);

	delete message;
	delete[] buf;

	NanReturnValue(buffer);
}
