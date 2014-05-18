#include "common.h"

#include "parse.h"
#include "serialize.h"

std::map< DescriptorPool*, Persistent<Object> > d;

// free descriptor pool when it's not needed anymore
void freeDescriptor(Persistent<Value> object, void *parameter) {
	DescriptorPool *pool = static_cast<DescriptorPool*>(parameter);
	d.erase(pool);
	delete pool;

	object.Dispose();
	object.Clear();
}

NAN_METHOD(libraryInit) {
	NanScope();

	Local<Object> buffer_obj = args[0]->ToObject();
	char *buffer_data = Buffer::Data(buffer_obj);
	size_t buffer_length = Buffer::Length(buffer_obj);

	FileDescriptorSet descriptors;
	if (!descriptors.ParseFromArray(buffer_data, buffer_length))
		return NanThrowError("Malformed descriptor");
	
	DescriptorPool* pool = new DescriptorPool;
	for (int i = 0; i < descriptors.file_size(); i++) {
		pool->BuildFile(descriptors.file(i));
	}

	if (d.count(pool) > 0)
		return d[pool];

	Handle<FunctionTemplate> fun_template = v8::FunctionTemplate::New();
	Handle<ObjectTemplate> point_templ = fun_template->InstanceTemplate();
	point_templ->SetInternalFieldCount(1);

	Persistent<Object> p_pool = Persistent<Object>::New(point_templ->NewInstance());

	// make this handle persistent so we're able to clean up when needed
	p_pool.MakeWeak(pool, freeDescriptor);

	d[pool] = p_pool;
	p_pool->SetInternalField(0, External::New(pool));

	NanReturnValue(p_pool);
}

void init(Handle<Object> exports) {
  exports->Set(NanSymbol("init"), NanNew<FunctionTemplate>(libraryInit)->GetFunction());

  exports->Set(NanSymbol("parse"), NanNew<FunctionTemplate>(Parse)->GetFunction());
  //exports->Set(NanSymbol("parseAsync"), NanNew<FunctionTemplate>(parseAsync)->GetFunction());

  exports->Set(NanSymbol("serialize"), NanNew<FunctionTemplate>(Serialize)->GetFunction());
  //exports->Set(NanSymbol("serializeAsync"), NanNew<FunctionTemplate>(serializeAsync)->GetFunction());
}

NODE_MODULE(protobuf, init)