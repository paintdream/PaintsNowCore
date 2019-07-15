// IDevice.h
// By PaintDream (paintdream@paintdream.com)
// 2018-3-12
//

#ifndef __IDEVICE_H__
#define __IDEVICE_H__

namespace PaintsNow {
	class IDevice {
	public:
		virtual ~IDevice();
		virtual void ReleaseDevice();
	};
}

#endif // __IDEVICE_H__