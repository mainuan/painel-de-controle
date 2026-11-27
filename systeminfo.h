#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

namespace SystemInfo {

enum class DeviceType {
    Desktop,
    Laptop
};

bool isLaptopChassisType(int chassisType);
DeviceType detectDeviceType();

} // namespace SystemInfo

#endif // SYSTEMINFO_H
