#include "Editor/ColorThemeProvider.h"
#include "Editor/LangServer.h"
#include <QtCore/qmap.h>
#include <QtCore/qstringlist.h>

namespace YSSCore::Editor {
	class ColorThemeProviderPrivate {
		friend class ColorThemeProvider;
	protected:
		QMap<QString, QMap<QString, StyleData>> themes; // 主题名 -> (配置节点键 -> 样式数据)
		QStringList staticThemes; // 静态主题列表，静态主题不允许被删除或修改
		QString templateTextPath; // 用作样式样本的文件路径
		QString currentThemeName; // 当前主题名
		QMap<QString, StyleData> currentTheme; // 当前主题的样式数据缓存，用于加速getCurrentThemeStyleData
		QString parseThemeJson(const QString& jsonStr); // 从JSON字符串解析一个主题，返回主题名
	};

	QString ColorThemeProviderPrivate::parseThemeJson(const QString& jsonStr) {
		Visindigo::Utility::JsonConfig config;
		QJsonParseError error = config.parse(jsonStr);
		if (error.error != QJsonParseError::NoError) {
			return QString();
		}
		QString themeName = config.getString("name");
		if (themeName.isEmpty()) {
			return QString();
		}
		QMap<QString, StyleData> styleData;
		Visindigo::Utility::JsonConfig datas = config.getObject("datas");
		QStringList keys = datas.keys();
		for (const QString& key : keys) {
			Visindigo::Utility::JsonConfig nodeConfig = datas.getObject(key);
			styleData.insert(key, Visindigo::Utility::JsonConfig::toMetable<StyleData>(nodeConfig));
		}
		themes.insert(themeName, styleData);
		if (themeName == currentThemeName) {
			currentTheme = styleData;
		}
		return themeName;
	}

	/*!
		\class YSSCore::Editor::ColorThemeProvider
		\brief 颜色主题提供者，负责维护一组颜色主题及其样式数据。
		\since YSS 0.16.0
		\inmodule YSSCore
		\ingroup LangService

		ColorThemeProvider是YSSCore::Editor::LangServer使用的颜色主题管理器，为语言服务器提供颜色主题服务。

		请注意，这个颜色主题与Visindigo中的颜色主题与模板不是相同的概念。此处颜色主题仅提供给
		编辑器使用，且一般来说，是给SyntaxHighlighter使用的。它与Visindigo中的颜色主题和模板没有直接关系。
		
		SyntaxHighlighter可以通过ColorThemeProvider获取当前主题的样式数据，并据此设置编辑器中不同语法元素的颜色和样式。

		每个颜色主题都通过一个主题名来唯一标识，主题名下保存了一组样式数据（YSSCore::Editor::StyleData），
		每个样式数据通过一个配置节点键（configNodeName）来索引。配置节点键是程序内部使用的索引，而
		样式数据中的styleName是用于显示的可读字符串，二者并不相同。

		主题可以通过JSON字符串解析和派生（参见parseStaticThemeFrom、parseUserThemeFrom和deriveUserThemeToJson）。
		主题分为静态主题和用户主题：静态主题通过parseStaticThemeFrom解析，不允许被删除或修改；
		用户主题可以通过createNewTheme创建、通过removeTheme删除，并可以通过setThemeStyleData修改。

		\section1 持久化数据的自动管理
		值得指出的是，这个类在实际使用中，有关持久化的需求大多已经由关联的其他类在后台自动完成，用户一般不需要再额外处理持久化数据。
		为了方便维护，我们披露关联逻辑的一些细节如下：
		\list
		\li 1. 主题的样式数据会在切换当前主题时自动缓存到currentTheme中，以加速getCurrentThemeStyleData的访问。
		用户一般不需要再缓存一次。
		\li 2. 用户数据的读取与保存在后台自动完成：当LangServer初始化时，会自动从 插件文件夹/_yss_auto_/LangServer/LangServerID 文件夹
		读取*.theme.json文件并解析为用户主题，当用户主题发生变更时，LangServer也会在内部通过监听信号themeModified自动保存用户主题到文件中。
		\li 3. 在LangServer被注册到EditorPlugin后，EditorPlugin会自动从插件配置的 _yss_auto_.LangServer.LangServerId.CurrentTheme 配置项中读取当前设置的主题名，
		并调用setCurrentTheme设置当前主题。且在currentTheme发生变化时，EditorPlugin也会自动将新的当前主题名写入该配置项。
		\endlist

		因此，一般来说，在使用这个类时，理应只需在自己所派生的LangServer类的构造函数直接用parseStaticThemeFrom加载静态主题即可，
		用户主题的读取和保存、当前主题的读取和保存都不需要额外处理。

		\section1 在SyntaxHighlighter中应用样式
		我们不推荐手动监听相关信号来做额外的持久化处理，因为这可能会导致重复保存或读取，造成不必要的性能开销。也不推荐通过
		这些信号来在SyntaxHighlighter中应用样式，SyntaxHighlighter中已经提供了YSSCore::Editor::SyntaxHighlighter::onThemeChanged
		虚函数来确保在所有需要SyntaxHighlighter更新样式的时机都能正确触发。

		这里需要披露的具体差异是，当用户在配置页面中手动修改主题时，相关页面会直接调用配置页面预览编辑器内部的SyntaxHighlighter::onThemeChanged函数来更新样式，
		而不实际改动任何现有主题数据，以避免不必要的持久化操作。只有当用户点击“保存”按钮时，才会真正修改主题数据并触发themeModified信号。

		因此，为了使配置页面预览编辑器能正确相应用户在编辑中的修改，就必须且只能依赖onThemeChanged函数。
	*/

