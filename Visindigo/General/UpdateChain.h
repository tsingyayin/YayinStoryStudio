#pragma once
#include "../Macro.h"
#include <QtCore/qobject.h>
#include <QtCore/qmap.h>
#include <QtCore/qlist.h>
#include <functional>
namespace Visindigo::General {
	class UpdateChain;
	class UpdateChainPrivate;
	class UpdateChainNodePrivate;

	class VisindigoAPI UpdateChainNode
	{
		friend class UpdateChain;
		friend class UpdateChainPrivate;
		friend class UpdateChainNodePrivate;
	public:
		UpdateChainNode(UpdateChain* parent = nullptr);
		UpdateChainNode(const UpdateChainNode& other);
		UpdateChainNode(UpdateChainNode&& other) noexcept;
		UpdateChainNode& operator=(const UpdateChainNode& other);
		UpdateChainNode& operator=(UpdateChainNode&& other) noexcept;
		UpdateChainNode& operator<<(const QString& name);
		UpdateChainNode& operator<<(std::function<void()> func);
	protected:
		UpdateChainNodePrivate* d;
	};

	class VisindigoAPI UpdateChain :public QObject {
		Q_OBJECT;

	public:
		UpdateChain(QObject* parent = nullptr);
		void doUpdate();
		virtual QString getCurrentVersion() = 0;
		UpdateChainNode& createNode();
		void updateNode(UpdateChainNode* node);
	private:
		UpdateChainPrivate* d;
	};
}
#define UNode(VERSION_FROM, VERSION_TO) createNode()<<#VERSION_FROM<<[this]()