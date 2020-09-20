//
// Created by darkboss on 8/22/20.
//

#include <iostream>
#include "Config.h"
#include "ConfigValidator.h"

namespace quasar {
	namespace core {

		DotAccess::DotAccess(ConfigNode *root_, const String &path_, unsigned int flags_)
			: flags(flags_)
			, path(path_)
			, parts(path_.split("."))
			, root(root_)
			, iter(nullptr)
			, child(nullptr)
			, propertyName()
			, property(nullptr)
		{}

		DotAccess &DotAccess::walk() {
			String prop;

			child = nullptr;
			property = nullptr;
			iter = root;
			if (iter->getName() == parts[0]) {
				parts->erase(parts.begin());
			}
			for (auto part = parts.begin(); part != parts.end(); part++) {
				if ((part + 1) == parts.end()) {
					if ((flags & DAF_NEED_PROPERTY)) {
						if ((flags & DAF_CREATE_NODES) && !iter->hasDirectProperty(*part)) {
							iter->setDirectProperty(*part, "");
						}
						if (iter->hasDirectProperty(*part)) {
							child = iter;
							propertyName = *part;
							property = &iter->mProps.at(*part);
						}
					} else {
						if (iter->getName() == *part) {
							child = iter;
						} else {
							if ((flags & DAF_CREATE_NODES) && !iter->hasDirectChild(*part)) {
								iter->addDirectChild(ConfigNode(nullptr, *part));
							}
							child = iter->getDirectChild(*part, false);
						}
					}
					break;
				}
				auto found = iter->findDirectChild(*part);
				if (found == iter->mChildren.end()) {
					if (flags & DAF_CREATE_NODES) {
						iter->addDirectChild(ConfigNode(nullptr, *part));
						found = iter->findDirectChild(*part);
					} else {
						break;
					}
				}
				iter = &*found;
			}
			return *this;
		}
		ConstDotAccess::ConstDotAccess(const ConfigNode *root_, const String &path_, unsigned int flags_)
			: flags(flags_)
			, path(path_)
			, parts(path_.split("."))
			, root(root_)
			, iter(nullptr)
			, child(nullptr)
			, propertyName()
			, property(nullptr)
		{
			if (flags & DAF_CREATE_NODES) {
				throw std::runtime_error("cannot create nodes in ConstDotAccess");
			}
		}

		ConstDotAccess &ConstDotAccess::walk() {
			String prop;

			child = nullptr;
			property = nullptr;
			iter = root;
			if (iter->getName() == parts[0]) {
				parts->erase(parts.begin());
			}
			for (auto part = parts.begin(); part != parts.end(); part++) {
				if ((part + 1) == parts.end()) {
					if ((flags & DAF_NEED_PROPERTY)) {
						if (iter->hasDirectProperty(*part)) {
							child = iter;
							propertyName = *part;
							property = &iter->mProps.at(*part);
						}
					} else {
						if (iter->getName() == *part) {
							child = iter;
						} else {
							child = iter->getDirectChild(*part, false);
						}
					}
					break;
				}
				auto found = iter->findDirectChild(*part);
				if (found == iter->mChildren.end()) {
					break;
				}
				iter = &*found;
			}
			return *this;
		}

		template<typename Type>
		Type ConfigNode::getValue() const noexcept(false) {
			StringStream buf(mValue);
			Type ret = Type();
			if (!(buf >> ret)) {
				throw std::runtime_error("cannot convert '" + mValue + "' to '" + std::string(typeid(Type).name()) + "'");
			}
			return ret;
		}

		template<typename Type>
		void ConfigNode::setValue(const Type &value) noexcept {
			StringStream ss;
			ss << value;
			mValue = ss.str();
		}

		template<> String                   ConfigNode::getValue() const noexcept(false) { return mValue; }
		template<> void                     ConfigNode::setValue(const String &value) noexcept { mValue = value; }

		template<> bool                     ConfigNode::getValue() const noexcept(false) {
			StringStream ss;
			ss << std::boolalpha << mValue;
			bool ret = false;
			ss >> ret;
			return ret;
		}
		template<> void                     ConfigNode::setValue(const bool &value) noexcept {
			StringStream ss;
			ss << std::boolalpha << value;
			mValue = ss.str();
		}

