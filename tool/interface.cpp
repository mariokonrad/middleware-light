#include <model.hpp>
#include <interface.tab.h>
#include <iostream>
#include <fstream>
#include <map>
#include <stdint.h>

extern int yyparse();

Model model;

static bool generate_dispatch_per_message_type = true;

struct TypeInfo {
	TypeInfo() {}
	TypeInfo(const std::string & name, unsigned int size) : name(name), size(size) {}

	std::string name;
	unsigned int size;
};

static std::map<int, TypeInfo> types;

class Indent
{
	private:
		int level;
	public:
		Indent() : level(0) {}
		void reset() { level = 0; }
		Indent & operator++() { ++level; return *this; }
		Indent & operator--() { --level; return *this; }
		Indent operator++(int) { Indent obj(*this); ++level; return obj; }
		Indent operator--(int) { Indent obj(*this); --level; return obj; }
		friend std::ostream & operator<<(std::ostream & os, const Indent & indent);
};

std::ostream & operator<<(std::ostream & os, const Indent & indent)
{
	for (int i = 0; i < indent.level; ++i) os << "\t";
	return os;
}

static void write_header_message_factory(std::ostream & ofs, Indent & indent, const Model::Module &) // {{{
{
	using std::endl;

	ofs << indent << "class DefaultMessageFactory : public mwl::MessageFactory" << endl;
	ofs << indent << "{" << endl;
	++indent;
	ofs << indent << "public:" << endl;
	++indent;
	ofs << indent << "virtual ~DefaultMessageFactory();" << endl;
	ofs << indent << "virtual mwl::Message * create_message();" << endl;
	ofs << indent << "virtual void dispose_message(mwl::Message *);" << endl;
	--indent;
	--indent;
	ofs << indent << "};" << endl;
} // }}}

static void write_header_interface(std::ostream & ofs, Indent & indent, const Model::Module & module) // {{{
{
	using std::endl;

	ofs << indent << "class ModuleInterface" << endl;
	ofs << indent << "{" << endl;

	if (generate_dispatch_per_message_type) {
		++indent;
		ofs << indent << "protected:" << endl;
		++indent;
		for (Model::Module::Messages::const_iterator message = module.msg.begin(); message != module.msg.end(); ++message) {
			ofs << indent << "virtual void recv(const mwl::Head &, const " << message->identifier << " &) = 0;" << endl;
		}
		--indent;
		ofs << indent << "protected:" << endl;
		++indent;
		ofs << indent << "virtual void dispatch(const mwl::Head &, const uint8_t *);" << endl;
		--indent;
		ofs << indent << "private:" << endl;
		++indent;
		ofs << indent << "template <class T> inline void dispatch(const mwl::Head & head, const uint8_t * buf)" << endl;
		ofs << indent << "{" << endl;
		++indent;
		ofs << indent << "T msg;" << endl;
		ofs << indent << "deserialize(msg, buf);" << endl;
		ofs << indent << "ntoh(msg);" << endl;
		ofs << indent << "recv(head, msg);" << endl;
		--indent;
		ofs << indent << "}" << endl;
		--indent;
	}
	ofs << indent << "public:" << endl;
	++indent;
	ofs << indent << "virtual ~ModuleInterface() {}" << endl;
	--indent;
	--indent;
	ofs << indent << "};" << endl;
} // }}}

static void write_header_stub(std::ostream & ofs, Indent & indent, const Model::Module &) // {{{
{
	using std::endl;

	ofs << indent << "class ModuleStub" << endl;
	ofs << indent << "{" << endl;
	++indent;
	ofs << indent << "private:" << endl;
	++indent;
	ofs << indent << "mwl::Channel * channel;" << endl;
	--indent;
	ofs << indent << "public:" << endl;
	++indent;
	ofs << indent << "ModuleStub(mwl::Channel * channel) : channel(channel) {}" << endl;
	ofs << indent << "virtual ~ModuleStub() {}" << endl;
	ofs << indent << "template <class T> inline int send(const T & msg)" << endl;
	ofs << indent << "{" << endl;
	++indent;
	ofs << indent << "if (!channel) return -1;" << endl;
	ofs << indent << "uint8_t buf[sizeof(T)];" << endl;
	ofs << indent << "T clone_msg(msg);" << endl;
	ofs << indent << "hton(clone_msg);" << endl;
	ofs << indent << "serialize(buf, clone_msg);" << endl;
	ofs << indent << "return channel->send(mwl::Head(0, 0, T::TYPE, sizeof(T)), buf, sizeof(T)); // TODO:src,dst" << endl;
	--indent;
	ofs << indent << "}" << endl;
	--indent;
	--indent;
	ofs << indent << "};" << endl;
} // }}}

