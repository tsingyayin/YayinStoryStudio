#include "General/Placeholder.h"
#include <QtCore/qstring.h>
#include <QtCore/qmap.h>
#include <QtCore/qregularexpression.h>
#include "General/Log.h"
#include "General/private//VIGeneral_p.h"

namespace Visindigo::General {
	class PlaceholderProviderPrivate {
		friend class PlaceholderProvider;
	protected:
		QString name;
	};

	/*!
		\class Visindigo::General::PlaceholderProvider
		\brief PlaceholderProvider是占位符提供者的基类，插件可以通过继承这个类来创建自己的占位符提供者。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		占位符提供者是一个插件模块，插件可以通过继承PlaceholderProvider来创建自己的占位符提供者。
		每个占位符提供者都有一个唯一的ID，用于在占位符中引用。占位符的格式为$(ID::Name::Param)，
		其中ID是占位符提供者的ID，Name是占位符的名称，Param是可选的参数。
	*/

	/*!
		\since Visindigo 0.13.0
		\a plugin 插件实例指针，必须传入所属插件的实例。
		\a moduleID 占位符提供者的模块ID，在本插件内唯一。当providerName不设置时，moduleID也将作为占位符提供者的ID使用。
		\a providerName 占位符提供者的名称，用于在占位符中引用。如果不设置，则使用moduleID作为占位符提供者的ID。
		构造函数。
	*/
	PlaceholderProvider::PlaceholderProvider(Plugin* plugin, const QString& moduleID, const QString& providerName) : 
		PluginModule(plugin, moduleID, VIModuleType_PlaceholderProvider, providerName), d(new PlaceholderProviderPrivate) {
		if (providerName == QString()) {
			d->name = moduleID;
		}
		else {
			d->name = providerName;
		}
	}

	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	PlaceholderProvider::~PlaceholderProvider() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		获取占位符提供者的ID
	*/
	QString PlaceholderProvider::getPlaceholderName() const {
		return d->name;
	}

	/*!
		\fn virtual QString PlaceholderProvider::onPlaceholderRequest(const QString& name, const QString& param) = 0;
		\since Visindigo 0.13.0
		当占位符被请求解析时，这个函数会被调用。name是占位符的名称，param是占位符的参数。插件需要重写这个函数来返回占位符的值。

		在编写这个函数时，不需要考虑嵌套和递归，它对这个函数屏蔽。只需要根据name和param返回正确的值即可。占位符解析系统会负责处理嵌套和递归的部分。
	*/

	class PlaceholderManagerPrivate {
		friend class PlaceholderManager;
	protected:
		QMap<QString, PlaceholderProvider*> providers;
		QMap<QString, QObject*> qobjectProviders;
		static PlaceholderManager* Instance;
	};
	PlaceholderManager* PlaceholderManagerPrivate::Instance = nullptr;

	/*!
		\class Visindigo::General::PlaceholderManager
		\brief PlaceManager提供一个全局的占位符解析系统，允许插件注册占位符提供者来动态生成文本内容。
		\since Visindigo 0.13.0
		\inmodule Visindigo

		首先要向Minecraft领域知名插件PlaceholderAPI致敬。这个类虽然在实现和原理上
		与PlaceholderAPI没有半毛钱关系，但在设计理念与生态定位上几乎完全与其相同，也是此类
		的主要灵感来源。

		PlaceholderManager是一个单例类，提供了一个全局的占位符解析系统。
		插件可以通过注册占位符提供者（PlaceholderProvider）来动态生成文本内容。
		占位符的格式为$(ID::Name::Param)，其中ID是占位符提供者的ID，Name是占位符的名称，Param是可选的参数。

		其中，param部分支持递归占位符，即参数中也可以包含占位符，这些占位符会在解析时被递归解析。

		要解析带有占位符的文本，直接使用VIPlacholder(str)宏即可，不需要单独识别字符串中带有占位符的部分。

		此外，PlaceholderManager还支持注册QObject作为占位符提供者，通过$(#QObject::ObjectName::Property)的格式来访问QObject的属性。

	*/

	/*!
		\relates Visindigo::General::PlaceholderManager
		\macro VIPM
		获取PlaceholderManager的单例实例。这个宏是PlaceholderManager的一个便捷接口，直接调用PlaceholderManager::getInstance()来获取单例实例。
	*/

	/*!
		\relates Visindigo::General::PlaceholderManager
		\macro VIPlaceholder(str)
		解析带有占位符的字符串。这个宏是PlaceholderManager的一个便捷接口，直接调用PlaceholderManager::getInstance()->requestPlaceholder(str)来解析字符串中的占位符。
	*/

