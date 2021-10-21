#pragma once

#include <vulkan.hpp>

#include <vector>

namespace Graphics {
	struct Version {
	public:
		constexpr Version() : m_Version(0) { }
		constexpr Version(std::uint32_t version) : m_Version(version) { }
		constexpr Version(std::uint32_t variant, std::uint32_t major, std::uint32_t minor, std::uint32_t patch) : m_Variant(variant), m_Major(major), m_Minor(minor), m_Patch(patch) { }

		friend auto operator<=>(Version lhs, Version rhs) {
			// clang-format off
			return lhs.m_Version <=> rhs.m_Version;
			// clang-format on
		}

		operator std::uint32_t() const { return m_Version; }
		operator bool() const { return m_Version; }

		union {
			struct {
				std::uint32_t m_Patch : 12;
				std::uint32_t m_Minor : 10;
				std::uint32_t m_Major : 7;
				std::uint32_t m_Variant : 3;
			};
			std::uint32_t m_Version;
		};
	};

	struct HandleBase {
	public:
		HandleBase(const std::vector<HandleBase*>& parents = {});
		virtual ~HandleBase();

		virtual bool create()  = 0;
		virtual void destroy() = 0;

		virtual bool isCreated() const     = 0;
		virtual bool isDestroyable() const = 0;

		auto& getParents() const { return m_Parents; }
		auto& getChildren() const { return m_Children; }

	protected:
		std::vector<HandleBase*> m_Parents;
		std::vector<HandleBase*> m_Children;
	};

	template <class HandleType, bool Destroyable = true>
	struct Handle : public HandleBase {
	public:
		using HandleT = HandleType;

	public:
		virtual bool create() override;
		virtual void destroy() override;

		virtual bool isCreated() const override { return m_Handle; }
		virtual bool isDestroyable() const override { return Destroyable; }
		HandleT& getHandle() const { return m_Handle; }

	private:
		virtual void createImpl()  = 0;
		virtual bool destroyImpl() = 0;

	protected:
		HandleT m_Handle = nullptr;

	private:
		std::vector<HandleBase*> m_DestroyedChildren;
		bool m_Recreate = false;
	};

	/* Implementation */

	template <class HandleType, bool Destroyable>
	bool Handle<HandleType, Destroyable>::create() {
		bool pCreated = m_Handle;
		if (pCreated) {
			m_Recreate = true;
			destroy();
		}

		createImpl();
		if (pCreated && m_Handle) {
			for (auto& child : m_DestroyedChildren) child->create();
			m_DestroyedChildren.clear();
		}
		m_Recreate = false;
		return m_Handle;
	}

	template <class HandleType, bool Destroyable>
	void Handle<HandleType, Destroyable>::destroy() {
		if (m_Recreate)
			m_DestroyedChildren.clear();

		auto itr = m_Children.begin();
		while (itr != m_Children.end()) {
			auto m_Child = *itr;

			if (m_Child->isCreated()) {
				m_Child->destroy();

				if (m_Recreate && m_Child->isDestroyable())
					m_DestroyedChildren.push_back(m_Child);

				continue;
			}

			++itr;
		}

		if (Destroyable && m_Handle && destroyImpl()) m_Handle = nullptr;
	}
} // namespace Graphics