static void write_header_module(std::ostream & ofs, Indent & indent, const Model::Module & module) // {{{
{
	using std::endl;

	ofs << indent << "namespace " << module.identifier << " {" << endl;
	++indent;
	unsigned int max_body_size = 0;
	for (Model::Module::Messages::const_iterator message = module.msg.begin(); message != module.msg.end(); ++message) {
		unsigned int body_size = 0;
		for (Model::Message::Attributes::const_iterator attr = message->attr.begin(); attr != message->attr.end(); ++attr) {
			body_size += types[attr->type].size;
			if (attr->attribute_size > 0) {
				body_size += attr->attribute_size * attr->array_size;
			}
		}
		if (body_size > max_body_size) max_body_size = body_size;
	}
	max_body_size += 8 - (max_body_size % 8); // 8 byte alignment
	ofs << indent << "enum { MAX_BODY_SIZE = " << max_body_size << " };" << endl;
	for (Model::Module::Messages::const_iterator message = module.msg.begin(); message != module.msg.end(); ++message) {
		ofs << indent << "struct " << message->identifier << " {" << endl;
		++indent;
		ofs << indent << "enum { TYPE = " << message->type << " };" << endl;
		for (Model::Message::Attributes::const_iterator attr = message->attr.begin(); attr != message->attr.end(); ++attr) {
			ofs << indent << types[attr->type].name << " " << attr->identifier;
			if (attr->type == STRING) ofs << "[" << attr->attribute_size << "]";
			if (attr->array_size > 1) ofs << "[" << attr->array_size << "]";
			ofs << ";" << endl;
		}
		--indent;
		ofs << indent << "} __attribute__((packed));" << endl;
		ofs << indent << "void hton(struct " << message->identifier << " &);" << endl;
		ofs << indent << "void ntoh(struct " << message->identifier << " &);" << endl;
		ofs << indent << "int serialize(uint8_t *, const struct " << message->identifier << " &);" << endl;
		ofs << indent << "int deserialize(struct " << message->identifier << " &, const uint8_t *);" << endl;
	}

	write_header_message_factory(ofs, indent, module);
	write_header_interface(ofs, indent, module);
	write_header_stub(ofs, indent, module);

	--indent;
	ofs << indent << "}" << endl;
} // }}}

static void write_header() // {{{
{
	using std::endl;

	std::ofstream ofs("test.hpp");
	Indent indent;
	ofs << indent << "#include <stdint.h>" << endl;
	ofs << indent << "#include <mwl/Message.hpp>" << endl;
	ofs << indent << "#include <mwl/MessageFactory.hpp>" << endl;
	ofs << indent << "#include <mwl/Channel.hpp>" << endl;
	for (Model::Modules::const_iterator module = model.modules.begin(); module != model.modules.end(); ++module) {
		write_header_module(ofs, indent, *module);
	}
} // }}}

static void write_source_message_factory(std::ostream & ofs, Indent & indent, const Model::Module &) // {{{
{
	using std::endl;

	ofs << indent << "DefaultMessageFactory::~DefaultMessageFactory()" << endl;
	ofs << indent << "{}" << endl;
	ofs << indent << "mwl::Message * DefaultMessageFactory::create_message()" << endl;
	ofs << indent << "{" << endl;
	++indent;
	ofs << indent << "mwl::Message * msg = new mwl::Message;" << endl;
	ofs << indent << "msg->size = MAX_BODY_SIZE;" << endl;
	ofs << indent << "msg->buf = (msg->size == 0) ? NULL : new uint8_t[msg->size];" << endl;
	ofs << indent << "return msg;" << endl;
	--indent;
	ofs << indent << "}" << endl;
	ofs << indent << "void DefaultMessageFactory::dispose_message(mwl::Message * msg)" << endl;
	ofs << indent << "{" << endl;
	++indent;
	ofs << indent << "if (!msg) return;" << endl;
	ofs << indent << "if (msg->buf) {" << endl;
	++indent;
	ofs << indent << "delete [] msg->buf;" << endl;
	ofs << indent << "msg->buf = NULL;" << endl;
	--indent;
	ofs << indent << "}" << endl;
	ofs << indent << "delete msg;" << endl;
	--indent;
	ofs << indent << "}" << endl;
} // }}}

