#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QString>

namespace SystemInfo {

enum class DeviceType {
    Desktop,
    Laptop
};

struct HardwareSummary {
    QString model;
    QString cpu;
    QString memory;
    QString graphicsDriver;

    QString displayText() const;
};

bool isLaptopChassisType(int chassisType);
DeviceType detectDeviceType();
HardwareSummary collectHardwareSummary();

} // namespace SystemInfo

#endif // SYSTEMINFO_H
