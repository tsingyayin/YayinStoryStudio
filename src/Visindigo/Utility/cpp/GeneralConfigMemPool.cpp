#include "Utility/GeneralConfig.h"
#include "Utility/private/GeneralConfig_p.h"

namespace Visindigo::__Private__ {
	GeneralConfigMemPool* GeneralConfigMemPool::Instance = nullptr;

	GeneralConfigMemPool::GeneralConfigMemPool() {
		void* page = malloc(1024 * sizeof(GeneralConfigPrivate));
		MemPages.append(page);
		FreePrivates.reserve(1024);
		for (int i = 0; i < 1024; i++) {
			FreePrivates.push((GeneralConfigPrivate*)((qint64)page + i * sizeof(GeneralConfigPrivate)));
		}
	}

	GeneralConfigMemPool* GeneralConfigMemPool::getInstance() {
		if (!Instance){
			Instance = new GeneralConfigMemPool();
		}
		return Instance;
	}

	GeneralConfigPrivate* GeneralConfigMemPool::getPrivate(Visindigo::Utility::GeneralConfig* q) {
		if (FreePrivates.isEmpty()) {
			void* page = malloc(1024 * sizeof(GeneralConfigPrivate));
			MemPages.append(page);
			FreePrivates.reserve(FreePrivates.size() + 1024);
			for (int i = 0; i < 1024; i++) {
				FreePrivates.push((GeneralConfigPrivate*)((qint64)page + i * sizeof(GeneralConfigPrivate)));
			}
		}
		GeneralConfigPrivate* p = FreePrivates.pop();
		memset(p, 0, sizeof(GeneralConfigPrivate));
		new (p) GeneralConfigPrivate(q);
		return p;
	}

	void GeneralConfigMemPool::releasePrivate(GeneralConfigPrivate* p) {
		p->~GeneralConfigPrivate();
		FreePrivates.push(p);
	}
}