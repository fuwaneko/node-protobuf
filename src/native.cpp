#include "native.h"
#include "parse.h"
#include "serialize.h"

Persistent<Function> constructor;

NativeProtobuf::NativeProtobuf(DescriptorPool *pool, std::vector<std::string> info): pool(pool), info(info) {}

void NativeProtobuf::Init(Handle<Object> exports) {
	// constructor
	Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);

	tpl->SetClassName(NanNew<String>("NativeProtobuf"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// prototype
	NODE_SET_PROTOTYPE_METHOD(tpl, "parse", NativeProtobuf::Parse);
	NODE_SET_PROTOTYPE_METHOD(tpl, "serialize", NativeProtobuf::Serialize);
	NODE_SET_PROTOTYPE_METHOD(tpl, "info", NativeProtobuf::Info);

	NanAssignPersistent(constructor, tpl->GetFunction());
	exports->Set(NanNew<String>("native"), tpl->GetFunction());
}

NAN_METHOD(NativeProtobuf::New) {
	NanScope();

	Local<Object> buffer_obj = args[0]->ToObject();
	char *buffer_data = Buffer::Data(buffer_obj);
	size_t buffer_length = Buffer::Length(buffer_obj);

	FileDescriptorSet descriptors;
	if (!descriptors.ParseFromArray(buffer_data, buffer_length))
		return NanThrowError("Malformed descriptor");
	
	std::vector<std::string> info;
	DescriptorPool* pool = new DescriptorPool;
	for (int i = 0; i < descriptors.file_size(); i++) {
		const FileDescriptor *f = pool->BuildFile(descriptors.file(i));
		for (int i = 0; i < f->message_type_count(); i++) {
			const Descriptor *d = f->message_type(i);
			const std::string name = d->full_name();

			info.push_back(name);
		}
	}

	NativeProtobuf *proto = new NativeProtobuf(pool, info);
	proto->Wrap(args.This());

	NanReturnValue(args.This());
}

NAN_METHOD(NativeProtobuf::Serialize) {
	NanScope();
	
	NativeProtobuf *self = ObjectWrap::Unwrap<NativeProtobuf>(args.This());

	// get object to serialize and name of schema
	Local<Object> subj = args[0]->ToObject();
	String::Utf8Value schemaName(args[1]->ToString());
	std::string schema_name = std::string(*schemaName);

	// create a message based on schema
	DynamicMessageFactory factory;
	const Descriptor *descriptor = self->pool->FindMessageTypeByName(schema_name);
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

NAN_METHOD(NativeProtobuf::Parse) {
	NanScope();

	NativeProtobuf *self = ObjectWrap::Unwrap<NativeProtobuf>(args.This());

	Local<Object> buffer_obj = args[0]->ToObject();
	char *buffer_data = Buffer::Data(buffer_obj);
	size_t buffer_length = Buffer::Length(buffer_obj);

	String::Utf8Value schemaName(args[1]->ToString());
	std::string schema_name = std::string(*schemaName);

	// create a message based on schema
	DynamicMessageFactory factory;
	const Descriptor *descriptor = self->pool->FindMessageTypeByName(schema_name);
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

NAN_METHOD(NativeProtobuf::Info) {
	NanScope();

	NativeProtobuf *self = ObjectWrap::Unwrap<NativeProtobuf>(args.This());
	Local<Array> array= NanNew<Array>();

	for (unsigned long i = 0; i < self->info.size(); i++)
		array->Set(i, NanNew<String>(self->info.at(i).c_str()));


	NanReturnValue(array);
}