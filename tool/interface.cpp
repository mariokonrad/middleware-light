#include <model.hpp>
#include <interface.tab.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <stdint.h>
#include <getopt.h>

static bool param_generate_dispatch_per_message_type = true;
static bool param_help = false;
static bool param_check = false;
static bool param_generate_stub = true;
static bool param_generate_factory = true;
static bool param_generate_interface = true;
static std::string input_file;
static std::string output_name;

static int parse_options(int argc, char ** argv)
{
	enum Parameter {
		 PARAM_HELP = 500
		,PARAM_CHECK
		,PARAM_OUTPUT_NAME
		,PARAM_DISPATCH_PER_MESSAGE
		,PARAM_GEN_STUB
		,PARAM_GEN_FACTORY
		,PARAM_GEN_INTERFACE
	};

	static const char * OPTIONS = "";
	static const struct option LONG_OPTIONS[] = {
		{ "help",             no_argument,       NULL, PARAM_HELP                 },
		{ "check",            no_argument,       NULL, PARAM_CHECK                },
		{ "output",           required_argument, NULL, PARAM_OUTPUT_NAME          },
		{ "dispatch-per-msg", required_argument, NULL, PARAM_DISPATCH_PER_MESSAGE },
		{ "gen-stub",         required_argument, NULL, PARAM_GEN_STUB             },
		{ "gen-factory",      required_argument, NULL, PARAM_GEN_FACTORY          },
		{ "gen-interface",    required_argument, NULL, PARAM_GEN_INTERFACE        },
		{ NULL,               no_argument,       NULL, 0                          },
	};
	for (optind = 1; optind < argc; ) {
		int index = -1;
		if (getopt_long(argc, argv, OPTIONS, LONG_OPTIONS, &index) < 0) break;
		if (index < 0) return -1;
		const struct option * opt = static_cast<const struct option *>(&LONG_OPTIONS[index]);
		switch (static_cast<Parameter>(opt->val)) {
			case PARAM_HELP:
				param_help = true;
				break;
			case PARAM_CHECK:
				param_check = true;
				break;
			case PARAM_OUTPUT_NAME:
				output_name = optarg;
				break;
			case PARAM_DISPATCH_PER_MESSAGE:
				std::istringstream(optarg) >> param_generate_dispatch_per_message_type;
				break;
			case PARAM_GEN_STUB:
				std::istringstream(optarg) >> param_generate_stub;
				break;
			case PARAM_GEN_FACTORY:
				std::istringstream(optarg) >> param_generate_factory;
				break;
			case PARAM_GEN_INTERFACE:
				std::istringstream(optarg) >> param_generate_interface;
				break;
		}
	}

	if (optind == argc) {
		// input to be read from stdin
	} else if (optind == argc - 1) {
		input_file = argv[optind];
	} else {
		std::cerr << std::endl << argv[0] << ": error: unknown parameters specified" << std::endl;
		return -1;
	}
	return 0;
}

static void usage(const char * name)
{
	using std::endl;

	std::cout
		<< endl
		<< "usage: " << name << " [options] input-file" << endl
		<< endl
		<< "Options:" << endl
		<< "\t" << "--help                     : this help" << endl
		<< "\t" << "--check                    : do not generate anything, just check the input." << endl
		<< "\t" << "                             in this case, no output name is needed" << endl
		<< "\t" << "--output=name              : file prefix for the output files (header, definition)" << endl
		<< "\t" << "--dispatch_per_msg=flag    : enable (1) or disable (0) the generation" << endl
		<< "\t" << "                             of individual receive methods per message." << endl
		<< "\t" << "                             default: 1" << endl
		<< "\t" << "--gen-stub=flag            : enable (1) or disable (0) the generation" << endl
		<< "\t" << "                             of the stub. default: 1" << endl
		<< "\t" << "--gen-factory=flag         : enable (1) or disable (0) the generation" << endl
		<< "\t" << "                             of the default factory. default: 1" << endl
		<< "\t" << "--gen-interface=flag       : enable (1) or disable (0) the generation" << endl
		<< "\t" << "                             of the module interface. default: 1" << endl
		<< endl;
}

extern int yyparse();

Model model;


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

	ofs << indent << "class ModuleInterface : public mwl::ModuleBase" << endl;
	ofs << indent << "{" << endl;

	++indent;
	if (param_generate_dispatch_per_message_type) {
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
		ofs << indent << "ModuleInterface(mwl::MessageFactory * factory) : mwl::ModuleBase(factory) {}" << endl;
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

	if (param_generate_factory) write_header_message_factory(ofs, indent, module);
	if (param_generate_interface) write_header_interface(ofs, indent, module);
	if (param_generate_stub) write_header_stub(ofs, indent, module);

	--indent;
	ofs << indent << "}" << endl;
} // }}}

static void write_header(const std::string & fn) // {{{
{
	using std::endl;

	std::ofstream ofs(fn.c_str());
	Indent indent;
	ofs << indent << "#include <stdint.h>" << endl;
	ofs << indent << "#include <mwl/Message.hpp>" << endl;
	ofs << indent << "#include <mwl/MessageFactory.hpp>" << endl;
	ofs << indent << "#include <mwl/ModuleBase.hpp>" << endl;
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

	if (param_generate_interface) write_source_interface_dispatch(ofs, indent, module);
	if (param_generate_factory) write_source_message_factory(ofs, indent, module);

	ofs << indent << "}" << endl;
} // }}}

static void write_source(const std::string & fn, const std::string & fn_header) // {{{
{
	using std::endl;
	std::ofstream ofs(fn.c_str());
	Indent indent;
	ofs << indent << "#include <cstring>" << endl;
	ofs << indent << "#include <mwl/endian.hpp>" << endl;
	ofs << indent << "#include <" << fn_header << ">" << endl;
	for (Model::Modules::const_iterator module = model.modules.begin(); module != model.modules.end(); ++module) {
		write_source_module(ofs, indent, *module);
	}
} // }}}

int main(int argc, char ** argv)
{
	FILE * old_stdin = NULL;
	FILE * file = NULL;

	if (parse_options(argc, argv) < 0) {
		usage(argv[0]);
		return -1;
	}

	if (param_help) {
		usage(argv[0]);
		return 0;
	}

	if (output_name.empty() && !param_check) {
		std::cerr << argv[0] << ": error: no output name specified." << std::endl;
		usage(argv[0]);
		return -1;
	}

	if (input_file.size()) {
		file = fopen(input_file.c_str(), "rt");
		if (file == NULL) {
			std::cerr << argv[0] << ": error: cannot open file '" << input_file << "'" << std::endl;
			return -1;
		}
		old_stdin = stdin;
		stdin = file;
	}

	yyparse();

	if (file) {
		stdin = old_stdin;
		fclose(file);
		file = NULL;
	}

	if (param_check) return 0;

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

	write_header(output_name + ".hpp");
	write_source(output_name + ".cpp", output_name + ".hpp");
	return 0;
}

