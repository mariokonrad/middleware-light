#ifndef __MODEL__HPP__
#define __MODEL__HPP__

#include <string>
#include <vector>

class Model
{
	public:
		struct Attribute
		{
			int type;
			std::string identifier;
			int attribute_size;
			int array_size;

			Attribute(int type, const std::string & identifier, int attribute_size, int array_size)
				: type(type)
				, identifier(identifier)
				, attribute_size(attribute_size)
				, array_size(array_size)
			{}
		};

		struct Message
		{
			typedef std::vector<Attribute> Attributes;

			std::string identifier;
			int type;
			Attributes attr;
		};

		struct Module
		{
			typedef std::vector<Message> Messages;

			std::string identifier;
			Messages msg;
		};

		typedef std::vector<Module> Modules;
	private: // working set
		Module module;
		Message message;
	public:
		Modules modules;
	public:
		void add_attribute(int, const char *, int = 0, int = 1);
		void add_message(const char *, int);
		void add_module(const char *);
};



#endif
