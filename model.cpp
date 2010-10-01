#include <model.hpp>

void Model::add_attribute(int type, const char * identifier, int attribute_size, int array_size)
{
	message.attr.push_back(Attribute(type, identifier, attribute_size, array_size));
}

void Model::add_message(const char * identifier)
{
	message.identifier = identifier;
	module.msg.push_back(message);
	message = Message();
}

void Model::add_module(const char * identifier)
{
	module.identifier = identifier;
	modules.push_back(module);
	module = Module();
}

