#include "Graphics/Common.h"

#include <algorithm>

namespace Graphics {
	HandleBase::HandleBase(const std::vector<HandleBase*>& parents)
	    : m_Parents(parents) {
		for (auto parent : m_Parents)
			parent->m_Children.push_back(this);
	}

	HandleBase::~HandleBase() {
		for (auto parent : m_Parents)
			std::erase(parent->m_Children, this);
	}
} // namespace Graphics