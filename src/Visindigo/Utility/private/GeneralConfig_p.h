#ifndef Visindigo_Utility_GeneralConfig_p_h
#define Visindigo_Utility_GeneralConfig_p_h
#include "Utility/GeneralConfig.h"
#include <QtCore/qstack.h>
namespace Visindigo::__Private__ {
	class GeneralConfigPrivate {
		friend class Visindigo::Utility::GeneralConfig;
		friend class GeneralConfigMemPool;
	protected:
		Visindigo::Utility::GeneralConfig* q = nullptr;
		Visindigo::Utility::GeneralConfig::Type Type = Visindigo::Utility::GeneralConfig::None;
		qint64 Value = 0; // danger value, MUST check Type before use
		Visindigo::Utility::GeneralConfig* Root = nullptr;
		Visindigo::Utility::GeneralConfig* Parent = nullptr;
		QString HeadComment;
		QString InlineComment;
		GeneralConfigPrivate(Visindigo::Utility::GeneralConfig* q);
		~GeneralConfigPrivate();
		void copyFrom(GeneralConfigPrivate* other);
		GeneralConfigPrivate* find(QString key, bool autoCreate = false, bool* justCreated = nullptr);
		GeneralConfigPrivate* find(QStringList* nodes, bool autoCreate, bool* justCreated);
		GeneralConfigPrivate* create(const QString& key);
		GeneralConfigPrivate* create(QStringList* nodes);
		bool contains(const QString& key);
		void changeRoot(Visindigo::Utility::GeneralConfig* newRoot);
		void changeQ(Visindigo::Utility::GeneralConfig* newQ);
		void clear();
	};

	class GeneralConfigMemPool {
		static GeneralConfigMemPool* Instance;
		QList<void*> MemPages;
		QStack<GeneralConfigPrivate*> FreePrivates;
		GeneralConfigMemPool();
	public:
		static GeneralConfigMemPool* getInstance();
		GeneralConfigPrivate* getPrivate(Visindigo::Utility::GeneralConfig* q);
		void releasePrivate(GeneralConfigPrivate* p);
	};
}
#define VPGCMP Visindigo::__Private__::GeneralConfigMemPool::getInstance()
#endif // Visindigo_Utility_GeneralConfig_p_h