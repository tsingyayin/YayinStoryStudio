#pragma once
#include "YSSCoreCompileMacro.h"
#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>
#include <Utility/JsonConfig.h>
#include <VIMacro.h>
namespace YSSCore::Editor {
	class VirtualFilePathPrivate;
	class YSSCoreAPI VirtualFilePath {
	public:
		static bool isVirtualFilePath(const QString& path);
	public:
		VirtualFilePath(const QString& path);
		VirtualFilePath(const QString& ext, const QString& fileName, const QString& param);
		VirtualFilePath(const QString& ext, const QString& fileName, const Visindigo::Utility::JsonConfig& params);
		VIMoveable(VirtualFilePath);
		VICopyable(VirtualFilePath);
		~VirtualFilePath();
	public:
		bool isValid() const;
		QString toString() const;
		void setExt(const QString& ext);
		QString getExt() const;
		void setFileName(const QString& fileName);
		QString getFileName() const;
		void setParam(const QString& param);
		QString getParam() const;
		void setParam(const QString& key, const QVariant& value);
		QVariant getParam(const QString& key, bool* ok = nullptr) const;
		void setParams(const Visindigo::Utility::JsonConfig& params);
		Visindigo::Utility::JsonConfig getParams() const;
	private:
		VirtualFilePathPrivate* d;
	};
}