		template<> std::nullptr_t           ConfigNode::getValue() const noexcept(false) { return nullptr; }
		template<> void                     ConfigNode::setValue(const std::nullptr_t &value) noexcept { mValue = "null"; }

		ConfigNode::ConfigNode(ConfigNode *parent,
							   const String &name,
							   const child_store_type &children,
							   const prop_store_type &props,
							   const String &value)
			: mSchema(nullptr)
			, mParent(parent)
			, mChildren(children)
			, mProps(props)
			, mName(name)
			, mValue(value)
		{
			for (auto &child: mChildren) {
				acquireChild(child, this);
			}
		}

		ConfigNode::ConfigNode(const ConfigNode &rhs)
			: mSchema(nullptr)
			, mParent(nullptr)
			, mChildren()
			, mProps()
			, mName()
			, mValue()
		{
			*this = rhs;
		}

		ConfigNode &ConfigNode::operator=(const ConfigNode &rhs) {
			mSchema = rhs.mSchema;
			mParent = rhs.mParent;
			mChildren = rhs.mChildren;
			mProps = rhs.mProps;
			mName = rhs.mName;
			mValue = rhs.mValue;
			for (auto &child: mChildren) {
				acquireChild(child, this);
			}
			validate();
			return *this;
		}

		void        ConfigNode::acquireChild(ConfigNode &child, ConfigNode *parent) {
			child.setParent(parent);
			for (auto &child2: child.getChildren()) {
				acquireChild(child2, &child);
			}
		}

		const String &ConfigNode::getName() const noexcept { return mName; }

		void ConfigNode::setName(const String &name) noexcept { mName = name; }

		const ConfigNode *ConfigNode::getParent() const noexcept { return mParent; }

		ConfigNode *ConfigNode::getParent() noexcept { return mParent; }

		void ConfigNode::setParent(ConfigNode *parent) {
			mParent = parent;
		}

		bool                    ConfigNode::hasChildren() const {
			return !mChildren.empty();
		}
		ConfigNode *ConfigNode::createChild(const String &name) {
			child_iter_type child = findChild(name);
			if (child == mChildren.end()) {
				addChild(ConfigNode(nullptr, name));;
				child = mChildren.end() - 1;
			}
			return &*child;
		}

		const ConfigNode::child_store_type &ConfigNode::getChildren() const noexcept { return mChildren; }

		ConfigNode::child_store_type &ConfigNode::getChildren() noexcept { return mChildren; }

		const ConfigNode *ConfigNode::getChild(const String &name, bool except) const noexcept(false) {
			ConstDotAccess access(this, name, false);
			access.walk();
			if (access.child == nullptr && except) {
				throw std::runtime_error("'" + name + "' unknown child in ConfigNode '" + getPath() + "'");
			}
			return access.child;
		}

		const ConfigNode            *ConfigNode::getFirstChild() const {
			if (!mChildren.empty()) {
				return &mChildren->front();
			}
			return nullptr;
		}
		ConfigNode                  *ConfigNode::getFirstChild() {
			if (!mChildren.empty()) {
				return &mChildren->front();
			}
			return nullptr;
		}
		const ConfigNode            *ConfigNode::getLastChild() const {
			if (!mChildren.empty()) {
				return &mChildren->back();
			}
			return nullptr;
		}
		ConfigNode                  *ConfigNode::getLastChild() {
			if (!mChildren.empty()) {
				return &mChildren->back();
			}
			return nullptr;
		}

		ConfigNode *ConfigNode::setChild(const String &name, const ConfigNode &n) noexcept(false) {
			auto it = findChild(name);
			String prop;
			DotAccess access(this, name, DAF_CREATE_NODES);
			access.walk();
			*access.child = n;
			acquireChild(*access.child, access.child->getParent());
			return access.child;
		}

		ConfigNode *ConfigNode::getChild(const String &name, bool except) noexcept(false) {
			DotAccess access(this, name);
			access.walk();
			if (access.child == nullptr && except) {
				throw std::runtime_error("'" + name + "' unknown child in ConfigNode '" + getPath() + "'");
			}
			return access.child;
		}

		bool ConfigNode::hasChild(const String &name) const noexcept {
			return getChild(name, false) != nullptr;
		}

