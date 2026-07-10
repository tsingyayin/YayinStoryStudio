#include "Editor/VirtualFilePath.h"
#include <QtCore/qregularexpression.h>
#include <QtCore/qjsondocument.h>

namespace YSSCore::Editor {
	class VirtualFilePathPrivate {
		friend class VirtualFilePath;
	protected:
		QString ext;
		QString fileName;
		Visindigo::Utility::JsonConfig param;
	};

	VIMoveable_Impl(VirtualFilePath);
	VICopyable_Impl(VirtualFilePath);

	/*!
		\class YSSCore::Editor::VirtualFilePath
		\inmodule YSSCore
		\brief 表示一个虚拟文件路径，提供解析和构造虚拟文件路径的能力.
		\since YSS 0.16.0
		\ingroup FileService

		虚拟文件路径是一种特殊的路径格式，用于在文件编辑区域打开一些并非真实存在的文件进行编辑，
		譬如程序设置、插件设置等页面。

		\section1 路径格式

		虚拟文件路径的格式固定为：
		\badcode
			@file_ext!file_name?param
		\endcode

		其中：
		\list
		\li \c file_ext 为虚拟文件扩展名，也作为虚拟文件服务器的ID使用。同一扩展名只能有一个FileServer注册。
		\li \c file_name 为显示用的文件名，推荐用于本地化展示。
		\li \c param 为可选参数，内部以 Visindigo::Utility::JsonConfig 对象存储，推荐按JSON格式书写。
		\endlist

		例如：
		\badcode
			@YSS.SettingsWidget!程序设置?{"from":"menubar"}
		\endcode

		\section1 使用方式

		可以使用静态函数 isVirtualFilePath() 来判断任意字符串是否符合虚拟文件路径的格式。

		可以通过构造函数从完整的虚拟文件路径字符串解析，也可以通过 ext、fileName、param 三个部分
		分别构造。toString() 方法可以将对象转回完整的虚拟文件路径字符串。

		参数部分支持两种存取方式：
		\list
		\li 原始字符串存取：通过 setParam() / getParam() 以JSON字符串形式读写 \c param 部分。
		\li JSON对象存取：通过 setParam(key, value) / getParam(key) / setParams() / getParams()
			以JsonConfig对象形式读写参数的键值对。
		\endlist

		\section1 与FileServer的关系

		虚拟文件路径的 \c file_ext 在 FileServer 中作为虚拟文件服务器的ID使用。
		有关虚拟文件服务器的注册和分发机制，请参考 YSSCore::Editor::FileServer 和
		YSSCore::Editor::FileServerManager 的相关文档。
	*/

	/*!
		\fn YSSCore::Editor::VirtualFilePath::isVirtualFilePath(const QString& path)
		\since YSS 0.16.0
		\a path 待检测的路径字符串。
		return 如果 \a path 符合虚拟文件路径格式 \c{@ext!fileName?param}，返回true；否则返回false。

		此函数使用正则表达式 \c{^@([^!]+)!([^?]+)\\?(.*)$} 进行匹配。
	*/
	bool VirtualFilePath::isVirtualFilePath(const QString& path) {
		static auto re = QRegularExpression(R"(^@([^!]+)!([^?]+)\?(.*)$)");
		return re.match(path).hasMatch();
	}

	/*!
		\since YSS 0.16.0
		\a path 完整的虚拟文件路径字符串。

		从完整的虚拟文件路径字符串构造。使用正则表达式 \c{^@([^!]+)!([^?]+)\\?(.*)$}
		解析路径，分别提取 ext、fileName 和 param 部分。param 部分会以JSON格式解析。

		如果 \a path 不符合虚拟文件路径格式，则 ext 和 fileName 均为空字符串，
		此时 isValid() 返回false。
	*/
	VirtualFilePath::VirtualFilePath(const QString& path) {
		d = new VirtualFilePathPrivate();
		static auto re = QRegularExpression(R"(^@([^!]+)!([^?]+)\?(.*)$)");
		auto match = re.match(path);
		if (match.hasMatch()) {
			d->ext = match.captured(1);
			d->fileName = match.captured(2);
			d->param = Visindigo::Utility::JsonConfig(match.captured(3));
		}
	}

