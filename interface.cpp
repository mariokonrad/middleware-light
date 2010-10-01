#include <model.hpp>
#include <interface.tab.h>
#include <iostream>
#include <fstream>
#include <map>

extern int yyparse();

Model model;
static std::map<int, std::string> types;

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

static void write_header()
{
	using std::endl;
	std::ofstream ofs("test.hpp");
	Indent indent;
	ofs << indent << "#include <stdint.h>" << endl;
	for (Model::Modules::const_iterator module = model.modules.begin(); module != model.modules.end(); ++module) {
		ofs << indent << "namespace " << module->identifier << " {" << endl;
		++indent;
		for (Model::Module::Messages::const_iterator message = module->msg.begin(); message != module->msg.end(); ++message) {
			ofs << indent << "struct " << message->identifier << " {" << endl;
			++indent;
			ofs << indent << "enum { TYPE = " << message->type << " };" << endl;
			for (Model::Message::Attributes::const_iterator attr = message->attr.begin(); attr != message->attr.end(); ++attr) {
				ofs << indent << types[attr->type] << " " << attr->identifier;
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
		--indent;
		ofs << indent << "}" << endl;
	}
}

static void write_source()
{
	using std::endl;
	std::ofstream ofs("test.cpp");
	Indent indent;
	for (Model::Modules::const_iterator module = model.modules.begin(); module != model.modules.end(); ++module) {
		ofs << indent << "#include <cstring>" << endl;
		ofs << indent << "#include <endian.hpp>" << endl;
		ofs << indent << "#include <test.hpp>" << endl;
		ofs << indent << "namespace " << module->identifier << " {" << endl;
		for (Model::Module::Messages::const_iterator message = module->msg.begin(); message != module->msg.end(); ++message) {

			// hton
			ofs << indent << "void hton(struct " << message->identifier << " & m)" << endl;
			ofs << indent << "{" << endl;
			++indent;
			for (Model::Message::Attributes::const_iterator attr = message->attr.begin(); attr != message->attr.end(); ++attr) {
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

			// ntoh
			ofs << indent << "void ntoh(struct " << message->identifier << " & m)" << endl;
			ofs << indent << "{" << endl;
			++indent;
			for (Model::Message::Attributes::const_iterator attr = message->attr.begin(); attr != message->attr.end(); ++attr) {
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

			// serialize
			ofs << indent << "int serialize(uint8_t * buf, const struct " << message->identifier << " & m)" << endl;
			ofs << indent << "{" << endl;
			++indent;
			ofs << indent << "if (!buf) return -1;" << endl;
			for (Model::Message::Attributes::const_iterator attr = message->attr.begin(); attr != message->attr.end(); ++attr) {
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

			// deserialize
			ofs << indent << "int deserialize(struct " << message->identifier << " & m, const uint8_t * buf)" << endl;
			ofs << indent << "{" << endl;
			++indent;
			ofs << indent << "if (!buf) return -1;" << endl;
			for (Model::Message::Attributes::const_iterator attr = message->attr.begin(); attr != message->attr.end(); ++attr) {
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
		}
		ofs << indent << "}" << endl;
	}
}

int main(int, char **)
{
	yyparse();

	types[INT8]   = "int8_t";
	types[INT16]  = "int16_t";
	types[INT32]  = "int32_t";
	types[INT64]  = "int64_t";
	types[UINT8]  = "uint8_t";
	types[UINT16] = "uint16_t";
	types[UINT32] = "uint32_t";
	types[UINT64] = "uint64_t";
	types[FLOAT]  = "float";
	types[DOUBLE] = "double";
	types[STRING] = "char";

	// TODO: seriailze/deserialize buffer: derived from std::streambuf / std::basic_streambuf<...>

	write_header();
	write_source();
	return 0;
}