		bool ConfigNode::removeChild(const String &name, ConfigNode *out) {
			for (auto it = mChildren.begin(); it != mChildren.end(); it++) {
				if (it->getName() == name) {
					if (out) {
						*out = *it;
					}
					mChildren->erase(it);
					return true;
				}
			}
			return false;
		}

		const ConfigNode::prop_store_type &ConfigNode::getProperties() const noexcept { return mProps; }

		ConfigNode::prop_store_type &ConfigNode::getProperties() noexcept { return mProps; }

		bool ConfigNode::hasProperty(const String &name) const noexcept {
			auto prop = mProps->find(name);
			return prop != mProps.end();
		}

		bool ConfigNode::hasDirectProperty(const String &name) const noexcept {
			auto prop = mProps->find(name);
			return prop != mProps.end();
		}

		ConfigNode::child_iter_type ConfigNode::findChild(const String &name) {
			for (auto it = mChildren.begin(); it != mChildren.end(); it++) {
				if (it->getName() == name) {
					return it;
				}
			}
			return mChildren.end();
		}

		ConfigNode::child_citer_type ConfigNode::findChild(const String &name) const {
			for (auto it = mChildren.begin(); it != mChildren.end(); it++) {
				if (it->getName() == name) {
					return it;
				}
			}
			return mChildren.end();
		}


		ConfigNode::child_riter_type ConfigNode::rfindChild(const String &name) {
			for (auto it = mChildren.rbegin(); it != mChildren.rend(); it++) {
				if (it->getName() == name) {
					return it;
				}
			}
			return mChildren.rend();
		}

		ConfigNode::child_criter_type ConfigNode::rfindChild(const String &name) const {
			for (auto it = mChildren.crbegin(); it != mChildren.crend(); it++) {
				if (it->getName() == name) {
					return it;
				}
			}
			return mChildren.crend();
		}

		ConfigNode *ConfigNode::addChild(const ConfigNode &n) {
			if (hasChild(n.getName())) {
				throw std::runtime_error("Child '" + std::string(n.getName().begin(), n.getName().end()) + "' already exists in ConfigNode '" + std::string(mName.begin(), mName.end()) + "'");
			}
			mChildren.add(n);
			acquireChild(mChildren->back(), this);
			return &mChildren->back();
		}

		ConfigNode ConfigNode::merged(const ConfigNode &with) const {
			ConfigNode ret(*this);
			ret.merge(with);
			return ret;
		}

		String ConfigNode::getPath() const {
			Vector<String>  ret;
			const ConfigNode *cur = this;
			while (cur) {
				ret.add(cur->getName());
				cur = cur->getParent();
			}
			String sret;
			for (auto it = ret.rbegin(); it != ret.rend(); it++) {
				if (!sret.empty()) {
					sret += ".";
				}
				sret += *it;
			}
			return sret;
		}

		ConfigNode  &ConfigNode::merge(const ConfigNode &with) {
			if (!with.mName.empty()) {
				mName = with.mName;
			}
			if (!with.mValue.empty()) {
				mValue = with.mValue;
			}
			for (auto const& child: with.mChildren) {
				setChild(child.getName(), child);
			}
			for (auto const& prop: with.mProps) {
				setProperty(prop.first, prop.second);
			}
			return *this;
		}

		ConfigNode::child_iter_type ConfigNode::findDirectChild(const String &name) {
			return mChildren.find([&](const ConfigNode &ch) { return ch.getName() == name; });
		}

		ConfigNode::child_citer_type ConfigNode::findDirectChild(const String &name) const {
			return mChildren.find([&](const ConfigNode &ch) { return ch.getName() == name; });
		}

		ConfigNode::child_riter_type ConfigNode::rfindDirectChild(const String &name) {
			return mChildren.rfind([&](const ConfigNode &ch) { return ch.getName() == name; });
		}

		ConfigNode::child_criter_type ConfigNode::rfindDirectChild(const String &name) const {
			return mChildren.rfind([&](const ConfigNode &ch) { return ch.getName() == name; });
		}