static void write_source_interface_dispatch(std::ostream & ofs, Indent & indent, const Model::Module & module) // {{{
{
	using std::endl;

	ofs << indent << "void ModuleInterface::dispatch(const mwl::Head & head, const uint8_t * buf)" << endl;
	ofs << indent << "{" << endl;
	++indent;
	ofs << indent << "switch (head.type) {" << endl;
	++indent;
	for (Model::Module::Messages::const_iterator message = module.msg.begin(); message != module.msg.end(); ++message) {
		ofs << indent << "case " << message->identifier << "::TYPE: dispatch<" << message->identifier << ">(head, buf); break;" << endl;
	}
	ofs << indent << "default: break;" << endl;
	--indent;
	ofs << indent << "}" << endl;
	--indent;
	ofs << indent << "}" << endl;

} // }}}

static void write_source_message_hton(std::ostream & ofs, Indent & indent, const Model::Message & message) // {{{
{
	using std::endl;

	ofs << indent << "void hton(struct " << message.identifier << " &" << (message.attr.empty() ? "" : " m") << ")" << endl;
	ofs << indent << "{" << endl;
	++indent;
	for (Model::Message::Attributes::const_iterator attr = message.attr.begin(); attr != message.attr.end(); ++attr) {
		if (attr->type == STRING) continue;
		std::string array_index = "";
		ofs << indent;
		if (attr->array_size > 1) {
			array_index = "[i]";
			ofs << "for (unsigned int i = 0; i < " << attr->array_size << "; ++i) ";
		}
		ofs << "::endian::hton(m." << attr->identifier << array_index << ")" << ";" << endl;
	}
	--indent;
	ofs << indent << "}" << endl;
} // }}}

static void write_source_message_ntoh(std::ostream & ofs, Indent & indent, const Model::Message & message) // {{{
{
	using std::endl;

	ofs << indent << "void ntoh(struct " << message.identifier << " &" << (message.attr.empty() ? "" : " m") << ")" << endl;
	ofs << indent << "{" << endl;
	++indent;
	for (Model::Message::Attributes::const_iterator attr = message.attr.begin(); attr != message.attr.end(); ++attr) {
		if (attr->type == STRING) continue;
		std::string array_index = "";
		ofs << indent;
		if (attr->array_size > 1) {
			array_index = "[i]";
			ofs << "for (unsigned int i = 0; i < " << attr->array_size << "; ++i) ";
		}
		ofs << "::endian::ntoh(m." << attr->identifier << array_index << ")" << ";" << endl;
	}
	--indent;
	ofs << indent << "}" << endl;
} // }}}

static void write_source_message_serialize(std::ostream & ofs, Indent & indent, const Model::Message & message) // {{{
{
	using std::endl;

	ofs << indent << "int serialize(uint8_t * buf, const struct " << message.identifier << " &" << (message.attr.empty() ? "" : " m") << ")" << endl;
	ofs << indent << "{" << endl;
	++indent;
	ofs << indent << "if (!buf) return -1;" << endl;
	for (Model::Message::Attributes::const_iterator attr = message.attr.begin(); attr != message.attr.end(); ++attr) {
		std::string array_index = "";
		if (attr->array_size > 1) {
			array_index = "[i]";
			ofs << indent << "for (unsigned int i = 0; i < " << attr->array_size << "; ++i) {" << endl;
			++indent;
		}
		if (attr->type == STRING) {
			ofs << indent << "memcpy(buf, m." << attr->identifier << array_index << ", " << attr->attribute_size << ");" << endl;
			ofs << indent << "buf += " << attr->attribute_size << ";" << endl;
		} else {
			ofs << indent << "memcpy(buf, &m." << attr->identifier << array_index << ", sizeof(m." << attr->identifier << array_index << "));" << endl;
			ofs << indent << "buf += sizeof(m." << attr->identifier << array_index << ");" << endl;
		}
		if (attr->array_size > 1) {
			--indent;
			ofs << indent << "}" << endl;
		}
	}
	ofs << indent << "return 0;" << endl;
	--indent;
	ofs << indent << "}" << endl;
} // }}}

