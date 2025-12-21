#include "../UpdateChain.h"
#include <QtCore/qdebug.h>
namespace Visindigo::General {
	class UpdateChainNodePrivate {
		friend class UpdateChainNode;
		friend class UpdateChain;
	protected:
		UpdateChain* Parent = nullptr;
		QString Name;
		std::function<void()> Function;
	};
	class UpdateChainPrivate {
		friend class UpdateChain;
	protected:
		QList<UpdateChainNode> ChainNode;
		QMap<QString, UpdateChainNode> ChainNodeMap;
	};
	
	/*!
		\class Visindigo::General::UpdateChainNode
		\inmodule Visindigo
		\brief 此类为Visindigo提供更新链节点表示。

		\warning 此类还在开发中，不应该使用它。
	*/

	UpdateChainNode::UpdateChainNode(UpdateChain* parent) {
		d = new UpdateChainNodePrivate;
		d->Parent = parent;
	}
	UpdateChainNode::UpdateChainNode(const UpdateChainNode& other) {
		d = new UpdateChainNodePrivate;
		d->Parent = other.d->Parent;
		d->Name = other.d->Name;
		d->Function = other.d->Function;
	}
	UpdateChainNode::UpdateChainNode(UpdateChainNode&& other) noexcept {
		d = other.d;
		other.d = nullptr;
	}
	UpdateChainNode& UpdateChainNode::operator=(const UpdateChainNode& other) {
		if (this != &other) {
			delete d;
			d = new UpdateChainNodePrivate;
			d->Parent = other.d->Parent;
			d->Name = other.d->Name;
			d->Function = other.d->Function;
		}
		return *this;
	}
	UpdateChainNode& UpdateChainNode::operator=(UpdateChainNode&& other) noexcept {
		if (this != &other) {
			delete d;
			d = other.d;
			other.d = nullptr;
		}
		return *this;
	}
	UpdateChainNode& UpdateChainNode::operator<<(const QString& name) {
		d->Name = name;
		if (d->Parent != nullptr) {
			d->Parent->updateNode(this);
		}
		return *this;
	}
	UpdateChainNode& UpdateChainNode::operator<<(std::function<void()> func) {
		d->Function = func;
		if (d->Parent != nullptr) {
			d->Parent->updateNode(this);
		}
		return *this;
	}

	UpdateChain::UpdateChain(QObject* parent) : QObject(parent)
	{
		d = new UpdateChainPrivate;
	}
	void UpdateChain::doUpdate() {
		for (QString version : d->ChainNodeMap.keys()) {
			UpdateChainNode node = d->ChainNodeMap[version];
			qDebug() << "Node Version:" << version;
			node.d->Function();
		}
	}
	UpdateChainNode& UpdateChain::createNode() {
		UpdateChainNode node(this);
		d->ChainNode.append(node);
		return d->ChainNode.last();
	}
	void UpdateChain::updateNode(UpdateChainNode* node) {
		if (node == nullptr) {
			return;
		}
		QString version = node->d->Name;
		if (d->ChainNodeMap.contains(version)) {
			d->ChainNodeMap[version] = *node;
		}
		else {
			d->ChainNodeMap.insert(version, *node);
		}
	}
};