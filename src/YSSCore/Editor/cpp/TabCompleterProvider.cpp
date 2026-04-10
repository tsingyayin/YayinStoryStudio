#include <QtGui/qtextdocument.h>
#include "Editor/private/TabCompleterProvider_p.h"
#include "Editor/TabCompleterProvider.h"
#include "Editor/TextEdit.h"
namespace YSSCore::Editor {
	class TabCompleterItemPrivate {
		friend class TabCompleterItem;
	protected:
		TabCompleterItem::ItemType Type = TabCompleterItem::ItemType::Default;
		QString IconPath;
		QString Text;
		QString Description;
		QString Content;
		bool Alignment;
	};

	/*!
		\class YSSCore::Editor::TabCompleterItem
		\brief 代表TabCompleter中的一个补全项。
		\since YSS 0.13.0
		\inmodule YSSCore

		这类是个数据类，代表一个补全项，包含了补全项的文本、描述、内容、类型等信息。
	*/

	/*!
		\enum YSSCore::Editor::TabCompleterItem::ItemType
		\since YSS 0.13.0
		\value Default 默认类型，图标为默认图标。
		\value Value 代表一个值，图标为值图标。
		\value Const 代表一个常量，图标为常量图标。
		\value Enum 代表一个枚举，图标为枚举图标。
		\value Function 代表一个函数，图标为函数图标。
		\value Object 代表一个对象，图标为对象图标。
		\value Operator 代表一个操作符，图标为操作符图标。
		\value UserDefined 代表一个用户自定义类型，图标需要用户自己设置。
	*/
	/*!
		\since YSS 0.13.0
		默认构造函数。
	*/
	TabCompleterItem::TabCompleterItem()
		: d(new TabCompleterItemPrivate) {
		d->IconPath = "";
		d->Text = "";
		d->Description = "";
		d->Content = "";
		d->Alignment = true;
	}

	/*!
		\since YSS 0.13.0
		构造函数，接受补全项的文本、内容、描述、类型和对齐方式。
		\a text 补全项的文本，用于显示在补全列表里。
		\a content 补全项的内容，通常是插入到编辑器中的文本。
		\a description 补全项的描述，通常用于显示在补全列表中。
		\a type 补全项的类型，用于区分不同类型的补全项。
		\a alignment 是否对齐补全项，默认为true。

		对齐是个特殊概念，意味着当TextEdit尝试使用content插入到编辑器中时，如果alignment为true，则会自动
		从光标所在位置向前检查，查看content的内容中是否有一些内容已经存在，如果存在就忽略重复性内容，
		只补全剩余部分。比如content是"System.out.println"，当alignment为true时，如果光标前已经有"System."，则只会补全"out.println"。

		为false时，则不进行对齐，直接插入content的全部内容。一般来说应该使用true。
	*/
	TabCompleterItem::TabCompleterItem(QString text, QString content, QString description, ItemType type, bool alignment)
		: d(new TabCompleterItemPrivate) {
		d->IconPath = "";
		d->Text = text;
		d->Description = description;
		d->Content = content;
		d->Type = type;
		d->Alignment = alignment;
		setType(type, true);
	}

	/*!
		\since YSS 0.13.0
		析构函数。
	*/
	TabCompleterItem::~TabCompleterItem() {
		delete d;
	}

	/*!
		\fn YSSCore::Editor::TabCompleterItem::TabCompleterItem(TabCompleterItem&& other)
		\since YSS 0.13.0
		移动构造函数。
	*/

	/*!
		\fn YSSCore::Editor::TabCompleterItem::TabCompleterItem(const TabCompleterItem& other)
		\since YSS 0.13.0
		复制构造函数。
	*/

	/*!
		\fn YSSCore::Editor::TabCompleterItem& TabCompleterItem::operator=(TabCompleterItem&& other)
		\since YSS 0.13.0
		移动赋值运算符。
	*/

	/*!
		\fn TabCompleterItem& TabCompleterItem::operator=(const TabCompleterItem& other)
		\since YSS 0.13.0
		复制赋值运算符。
	*/
	VIMoveable_Impl(TabCompleterItem);

	VICopyable_Impl(TabCompleterItem);

	/*!
		\since YSS 0.13.0
		设置补全项的图标路径。
	*/
	void TabCompleterItem::setIconPath(const QString& iconPath) {
		d->IconPath = iconPath;
	}

	/*!
		\since YSS 0.13.0
		设置补全项的文本。
	*/
	void TabCompleterItem::setText(const QString& text) {
		d->Text = text;
	}

	/*!
		\since YSS 0.13.0
		设置补全项的描述。
	*/
	void TabCompleterItem::setDescription(const QString& description) {
		d->Description = description;
	}

	/*!
		\since YSS 0.13.0
		设置补全项的内容。
	*/
	void TabCompleterItem::setContent(const QString& content) {
		d->Content = content;
	}