	/*!
		\since Visindigo 0.13.0
		构造函数
	*/
	PlaceholderManager::PlaceholderManager() : d(new PlaceholderManagerPrivate) {

	}

	/*!
		\since Visindigo 0.13.0
		析构函数
	*/
	PlaceholderManager::~PlaceholderManager() {
		delete d;
	}

	/*!
		\since Visindigo 0.13.0
		获取PlaceholderManager的单例实例
	*/
	PlaceholderManager* PlaceholderManager::getInstance() {
		if (PlaceholderManagerPrivate::Instance == nullptr) {
			PlaceholderManagerPrivate::Instance = new PlaceholderManager();
		}
		return PlaceholderManagerPrivate::Instance;
	}

	/*!
		\since Visindigo 0.13.0
		注册一个占位符提供者
	*/
	void PlaceholderManager::registerProvider(PlaceholderProvider* provider) {
		if (!provider) {
			return;
		}
		if(d->providers.contains(provider->getPlaceholderName())) {
			vgWarningF << "Provider with ID " << provider->getPlaceholderName() << " already exists, skipping registration.";
			return;
		}
		d->providers.insert(provider->getPlaceholderName(), provider);
		vgSuccessF << "Registered provider with ID " << provider->getPlaceholderName();
	}

	/*!
		\since Visindigo 0.13.0
		注册一个QObject作为占位符提供者
	*/
	void PlaceholderManager::registerQObject(QObject* provider) {
		if (!provider) {
			return;
		}
		if ( d->qobjectProviders.contains(provider->objectName()) ) {
			vgWarningF << "QObject provider with object name " << provider->objectName() << " already exists, skipping registration.";
			return;
		}
		d->qobjectProviders.insert(provider->objectName(), provider);
		vgSuccessF << "Registered QObject provider with object name " << provider->objectName();
	}

	/*!
		\since Visindigo 0.13.0
		注销一个占位符提供者
	*/
	void PlaceholderManager::unregisterProvider(const QString& providerID) {
		d->providers.remove(providerID);
		vgSuccessF << "Unregistered provider with ID " << providerID;
	}

	/*!
		\since Visindigo 0.13.0
		注销一个QObject占位符提供者
	*/
	void PlaceholderManager::unregisterQObject(const QString& providerObjectName) {
		d->qobjectProviders.remove(providerObjectName);
		vgSuccessF << "Unregistered QObject provider with object name " << providerObjectName;
	}

	/*!
		\since Visindigo 0.13.0
		请求解析一个包含占位符的文本，返回解析后的文本。

		占位符解析是懒替换的，如果没有可以解析这个占位符的Provider，则占位符将被原样保留在文本中。
	*/
	QString PlaceholderManager::requestPlaceholder(QString rawText) {
		// Placeholder format: $(ID::Name::Param), Param is optional.
		// maybe more than one match
		QRegularExpression re(R"(\$\((\w+)::(\w+)(::(.*))*\))");
		QRegularExpressionMatchIterator it = re.globalMatch(rawText);
		if (!it.isValid()) {
			return rawText;
		}
		while (it.hasNext()) {
			QRegularExpressionMatch match = it.next();
			QString providerID = match.captured(1);
			QString name = match.captured(2);
			QString param = match.captured(4);
			if (providerID=="#QObject") {
				QObject* provider = d->qobjectProviders.value(name);
				if (!provider) {
					vgErrorF << "QObject provider with object name " << name << " unexpectedly null, will be removed from QObject providers list.";
					d->qobjectProviders.remove(name);
					continue;
				}
				QVariant result = provider->property(param.toStdString().c_str());
				rawText.replace(match.capturedStart(), match.capturedLength(), result.toString());
			}
			else if (d->providers.contains(providerID)) {
				if(param.contains("$(")) {
					//vgDebugF << "recursive detected: " << param;
					param = requestPlaceholder(param); // recursive 
					//vgDebugF << param;
				}
				PlaceholderProvider* provider = d->providers.value(providerID);
				if (!provider) {
					vgErrorF << "Provider with ID " << providerID << " unexpectedly null, will be removed from providers list.";
					d->providers.remove(providerID);
					continue;
				}
				QString result = provider->onPlaceholderRequest(name, param);
				rawText.replace(match.capturedStart(), match.capturedLength(), result);
			}
		}
		return rawText; 
	}

	/*!
		\since Visindigo 0.13.0
		解析一个包含占位符的文本，返回解析后的文本。它只是requestPlaceholder的别名。
	*/
	QString PlaceholderManager::parse(QString rawText) {
		return requestPlaceholder(rawText);
	}
}