	/*!
		\fn YSSCore::Editor::ColorThemeProvider::currentThemeChanged(const QString& themeName)
		\since YSS 0.16.0
		\a themeName 新的当前主题的主题名。如果当前主题被删除导致没有当前主题，则为空字符串。

		当前主题发生变化时发出此信号。当前主题发生变化的情况包括：通过setCurrentTheme切换当前主题，
		或通过removeTheme删除当前主题。
	*/

	/*!
		\fn YSSCore::Editor::ColorThemeProvider::themeAdded(const QString& themeName)
		\since YSS 0.16.0

		\a themeName 新添加的主题的主题名。

		当一个新的主题被添加时发出此信号。新主题可以通过createNewTheme创建，或通过parseStaticThemeFrom/parseUserThemeFrom解析。
	*/

	/*!
		\fn YSSCore::Editor::ColorThemeProvider::themeRemoved(const QString& themeName)
		\since YSS 0.16.0
		\a themeName 被删除的主题的主题名。

		当一个主题被删除时发出此信号。主题可以通过removeTheme删除。
		这个信号在removeTheme函数中发出，且在发出此信号时，主题已经被删除。
	*/

	/*!
		\fn YSSCore::Editor::ColorThemeProvider::themeModified(const QString& themeName)
		\since YSS 0.16.0
		\a themeName 被修改的主题的主题名。

		当一个主题被修改时发出。通过setThemeStyleData修改主题的样式数据时会发出此信号，此外，setCurrentTheme也会在切换当前主题时发出此信号。
	*/

	/*!
		\since YSS 0.16.0
		\a parent 所属的语言服务器。

		构造一个颜色主题提供者。\a parent 将作为此对象的QObject父对象，同时用于关联所属的语言服务器。
	*/
	ColorThemeProvider::ColorThemeProvider(LangServer* parent) : QObject(parent) {
		d = new ColorThemeProviderPrivate();
	}

	/*!
		\since YSS 0.16.0

		析构颜色主题提供者。
	*/
	ColorThemeProvider::~ColorThemeProvider() {
		delete d;
	}

	/*!
		\fn void ColorThemeProvider::currentThemeChanged(const QString& themeName)
		\since YSS 0.16.0
		\a themeName 新的当前主题的主题名。如果当前主题被删除导致没有当前主题，则为空字符串。

		当当前主题发生变化时发出此信号。当前主题发生变化的情况包括：通过setCurrentTheme切换当前主题，
		或通过removeTheme删除当前主题。
	*/

	/*!
		\since YSS 0.16.0
		return 所有已加载主题的主题名列表。

		返回当前所有已加载主题的主题名列表，包括静态主题和用户主题。

		如果没有任何主题，则列表是空的。

		键的顺序与加载顺序无关，也不保证按照任何特定顺序排列。
	*/
	QStringList ColorThemeProvider::getSupportedThemes() {
		return d->themes.keys();
	}

	/*!
		\since YSS 0.16.0
		return 当前主题的主题名。如果没有设置当前主题，则返回空字符串。

		返回当前主题的主题名。
	*/
	QString ColorThemeProvider::getCurrentTheme() {
		return d->currentThemeName;
	}

	/*!
		\since YSS 0.16.0
		\a themeName 要创建的新主题的主题名。
		\a copyFromTheme 要复制样式的源主题的主题名。

		创建一个新的主题。如果 \a themeName 为空或已存在同名的主题，则不做任何操作。
		如果 \a copyFromTheme 指定了一个已存在的主题，则新主题会复制该主题的全部样式数据，
		否则新主题将不含任何样式数据。新创建的主题不会被视为静态主题。
	*/
	void ColorThemeProvider::createNewTheme(const QString& themeName, const QString& copyFromTheme) {
		if (themeName.isEmpty() || d->themes.contains(themeName)) {
			return;
		}
		QMap<QString, StyleData> styleData;
		if (d->themes.contains(copyFromTheme)) {
			styleData = d->themes.value(copyFromTheme);
		}
		d->themes.insert(themeName, styleData);
		emit themeAdded(themeName);
	}

