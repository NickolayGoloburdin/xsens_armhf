#include <iomanip>
#include <iostream>
#include <list>
#include <queue>
#include <stdexcept>
#include <string>
#include <xscontroller/xscontrol_def.h>
#include <xscontroller/xsdevice_def.h>
#include <xscontroller/xsscanner.h>
#include <xstypes/xsdataidentifier.h>
#include <xstypes/xsdatapacket.h>
#include <xstypes/xsdeviceid.h>
#include <xstypes/xseuler.h>
#include <xstypes/xsoutputconfigurationarray.h>
#include <xstypes/xsportinfo.h>
#include <xstypes/xstime.h>

class CallbackHandler : public XsCallback {
public:
  // CallbackHandler(size_t maxBufferSize = 5)
  //     : m_maxNumberOfPacketsInBuffer(maxBufferSize),
  //       m_numberOfPacketsInBuffer(0){};
  // virtual ~CallbackHandler();
  CallbackHandler(size_t maxBufferSize = 5)
      : m_maxNumberOfPacketsInBuffer(maxBufferSize),
        m_numberOfPacketsInBuffer(0){};

  virtual ~CallbackHandler() {}
  XsDataPacket getNextPacket();
  bool packetAvailable() const;

protected:
  void onLiveDataAvailable(XsDevice *, const XsDataPacket *packet) override;

private:
  mutable xsens::Mutex m_mutex;

  size_t m_maxNumberOfPacketsInBuffer;
  size_t m_numberOfPacketsInBuffer;
  std::queue<XsDataPacket> m_packetBuffer;
};
enum ResultCode { OK, NOT_OK };
class XsensReciever {

public:
  XsensReciever();

  ResultCode setConfigs();
  struct GnssImu {
    XsEuler eulers;
    double lat;
    double lon;
  };
  GnssImu getdata();
  std::queue<GnssImu> dataqueue;
  ResultCode recieve();

private:
  XsControl *control;
  XsPortInfo mtPort;
  XsDeviceId id;
  XsPortInfo portName;
  XsPortInfo baudrate;
  XsDevice *device;
  CallbackHandler *callback = nullptr;
};