static void write_source_message_deserialize(std::ostream & ofs, Indent & indent, const Model::Message & message) // {{{
{
	using std::endl;

	ofs << indent << "int deserialize(struct " << message.identifier << " & m, const uint8_t * buf)" << endl;
	ofs << indent << "{" << endl;
	++indent;
	ofs << indent << "if (!buf) return -1;" << endl;
	for (Model::Message::Attributes::const_iterator attr = message.attr.begin(); attr != message.attr.end(); ++attr) {
		std::string array_index = "";
		if (attr->array_size > 1) {
			array_index = "[i]";
			ofs << indent << "for (unsigned int i = 0; i < " << attr->array_size << "; ++i) {" << endl;
			++indent;
		}
		if (attr->type == STRING) {
			ofs << indent << "memcpy(m." << attr->identifier << array_index << ", buf, " << attr->attribute_size << ");" << endl;
			ofs << indent << "buf += " << attr->attribute_size << ";" << endl;
		} else {
			ofs << indent << "memcpy(&m." << attr->identifier << array_index << ", buf, sizeof(m." << attr->identifier << array_index << "));" << endl;
			ofs << indent << "buf += sizeof(m." << attr->identifier << array_index << ");" << endl;
		}
		if (attr->array_size > 1) {
			--indent;
			ofs << indent << "}" << endl;
		}
	}
	ofs << indent << "return 0;" << endl;
	--indent;
	ofs << indent << "}" << endl;
} // }}}

static void write_source_message(std::ostream & ofs, Indent & indent, const Model::Message & message) // {{{
{
	write_source_message_hton(ofs, indent, message);
	write_source_message_ntoh(ofs, indent, message);
	write_source_message_serialize(ofs, indent, message);
	write_source_message_deserialize(ofs, indent, message);
} // }}}

static void write_source_module(std::ostream & ofs, Indent & indent, const Model::Module & module) // {{{
{
	using std::endl;

	ofs << indent << "namespace " << module.identifier << " {" << endl;

	for (Model::Module::Messages::const_iterator message = module.msg.begin(); message != module.msg.end(); ++message) {
		write_source_message(ofs, indent, *message);
	}

	write_source_interface_dispatch(ofs, indent, module);
	write_source_message_factory(ofs, indent, module);

	ofs << indent << "}" << endl;
} // }}}

static void write_source() // {{{
{
	using std::endl;
	std::ofstream ofs("test.cpp");
	Indent indent;
	ofs << indent << "#include <cstring>" << endl;
	ofs << indent << "#include <mwl/endian.hpp>" << endl;
	ofs << indent << "#include <test.hpp>" << endl;
	for (Model::Modules::const_iterator module = model.modules.begin(); module != model.modules.end(); ++module) {
		write_source_module(ofs, indent, *module);
	}
} // }}}

int main(int, char **)
{
	yyparse();

	types[INT8]   = TypeInfo("int8_t",   sizeof(int8_t));
	types[INT16]  = TypeInfo("int16_t",  sizeof(int16_t));
	types[INT32]  = TypeInfo("int32_t",  sizeof(int32_t));
	types[INT64]  = TypeInfo("int64_t",  sizeof(int64_t));
	types[UINT8]  = TypeInfo("uint8_t",  sizeof(uint8_t));
	types[UINT16] = TypeInfo("uint16_t", sizeof(uint16_t));
	types[UINT32] = TypeInfo("uint32_t", sizeof(uint32_t));
	types[UINT64] = TypeInfo("uint64_t", sizeof(uint64_t));
	types[FLOAT]  = TypeInfo("float",    sizeof(float));
	types[DOUBLE] = TypeInfo("double",   sizeof(double));
	types[STRING] = TypeInfo("char",     sizeof(char));

	// TODO: seriailze/deserialize buffer: derived from std::streambuf / std::basic_streambuf<...>

	write_header();
	write_source();
	return 0;
}

