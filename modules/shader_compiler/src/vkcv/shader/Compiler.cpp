
#include "vkcv/shader/Compiler.hpp"

namespace vkcv::shader {
	
	std::string Compiler::getDefine(const std::string &name) const {
		return m_defines.at(name);
	}
	
	void Compiler::setDefine(const std::string &name, const std::string &value) {
		m_defines[name] = value;
	}
	
}
