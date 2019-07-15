// TFactory.h
// By PaintDream (paintdream@paintdream.com)
// 2014-12-14
//

#ifndef __TFACTORY_H__
#define __TFACTORY_H__

#include "../PaintsNow.h"
#include "../Interface/IType.h"
#include "../Template/TProxy.h"
#include <string>

namespace PaintsNow {
	template <class T>
	class TFactoryBase : public TWrapper<T*, const String&> {
	public:
		TFactoryBase(const TWrapper<T*, const String&>& wrapper = TWrapper<T*, const String&>()) : TWrapper<T*, const String&>(wrapper) {}

		operator const TWrapper<void*, const String&>* () const {
			return reinterpret_cast<const TWrapper<void*, const String&>*>(this);
		}
		
		inline T* Create(const String& info = "") const {
			if (*this) {
				return (*this)(info);
			} else {
				return nullptr;
			}
		}
	};

	template <class T, class B>
	class TFactory : public TFactoryBase<B> {
	public:
		TFactory() : TFactoryBase<B>(Wrap(this, &TFactory::CreateEx)) {}

	private:
		B* CreateEx(const String& info = "") const {
			return new T();
		}
	};

	template <class T, class B>
	class TFactoryConstruct : public TFactoryBase<B> {
	public:
		TFactoryConstruct() : TFactoryBase<B>(Wrap(this, &TFactoryConstruct::CreateEx)) {}

	private:
		B* CreateEx(const String& info = "") const {
			return new T(info);
		}
	};
}


#endif // __TFACTORY_H__