	/*!
		\since YSS 0.16.0
		\a themeName 要删除的主题的主题名。

		删除指定的主题。如果该主题是静态主题（参见isStaticTheme），则不允许删除，此函数不做任何操作。
		如果被删除的主题是当前主题，则当前主题会被清空。
	*/
	void ColorThemeProvider::removeTheme(const QString& themeName) {
		if (d->staticThemes.contains(themeName)) {
			return;
		}
		d->themes.remove(themeName);
		if (d->currentThemeName == themeName) {
			d->currentThemeName.clear();
			d->currentTheme.clear();
			emit currentThemeChanged(QString());
		}
		emit themeRemoved(themeName);
	}

	/*!
		\since YSS 0.16.0
		\a themeName 要设为当前主题的主题名。

		将指定主题设为当前主题。如果 \a themeName 对应的主题不存在，则不做任何操作。
	*/
	void ColorThemeProvider::setCurrentTheme(const QString& themeName) {
		if (d->themes.contains(themeName) && d->currentThemeName != themeName) {
			d->currentThemeName = themeName;
			d->currentTheme = d->themes.value(themeName);
			emit currentThemeChanged(themeName);
			emit themeModified(themeName);
		}
	}

	/*!
		\since YSS 0.16.0
		\a jsonStr 静态主题的JSON字符串。

		从JSON字符串解析一个静态主题。静态主题不允许被删除或修改（removeTheme和setThemeStyleData
		对静态主题无效）。JSON的结构与parseUserThemeFrom相同，区别在于解析出的主题会被标记为静态主题。

		此函数直接接收JSON字符串而不接触文件系统。
	*/
	void ColorThemeProvider::parseStaticThemeFrom(const QString& jsonStr) {
		QString themeName = d->parseThemeJson(jsonStr);
		if (not themeName.isEmpty() && not d->staticThemes.contains(themeName)) {
			d->staticThemes.append(themeName);
			emit themeAdded(themeName);
		}
	}

	/*!
		\since YSS 0.16.0
		\a jsonStr 用户主题的JSON字符串。

		从JSON字符串解析一个用户主题。JSON的结构与parseStaticThemeFrom相同，区别在于此函数
		不会将该主题标记为静态主题。
	*/
	void ColorThemeProvider::parseUserThemeFrom(const QString& jsonStr) {
		d->parseThemeJson(jsonStr);
	}

	/*!
		\since YSS 0.16.0
		\a themeName 要派生为JSON的主题名。
		return 表示指定主题的JSON字符串。如果主题不存在，则返回空字符串。

		将指定主题派生为JSON字符串。JSON的结构与加载时的结构一致："name"为 \a themeName，
		"datas"下的每个键为配置节点键，对应的子对象通过Visindigo::Utility::JsonConfig::fromMetable
		从YSSCore::Editor::StyleData生成。此函数不接触文件系统，由调用方自行决定如何保存返回的字符串。
	*/
	QString ColorThemeProvider::deriveUserThemeToJson(const QString& themeName) {
		if (themeName.isEmpty() || not d->themes.contains(themeName)) {
			return QString();
		}
		Visindigo::Utility::JsonConfig config;
		config.setString("name", themeName);
		Visindigo::Utility::JsonConfig datas;
		const QMap<QString, StyleData>& styleData = d->themes.value(themeName);
		for (auto it = styleData.begin(); it != styleData.end(); ++it) {
			datas.setObject(it.key(), Visindigo::Utility::JsonConfig::fromMetable(it.value()));
		}
		config.setObject("datas", datas);
		return config.toString();
	}

	/*!
		\since YSS 0.16.0
		\a themeName 主题名。
		return 指定主题的全部样式数据。如果主题不存在，则返回空的QMap。

		返回指定主题下所有配置节点键对应的样式数据。
	*/
	QMap<QString, StyleData> ColorThemeProvider::getThemeStyleData(const QString& themeName) {
		return d->themes.value(themeName);
	}

	/*!
		\since YSS 0.16.0
		\a themeName 主题名。
		\a styleData 要设置的样式数据。

		设置指定主题的全部样式数据。如果该主题是静态主题（参见isStaticTheme），则不允许修改，
		此函数不做任何操作。如果主题不存在，则会创建该主题。
	*/
	void ColorThemeProvider::setThemeStyleData(const QString& themeName, const QMap<QString, StyleData>& styleData) {
		if (d->staticThemes.contains(themeName)) {
			return;
		}
		d->themes.insert(themeName, styleData);
		if (d->currentThemeName == themeName) {
			d->currentTheme = styleData;
		}
		emit themeModified(themeName);
	}

