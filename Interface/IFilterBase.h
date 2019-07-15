// IFilterBase.h
// By PaintDream (paintdream@paintdream.com)
// 2014-12-13
//

#ifndef __IFILTERBASE_H__
#define __IFILTERBASE_H__


#include "../PaintsNow.h"
#include "IDevice.h"
#include "IStreamBase.h"

namespace PaintsNow {
	class IFilterBase : public IDevice {
	public:
		virtual ~IFilterBase();
		// attach a stream, which all read operations depend on.
		virtual void ExportConfig(std::vector<std::pair<String, String> >& config) const;
		virtual void ImportConfig(const std::vector<std::pair<String, String> >& config);
		virtual IStreamBase* CreateFilter(IStreamBase& inputStream) = 0;
	};

	class NoFilter : public IFilterBase {
	public:
		virtual IStreamBase* CreateFilter(IStreamBase& inputStream);
	};
}



#endif // __IFILTERBASE_H__