		ConfigNode *ConfigNode::setDirectChild(const String &name, const ConfigNode &n) noexcept(false) {
			auto found = mChildren.find([&](const ConfigNode &ch) { return ch.getName() == name; });
			if (found == mChildren.end()) {
				ConfigNode newNode(n);
				mChildren.add(n);
				found = mChildren.find([&](const ConfigNode &ch) { return ch.getName() == name; });
			}
			*found = n;
			found->setName(name);
			acquireChild(*found, this);
			return &*found;
		}

		const ConfigNode *ConfigNode::getDirectChild(const String &name, bool except) const noexcept(false) {
			auto found = mChildren.find([&](const ConfigNode &ch) { return ch.getName() == name; });
			if (found == mChildren.end()) {
				if (except) {
					throw std::runtime_error("missing child '" + name + "' in ConfigNode '" + getPath() + "'");
				}
				return nullptr;
			}
			return &*found;
		}

		ConfigNode *ConfigNode::getDirectChild(const String &name, bool except) noexcept(false) {
			auto found = mChildren.find([&](const ConfigNode &ch) { return ch.getName() == name; });
			if (found == mChildren.end()) {
				if (except) {
					throw std::runtime_error("missing child '" + name + "' in ConfigNode '" + getPath() + "'");
				}
				return nullptr;
			}
			return &*found;
		}

		bool ConfigNode::hasDirectChild(const String &name) const noexcept {
			return getDirectChild(name, false) != nullptr;
		}

		bool ConfigNode::removeDirectChild(const String &name, ConfigNode *out) {
			auto found = mChildren.iter([&](const ConfigNode &c) { return c.getName() == name; });
			if (found == mChildren.end()) {
				return false;
			}
			if (out) {
				*out = *found;
			}
			mChildren->erase(found);
			return true;
		}

		bool ConfigNode::removeDirectProperty(const String &name, String *out) {
			auto found = mProps->find(name);
			if (found == mProps.end()) {
				return false;
			}
			if (out) {
				*out = found->second;
			}
			mProps->erase(found);
			return true;
		}

		ConfigNode *ConfigNode::addDirectChild(const ConfigNode &n) {
			if (hasDirectChild(n.getName())) {
				throw std::runtime_error("'" + n.getName() + "' already registered in ConfigNode '" + getPath() + "'");
			}
			mChildren.add(n);
			acquireChild(mChildren->back(), this);
			return &mChildren->back();
		}

		bool ConfigNode::hasProperties() const {
			return !mProps.empty();
		}

		bool ConfigNode::removeProperty(const String &name, String *out) {
			DotAccess access(this, name, DAF_NEED_PROPERTY);
			access.walk();
			if (access.property) {
				if (out) {
					*out = *access.property;
				}
				access.child->removeDirectProperty(access.propertyName);
				return true;
			}
			return false;
		}

		ConfigNode::prop_iter_type ConfigNode::findProperty(const String &name) {
			return mProps->find(name);
		}
		ConfigNode::prop_citer_type ConfigNode::findProperty(const String &name) const {
			return mProps->find(name);
		}

		bool ConfigNode::empty() const noexcept {
			return mProps.empty() && mChildren.empty() && mValue.empty();
		}

		template<typename Out, typename Item = typename Out::value_type, typename ItemRes = typename Out::value_type>
		static Out *aggregate_node_data(const ConfigNode *node,
										std::function<bool(const ConfigNode *node, const std::pair<const Item, Item> *prop, ItemRes *result)> get_value,
										bool recurs,
										Out *ret) {
			ItemRes item;
			for (auto const& prop: node->getProperties()) {
				if (get_value(node, &prop, &item)) {
					ret->add(item);
				}
			}
			for (auto const& child: node->getChildren()) {
				if (get_value(&child, nullptr, &item)) {
					ret->add(item);
				}
				if (recurs) {
					ret = aggregate_node_data<Out, Item, ItemRes>(&child, get_value, recurs, ret);
				}
			}
			return ret;
		}

		Vector<String> ConfigNode::getDirectChildrenNames() const noexcept {
			Vector<String>  ret;
			auto get_name = [&](const ConfigNode *child, const std::pair<const String, String> *prop, String *res) -> bool {
				if (child != this && prop == nullptr) {
					*res = child->getName();
					return true;
				}
				return false;
			};
			return *aggregate_node_data<Vector<String>, String, String>(this, get_name, false, &ret);
		}

