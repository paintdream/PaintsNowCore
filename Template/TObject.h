// TObject.h
// By PaintDream (paintdream@paintdream.com)
// 2014-12-19
//

#ifndef __TOBJECT_H__
#define __TOBJECT_H__

#include "../Template/TProxy.h"

namespace PaintsNow {
	template <class T>
	class TObject {
	public:
		TObject() {}
		virtual ~TObject() {}
		virtual TObject& operator ()(T& t) = 0;
	};
}


#endif // __ZOBJECT_H__
