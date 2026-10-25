#ifndef Plugin_ASERStudio_YSS_InlayColorProvider_h
#define Plugin_ASERStudio_YSS_InlayColorProvider_h
#include <QtCore/qhash.h>
#include <QtGui/qcolor.h>
#include <Editor/InlayHintProvider.h>
namespace ASERStudio::YSS {
	inline constexpr const char* InlayColorProviderID = "cn.yxgeneral.aserstudio.inlaycolor";
	namespace InlayColorKeys {
		inline constexpr const char* Kind = "kind";
		inline constexpr const char* KindHex = "hex";
		inline constexpr const char* KindVector = "vector";
		inline constexpr const char* Start = "start";
		inline constexpr const char* Length = "length";
		inline constexpr const char* Value = "value";
		inline constexpr const char* Scale = "scale";
		inline constexpr const char* Alpha = "alpha";
		inline constexpr const char* Components = "components";
	};

	class InlayColorProvider :public YSSCore::Editor::InlayHintProvider {
		Q_OBJECT;
	public:
		InlayColorProvider(const QString& name, const QString& id, YSSCore::Editor::EditorPlugin* plugin);
		virtual ~InlayColorProvider();
	public:
		virtual QImage& drawInlayHint(qint32 w, qint32 h, const YSSCore::General::YSSURI& file,
			const QString& name, Visindigo::Utility::JsonConfig& para) override;
		virtual YSSCore::Editor::InlayHintHoverWidget* createHoverWidget(
			const YSSCore::General::YSSURI& targetFile) override;
	public:
		static QColor colorFromString(const QString& value, double* scale = nullptr);
		static QString colorToHex(const QColor& color, bool withAlpha);
		static QString colorToVector(const QColor& color, double scale, int components);
	private:
		QHash<QString, QImage> ImageCache;
	};
}
#endif // Plugin_ASERStudio_YSS_InlayColorProvider_h