	/*!
		\since YSS 0.16.0
		\a ext 虚拟文件扩展名（同时也是虚拟文件服务器ID）。
		\a fileName 显示用的文件名。
		\a param JSON格式的参数字符串。

		从各个部分构造虚拟文件路径。param 字符串会以JSON格式解析到内部JsonConfig对象中。
		如果 \a ext 或 \a fileName 为空，构造出的对象 isValid() 将返回false。
	*/
	VirtualFilePath::VirtualFilePath(const QString& ext, const QString& fileName, const QString& param) {
		d = new VirtualFilePathPrivate();
		d->ext = ext;
		d->fileName = fileName;
		d->param = Visindigo::Utility::JsonConfig(param);
	}

	/*!
		\since YSS 0.16.0
		\a ext 虚拟文件扩展名（同时也是虚拟文件服务器ID）。
		\a fileName 显示用的文件名。
		\a params JSON格式的参数对象。

		从各个部分构造虚拟文件路径。参数以JsonConfig对象形式直接存储。
	*/
	VirtualFilePath::VirtualFilePath(const QString& ext, const QString& fileName, const Visindigo::Utility::JsonConfig& params) {
		d = new VirtualFilePathPrivate();
		d->ext = ext;
		d->fileName = fileName;
		d->param = params;
	}

	/*!
		\since YSS 0.16.0
		析构VirtualFilePath对象。
	*/
	VirtualFilePath::~VirtualFilePath() {
		delete d;
	}

	/*!
		\since YSS 0.16.0
		return 当前对象是否表示一个有效的虚拟文件路径。

		只有当 ext 和 fileName 均非空时，才返回true。
	*/
	bool VirtualFilePath::isValid() const {
		return !d->ext.isEmpty() && !d->fileName.isEmpty();
	}

	/*!
		\since YSS 0.16.0
		return 完整的虚拟文件路径字符串，格式为 \c{@ext!fileName?param}。
		其中param部分以紧凑JSON格式序列化。
	*/
	QString VirtualFilePath::toString() const {
		return QStringLiteral("@%1!%2?%3").arg(d->ext, d->fileName, d->param.toString(QJsonDocument::Compact));
	}

	/*!
		\since YSS 0.16.0
		\a ext 新的虚拟文件扩展名。
	*/
	void VirtualFilePath::setExt(const QString& ext) {
		d->ext = ext;
	}

	/*!
		\since YSS 0.16.0
		return 当前虚拟文件扩展名。
	*/
	QString VirtualFilePath::getExt() const {
		return d->ext;
	}

	/*!
		\since YSS 0.16.0
		\a fileName 新的显示文件名。
	*/
	void VirtualFilePath::setFileName(const QString& fileName) {
		d->fileName = fileName;
	}

	/*!
		\since YSS 0.16.0
		return 当前显示文件名。
	*/
	QString VirtualFilePath::getFileName() const {
		return d->fileName;
	}

	/*!
		\since YSS 0.16.0
		\a param JSON格式的参数字符串。

		将参数字符串以JSON格式解析并替换当前参数。
	*/
	void VirtualFilePath::setParam(const QString& param) {
		d->param = Visindigo::Utility::JsonConfig(param);
	}

	/*!
		\since YSS 0.16.0
		return 以紧凑JSON字符串表示的当前参数。
	*/
	QString VirtualFilePath::getParam() const {
		return d->param.toString(QJsonDocument::Compact);
	}

	/*!
		\since YSS 0.16.0
		\a key 参数的键名。
		\a value 参数的键值。

		以JSON方式设置单个参数键值对。会修改现有参数（如果存在）或添加新参数。

		直接操作内部的JsonConfig对象。
	*/
	void VirtualFilePath::setParam(const QString& key, const QVariant& value) {
		d->param.setValue(key, QJsonValue::fromVariant(value));
	}

	/*!
		\since YSS 0.16.0
		\a key 参数的键名。
		\a ok 可选的输出参数，指示是否成功获取。
		return 参数键对应的QVariant值。

		以JSON方式获取单个参数键值。如果参数不存在，则 \a ok（如果提供）会被设为false，
		并返回空的QVariant。
	*/
	QVariant VirtualFilePath::getParam(const QString& key, bool* ok) const {
		auto val = d->param.getValue(key, ok);
		if (ok && !(*ok)) {
			return QVariant();
		}
		return val.toVariant();
	}

	/*!
		\since YSS 0.16.0
		\a params 要设置的完整参数JSON对象。

		以JsonConfig对象设置全部参数，会覆盖现有参数。
	*/
	void VirtualFilePath::setParams(const Visindigo::Utility::JsonConfig& params) {
		d->param = params;
	}

	/*!
		\since YSS 0.16.0
		return 以JsonConfig对象表示的当前全部参数。
	*/
	Visindigo::Utility::JsonConfig VirtualFilePath::getParams() const {
		return d->param;
	}
}