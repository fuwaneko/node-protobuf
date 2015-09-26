#include "native.h"
#include "parse.h"
#include "serialize.h"

Nan::Persistent<Function> constructor;

NativeProtobuf::NativeProtobuf(DescriptorPool *pool, std::vector<std::string> info): pool(pool), info(info) {}

void NativeProtobuf::Init(Local<Object> exports) {
	// constructor
	Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);

	tpl->SetClassName(Nan::New<String>("NativeProtobuf").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// prototype
	Nan::SetPrototypeMethod(tpl, "parse", NativeProtobuf::Parse);
	Nan::SetPrototypeMethod(tpl, "serialize", NativeProtobuf::Serialize);
	Nan::SetPrototypeMethod(tpl, "info", NativeProtobuf::Info);

	constructor.Reset(tpl->GetFunction());
	exports->Set(Nan::New<String>("native").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(NativeProtobuf::New) {

	Local<Object> buffer_obj = info[0]->ToObject();
	char *buffer_data = Buffer::Data(buffer_obj);
	size_t buffer_length = Buffer::Length(buffer_obj);

	FileDescriptorSet descriptors;
	if (!descriptors.ParseFromArray(buffer_data, buffer_length))
		return Nan::ThrowError("Malformed descriptor");

	std::vector<std::string> infoList;
	DescriptorPool* pool = new DescriptorPool;
	for (int i = 0; i < descriptors.file_size(); i++) {
		const FileDescriptor *f = pool->BuildFile(descriptors.file(i));
		for (int i = 0; i < f->message_type_count(); i++) {
			const Descriptor *d = f->message_type(i);
			const std::string name = d->full_name();

			infoList.push_back(name);
		}
	}

	NativeProtobuf *proto = new NativeProtobuf(pool, infoList);
	proto->Wrap(info.This());

	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(NativeProtobuf::Serialize) {

	NativeProtobuf *self = ObjectWrap::Unwrap<NativeProtobuf>(info.This());

	// get object to serialize and name of schema
	Local<Object> subj = info[0]->ToObject();
	String::Utf8Value schemaName(info[1]->ToString());
	std::string schema_name = std::string(*schemaName);

	// create a message based on schema
	DynamicMessageFactory factory;
	const Descriptor *descriptor = self->pool->FindMessageTypeByName(schema_name);
	if (descriptor == NULL) {
		Nan::ThrowError(("Unknown schema name: " + schema_name).c_str());
		info.GetReturnValue().Set(Nan::Undefined());
		return;
	}

	google::protobuf::Message *message = factory.GetPrototype(descriptor)->New();

	if (SerializePart(message, subj) < 0) {
		// required field not present!
		info.GetReturnValue().Set(Nan::Null());
		return;
	}

	// make JS Buffer instead of SlowBuffer
	int size = message->ByteSize();
	char *buf = new char[size];
	bool result = message->SerializeToArray(buf, size);

	if (!result) {
		Nan::ThrowError("Can't serialize");
		info.GetReturnValue().Set(Nan::Undefined());
		return;
	}

	Local<Object> buffer = Nan::NewBuffer(buf, size).ToLocalChecked();

	delete message;
	delete[] buf;

	info.GetReturnValue().Set(buffer);
}

NAN_METHOD(NativeProtobuf::Parse) {

	NativeProtobuf *self = ObjectWrap::Unwrap<NativeProtobuf>(info.This());

	Local<Object> buffer_obj = info[0]->ToObject();
	char *buffer_data = Buffer::Data(buffer_obj);
	size_t buffer_length = Buffer::Length(buffer_obj);

	String::Utf8Value schemaName(info[1]->ToString());
	std::string schema_name = std::string(*schemaName);

	// create a message based on schema
	DynamicMessageFactory factory;
	const Descriptor *descriptor = self->pool->FindMessageTypeByName(schema_name);
	if (descriptor == NULL) {
		Nan::ThrowError(("Unknown schema name: " + schema_name).c_str());
		info.GetReturnValue().Set(Nan::Null());
	}
	google::protobuf::Message *message = factory.GetPrototype(descriptor)->New();

	bool parseResult = message->ParseFromArray(buffer_data, buffer_length);

	if (parseResult) {
		Local<Object> ret = ParsePart(*message);
		delete message;
		info.GetReturnValue().Set(ret);
	} else {
		Nan::ThrowError("Malformed protocol buffer");
		info.GetReturnValue().Set(Nan::Null());
	}
}

NAN_METHOD(NativeProtobuf::Info) {

	NativeProtobuf *self = ObjectWrap::Unwrap<NativeProtobuf>(info.This());
	Local<Array> array= Nan::New<Array>();

	for (unsigned long i = 0; i < self->info.size(); i++)
		array->Set(i, Nan::New<String>(self->info.at(i).c_str()).ToLocalChecked());


	info.GetReturnValue().Set(array);
}