	/*!
		\since YSS 0.16.0
		\a themeName 主题名。
		return 如果该主题是静态主题，返回true；否则返回false。

		判断指定主题是否为静态主题。静态主题不允许被删除或修改。
	*/
	bool ColorThemeProvider::isStaticTheme(const QString& themeName) {
		return d->staticThemes.contains(themeName);
	}

	/*!
		\since YSS 0.16.0
		\a filePath 用作样式样本的文件的路径。

		设置用作样式样本的文件的路径。样式样本用于在配置页面预览编辑器中显示当前主题的样式效果。
		此函数不检查文件是否存在，由调用方自行保证路径有效。

		建议在此文件中尽可能多的展示所支持的全部配置节点键的样式，以便在配置页面预览编辑器中能完整展示当前主题的样式效果。
	*/
	void ColorThemeProvider::setTemplateTextPath(const QString& filePath) {
		d->templateTextPath = filePath;
	}

	/*!
		\since YSS 0.16.0
		return 用作样式样本的文件的路径。

		返回用作样式样本的文件的路径。
	*/
	QString ColorThemeProvider::getTemplateTextPath() {
		return d->templateTextPath;
	}

	/*!
		\since YSS 0.16.0
		\a themeName 主题名。
		\a styleName 配置节点键。
		return 指定主题下指定配置节点键对应的样式数据。如果主题或配置节点不存在，则返回默认构造的StyleData。

		返回指定主题下指定配置节点键对应的样式数据。
	*/
	StyleData ColorThemeProvider::getStyleData(const QString& themeName, const QString& styleName) {
		if (d->themes.contains(themeName)) {
			return d->themes.value(themeName).value(styleName);
		}
		return StyleData();
	}

	/*!
		\since YSS 0.16.0
		\a styleName 配置节点键。
		return 当前主题下指定配置节点键对应的样式数据。如果当前主题未设置或配置节点不存在，则返回默认构造的StyleData。

		返回当前主题下指定配置节点键对应的样式数据。
	*/
	StyleData ColorThemeProvider::getCurrentThemeStyleData(const QString& styleName) {
		return d->currentTheme.value(styleName);
	}

	/*!
		\since YSS 0.16.0
		return 当前主题下所有配置节点键对应的样式数据。如果当前主题未设置，则返回空的QMap。
	*/
	QMap<QString, StyleData> ColorThemeProvider::getCurrentThemeStyleData() {
		return d->currentTheme;
	}

	
	/*!
		\class YSSCore::Editor::StyleData
		\brief 样式数据，表示一个配置节点的样式信息，与YSSCore::Editor::ColorThemeProvider配合使用。
		\since YSS 0.16.0
		\inmodule YSSCore
		\ingroup LangService

		StyleData表示一个配置节点的样式信息，包括文本颜色、背景颜色、下划线颜色、下划线类型、加粗和斜体等属性。

		\note 这是个Q_GADGET类，支持Visindigo::Utility::JsonConfig的元对象方法，可以自动序列化和反序列化为JSON对象。
	*/
	QString StyleData::getStyleName() const {
		return styleName;
	}
	
	StyleData::IdentifierType StyleData::getIdentifierType() const {
		return identifierType;
	}

	void StyleData::setIdentifierType(IdentifierType type) {
		identifierType = type;
	}

	void StyleData::setStyleName(const QString& name) {
		styleName = name;
	}

	QColor StyleData::getTextColor() const {
		return textColor;
	}

	void StyleData::setTextColor(const QColor& color) {
		textColor = color;
	}

	QColor StyleData::getBgColor() const {
		return bgColor;
	}

	void StyleData::setBgColor(const QColor& color) {
		bgColor = color;
	}

	bool StyleData::isBgColorEnabled() const {
		return bgColorEnabled;
	}

	void StyleData::setBgColorEnabled(bool enabled) {
		bgColorEnabled = enabled;
	}
	
	QColor StyleData::getLineColor() const {
		return lineColor;
	}

	void StyleData::setLineColor(const QColor& color) {
		lineColor = color;
	}

	StyleData::UnderlineType StyleData::getUnderlineType() const {
		return underlineType;
	}

	void StyleData::setUnderlineType(UnderlineType type) {
		underlineType = type;
	}

	bool StyleData::isBold() const {
		return bold;
	}

	void StyleData::setBold(bool bold) {
		this->bold = bold;
	}

	bool StyleData::isItalic() const {
		return italic;
	}

	void StyleData::setItalic(bool italic) {
		this->italic = italic;
	}
}