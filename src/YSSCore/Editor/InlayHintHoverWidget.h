#ifndef YSSCore_Editor_InlayHintHoverWidget_h
#define YSSCore_Editor_InlayHintHoverWidget_h
#include "YSSCoreCompileMacro.h"
#include <QtWidgets/qframe.h>
#include <Utility/JsonConfig.h>
#include <General/YSSURI.h>
namespace YSSCore::Editor {
	class InlayHintHoverWidgetPrivate;
	class YSSCoreAPI InlayHintHoverWidget :public QFrame {
		Q_OBJECT;
		friend class InlayHintHoverWidgetPrivate;
	public:
		InlayHintHoverWidget(QWidget* parent = nullptr);
		virtual ~InlayHintHoverWidget();
	public:
		void setTargetFile(const YSSCore::General::YSSURI& targetFile);
		YSSCore::General::YSSURI getTargetFile() const;
		void setBlockNumber(qint32 blockNumber);
		qint32 getBlockNumber() const;
		void setColumn(qint32 column);
		qint32 getColumn() const;
		void setRawText(const QString& rawText);
		QString getRawText() const;
		void setPara(const Visindigo::Utility::JsonConfig& para);
		Visindigo::Utility::JsonConfig getPara() const;
	public:
		virtual void onHoverRequest() = 0;
	signals:
		void requestModify(qint32 startColumn, qint32 rawLength, QString& newContent);
	private:
		InlayHintHoverWidgetPrivate* d;
	};
}
#endif // YSSCore_Editor_InlayHintHoverWidget_h