	/*!
		\since YSS 0.13.0
		设置补全项的类型。
		\a type 补全项的类型。
		\a redirectIcon 是否根据类型自动设置图标路径，默认为true。
	*/
	void TabCompleterItem::setType(ItemType type, bool redirectIcon) {
		if (redirectIcon) {
			switch (type) {
			case ItemType::Default:
				d->IconPath = ":/resource/cn.yxgeneral.visindigo/icon/default.png";
				break;
			case ItemType::Value:
				d->IconPath = ":/resource/cn.yxgeneral.visindigo/icon/value.png";
				break;
			case ItemType::Const:
				d->IconPath = ":/resource/cn.yxgeneral.visindigo/icon/const.png";
				break;
			case ItemType::Enum:
				d->IconPath = ":/resource/cn.yxgeneral.visindigo/icon/enum.png";
				break;
			case ItemType::Function:
				d->IconPath = ":/resource/cn.yxgeneral.visindigo/icon/function.png";
				break;
			case ItemType::Object:
				d->IconPath = ":/resource/cn.yxgeneral.visindigo/icon/object.png";
				break;
			case ItemType::Operator:
				d->IconPath = ":/resource/cn.yxgeneral.visindigo/icon/operator.png";
				break;
			default:
				d->IconPath = ":/resource/cn.yxgeneral.visindigo/icon/default.png";
			}
		}
		d->Type = type;
	}

	/*!
		\since YSS 0.13.0
		获取补全项的图标路径。
	*/
	QString TabCompleterItem::getIconPath() const {
		return d->IconPath;
	}

	/*!
		\since YSS 0.13.0
		获取补全项的文本。
	*/
	QString TabCompleterItem::getText() const {
		return d->Text;
	}

	/*!
		\since YSS 0.13.0
		获取补全项的描述。
	*/
	QString TabCompleterItem::getDescription() const {
		return d->Description;
	}

	/*!
		\since YSS 0.13.0
		获取补全项的内容。
	*/
	QString TabCompleterItem::getContent() const {
		return d->Content;
	}

	/*!
		\since YSS 0.13.0
		获取补全项的类型。
	*/
	TabCompleterItem::ItemType TabCompleterItem::getType() const {
		return d->Type;
	}

	/*!
		\since YSS 0.13.0
		判断补全项是否对齐。
	*/
	bool TabCompleterItem::isAlignment() const {
		return d->Alignment;
	}

	/*!
		\class YSSCore::Editor::TabCompleterProvider
		\brief TabCompleter的提供者接口
		\since YSS 0.13.0
		\inmodule YSSCore
		
		这类是YSSCore::Editor::LangServer（语言服务器）的三大接口类其中之一，
		用于用户实现语言服务中的Tab补全功能。

		这类在初始化时会绑定到一个YSSCore::Editor::TextEdit上，用户可以通过getDocument()函数获取到该TextEdit的文档对象。
		不过一般来说，实现补全仅需要onTabComplete函数提供的参数即可，一般不需要额外操作文档对象。

		请注意，这个类的生命周期由TextEdit管理，用户不需要也不应该手动删除TabCompleterProvider对象。
		当TextEdit被销毁时，TabCompleterProvider也会被自动销毁。
	*/

	/*!
		\fn QList<YSSCore::Editor::TabCompleterItem> TabCompleterProvider::onTabComplete(qint32 lineNumber, qint32 column, const QString& content) = 0
		\since YSS 0.13.0
		纯虚函数，用户需要重写这个函数来提供Tab补全功能。
		\a lineNumber 光标所在的行号，从0开始。
		\a column 光标所在的列号，从0开始。
		\a 当前整行的内容。

		如果返回空列表，则表示没有补全项；如果返回非空列表，则会在编辑器中显示这些补全项供用户选择。
		编辑器不负责优化补全列表的顺序，用户应该根据实际情况自行排序补全项，以提供更好的用户体验。
	*/

	/*!
		\since YSS 0.13.0
		构造函数，接受一个TextEdit指针作为参数。
		\a textEdit 绑定的TextEdit对象。
		构造函数会将TabCompleterProvider绑定到传入的TextEdit上，以便提供补全功能。
	*/
	TabCompleterProvider::TabCompleterProvider(TextEdit* textEdit)
		: QObject(textEdit->getDocument()), d(new YSSCore::__Private__::TabCompleterProviderPrivate) {
		d->Q = this;
		d->Document = textEdit->getDocument();
	}

	/*!
		\since YSS 0.13.0
		析构函数。
	*/
	TabCompleterProvider::~TabCompleterProvider() {
		delete d;
	}

	/*!
		\since YSS 0.13.0
		获取绑定的TextEdit的文档对象。
	*/
	QTextDocument* TabCompleterProvider::getDocument() const {
		return d->Document;
	}
}