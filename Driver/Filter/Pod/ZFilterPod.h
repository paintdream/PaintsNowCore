// ZFilterPod.h
// By PaintDream (paintdream@paintdream.com)
// 2015-6-10
//

#ifndef __ZFILTERPOD_H__
#define __ZFILTERPOD_H__

#include "../../../Interface/IFilterBase.h"

namespace PaintsNow {
	class ZFilterPod final : public IFilterBase {
	public:
		virtual IStreamBase* CreateFilter(IStreamBase& stream) override;
	};
}


#endif // __ZFILTERPOD_H__