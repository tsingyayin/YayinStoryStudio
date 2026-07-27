#ifndef Visindigo_Widgets_private_IconFontRouter_SegoeFluent_p_h
#define Visindigo_Widgets_private_IconFontRouter_SegoeFluent_p_h
#include "Widgets/IconFontRouter.h"

namespace Visindigo::__Private__ {
	class IconFontRouter_SegoeFluent :public Visindigo::Widgets::IconFontRouter {
		static IconFontRouter_SegoeFluent* Instance;
	public:
		static IconFontRouter_SegoeFluent* getRouter();
		IconFontRouter_SegoeFluent();
		virtual ~IconFontRouter_SegoeFluent() override;
		virtual QChar getChar(IconName iconName) override;
	};
}
#endif // Visindigo_Widgets_private_IconFontRouter_SegoeFluent_p_h
