#include "xsensereciever.hpp"
#include <xstypes/xsdataidentifier.h>
#include <xstypes/xsxbusmessageid.h>

bool CallbackHandler::packetAvailable() const {
  xsens::Lock locky(&m_mutex);
  return m_numberOfPacketsInBuffer > 0;
}

XsDataPacket CallbackHandler::getNextPacket() {
  assert(packetAvailable());
  xsens::Lock locky(&m_mutex);
  XsDataPacket oldestPacket(m_packetBuffer.front());
  m_packetBuffer.pop_front();
  --m_numberOfPacketsInBuffer;
  return oldestPacket;
}

void CallbackHandler::onLiveDataAvailable(XsDevice *,
                                          const XsDataPacket *packet) {
  xsens::Lock locky(&m_mutex);
  assert(packet != 0);
  while (m_numberOfPacketsInBuffer >= m_maxNumberOfPacketsInBuffer)
    (void)getNextPacket();

  m_packetBuffer.push_back(*packet);
  ++m_numberOfPacketsInBuffer;
  assert(m_numberOfPacketsInBuffer <= m_maxNumberOfPacketsInBuffer);
}

XsensReciever::XsensReciever() {

  control = XsControl::construct();
  assert(control != 0);
  XsPortInfoArray portInfoArray = XsScanner::scanPorts();
  for (auto const &portInfo : portInfoArray) {
    if (portInfo.deviceId().isMti() || portInfo.deviceId().isMtig()) {
      mtPort = portInfo;
      break;
    }
  }
  if (!control->openPort(mtPort.portName().toStdString(), mtPort.baudrate())) {
  }
  // throw invalid_argument("Cannot open port");
  device = control->device(mtPort.deviceId());
  assert(device != 0);
  callback = new CallbackHandler();
  device->addCallbackHandler(callback);
}
ResultCode XsensReciever::setConfigs() {

  if (!device->gotoConfig()) {
  }
  // throw invalid_argument("Cannot set config mode");
  device->readEmtsAndDeviceConfiguration();

  XsOutputConfigurationArray configArray;
  configArray.push_back(XsOutputConfiguration(XDI_PacketCounter, 0));
  configArray.push_back(XsOutputConfiguration(XDI_SampleTimeFine, 0));

  // configArray.push_back(XsOutputConfiguration(XDI_AccelerationHR, 2000));
  // configArray.push_back(XsOutputConfiguration(XDI_RateOfTurnHR, 1600));
  // configArray.push_back(XsOutputConfiguration(XDI_MagneticField, 400));
  configArray.push_back(XsOutputConfiguration(XDI_Quaternion, 400));
  // configArray.push_back(XsOutputConfiguration(XDI_LatLon, 100));
  configArray.push_back(XsOutputConfiguration(XDI_GnssPvtData, 4));
  // configArray.push_back(XsOutputConfiguration(XDI_AltitudeEllipsoid, 400));
  // configArray.push_back(XsOutputConfiguration(XDI_VelocityXYZ, 400));
  // configArray.push_back(XsOutputConfiguration(XDI_DeltaQ, 500));
  // configArray.push_back(XsOutputConfiguration(XDI_DeltaV, 500));

  // configArray.push_back(XsOutputConfiguration(XDI_, 400));

  if (!device->setOutputConfiguration(configArray))
    // throw invalid_argument("Could not configure MTi device. Aborting.");
    return ResultCode::NOT_OK;
  return ResultCode::OK;
}
ResultCode XsensReciever::recieve() {
  if (!device->gotoMeasurement())
    return ResultCode::NOT_OK;
  // throw invalid_argument(
  //     "Could not put device into measurement mode. Aborting.");
  if (!device->startRecording())
    return ResultCode::NOT_OK;

  // throw invalid_argument("Failed to start recording. Aborting.");

  while (true) {
    if (callback->packetAvailable()) {
      GnssImu pack;

      XsDataPacket packet = callback->getNextPacket();

      if (packet.containsOrientation()) {

        pack.eulers = packet.orientationEuler();
        std::cout << pack.eulers.roll() << "   " << pack.eulers.pitch() << "   "
                  << pack.eulers.yaw() << "\n";
      }

      // if (packet.containsLatitudeLongitude()) {
      //   pack.latLon = packet.latitudeLongitude();
      //   std::cout << pack.latLon[0] << "   " << pack.latLon[1] << "\n\n";
      // }
      if (packet.containsRawGnssPvtData()) {
        pack.lat =
            static_cast<double>(packet.rawGnssPvtData().m_lat) / 10000000;
        pack.lon =
            static_cast<double>(packet.rawGnssPvtData().m_lon) / 10000000;
        std::cout << pack.lat << "   " << pack.lon << "\n\n";
      }

      dataqueue.push(pack);
    }
    XsTime::msleep(0);
  }
}
XsensReciever::GnssImu XsensReciever::getdata() {
  GnssImu data;
  data = dataqueue.front();
  dataqueue.pop();
  return data;
}