		Vector<String> ConfigNode::getDirectPropertyNames() const noexcept {
			Vector<String>  ret;
			auto get_name = [&](const ConfigNode *child, const std::pair<const String, String> *prop, String *res) -> bool {
				if (child == this && prop != nullptr) {
					*res = prop->first;
					return true;
				}
				return false;
			};
			return *aggregate_node_data<Vector<String>, String, String>(this, get_name, false, &ret);
		}

		Vector<String> ConfigNode::getChildrenNames() const noexcept {
			Vector<String>  ret;
			auto get_name = [&](const ConfigNode *child, const std::pair<const String, String> *prop, String *res) -> bool {
				if (child != this && prop == nullptr) {
					*res = child->getPath();
					return true;
				}
				return false;
			};
			aggregate_node_data<Vector<String>, String, String>(this, get_name, true, &ret);
			return ret;
		}

		Vector<String> ConfigNode::getPropertyNames() const noexcept {
			Vector<String>  ret;
			auto get_name = [&](const ConfigNode *child, const std::pair<const String, String> *prop, String *res) -> bool {
				if (prop != nullptr) {
					*res = child->getPath().empty() ? String() : (child->getPath() + ".")
							+ prop->first;
					return true;
				}
				return false;
			};
			aggregate_node_data<Vector<String>, String, String>(this, get_name, true, &ret);
			return ret;
		}

		Map<String, String> ConfigNode::getPropertyNameValuePairs() const noexcept {
			Map<String, String>  ret;
			auto get_name = [&](const ConfigNode *child, const std::pair<const String, String> *prop, std::pair<String, String> *res) -> bool {
				if (prop != nullptr) {
					*res = std::make_pair(
						child->getPath().empty() ? String() : (child->getPath() + ".") + prop->first,
						prop->second
					);
					return true;
				}
				return false;
			};
			aggregate_node_data<Map<String, String>, String, std::pair<String, String>>(this, get_name, true, &ret);
			return ret;
		}

		Map<String, String> ConfigNode::getDirectPropertyNameValuePairs() const noexcept {
			Map<String, String>  ret;
			auto get_name = [&](const ConfigNode *child, const std::pair<const String, String> *prop, std::pair<String, String> *res) -> bool {
				if (prop != nullptr) {
					*res = std::make_pair(
						prop->first,
						prop->second
					);
					return true;
				}
				return false;
			};
			aggregate_node_data<Map<String, String>, String, std::pair<String, String>>(this, get_name, false, &ret);
			return ret;
		}

		const ConfigNode *ConfigNode::getSchema() const noexcept {
			return mSchema;
		}

		void ConfigNode::setSchema(const ConfigNode *v) noexcept {
			mSchema = v;
		}

		void ConfigNode::validate(unsigned int flags) noexcept(false) {
			if (mSchema) {
				ConfigValidator validator(*mSchema);
				validator.validate(this, flags);
			}
		}

		Vector<ConfigNode *> ConfigNode::getAncestors() {
			Vector<ConfigNode*>     invret;
			ConfigNode *cur = this;
			while (cur) {
				invret.add(cur);
				cur = cur->getParent();
			}
			Vector<ConfigNode*>     ret;
			for (auto it = invret.rbegin(); it != invret.rend(); it++) {
				ret.add(*it);
			}
			return ret;
		}

		Vector<const ConfigNode *> ConfigNode::getAncestors() const {
			Vector<const ConfigNode*>     invret;
			const ConfigNode *cur = this;
			while (cur) {
				invret.add(cur);
				cur = cur->getParent();
			}
			Vector<const ConfigNode*>     ret;
			for (auto it = invret.rbegin(); it != invret.rend(); it++) {
				ret.add(*it);
			}
			return ret;
		}


		ConfigFile::ConfigFile()
				: Resource(), ConfigNode() {}

		ConfigFile::ConfigFile(ResourceFactory *factory, const String &name, const String &path,
		                       const ResourceType &type, const PropertyMap &props, const SharedIOStream &stream)
				: Resource(factory, name, path, type == ResourceType::Unknown ? ResourceType::Config : type, props, stream)
				, ConfigNode()
		{}

		void ConfigFile::setName(const String &name) noexcept {
			Resource::setName(name);
		}

		const String &ConfigFile::getName() const noexcept {
			return Resource::getName();
		}

	}
}