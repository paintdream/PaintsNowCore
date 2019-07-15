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
		virtual void ExportConfig(std::vector<std::pair<String, String> >& config) const override;
		virtual void ImportConfig(const std::vector<std::pair<String, String> >& config) override;
	};
}


#endif // __ZFILTERPOD_H__