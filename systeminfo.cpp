#include "systeminfo.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

namespace SystemInfo {
namespace {

QString readFirstLine(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream stream(&file);
    return stream.readLine().trimmed();
}

bool hasBattery()
{
    const QDir powerSupplyDirectory(QStringLiteral("/sys/class/power_supply"));
    const QStringList supplies = powerSupplyDirectory.entryList(
        QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    for (const QString &supply : supplies) {
        const QString type = readFirstLine(
            powerSupplyDirectory.filePath(supply) + QStringLiteral("/type"));
        if (type.compare(QStringLiteral("Battery"), Qt::CaseInsensitive) == 0) {
            return true;
        }
    }

    return false;
}

} // namespace

bool isLaptopChassisType(int chassisType)
{
    // SMBIOS chassis types: portable, laptop, notebook, handheld,
    // sub-notebook, tablet, convertible and detachable.
    switch (chassisType) {
    case 8:
    case 9:
    case 10:
    case 11:
    case 14:
    case 30:
    case 31:
    case 32:
        return true;
    default:
        return false;
    }
}

DeviceType detectDeviceType()
{
    bool ok = false;
    const int chassisType = readFirstLine(
        QStringLiteral("/sys/class/dmi/id/chassis_type")).toInt(&ok);

    // SMBIOS values 3–36 identify a chassis class. Values 0–2 mean that
    // firmware did not provide a usable classification.
    if (ok && chassisType >= 3 && chassisType <= 36) {
        return isLaptopChassisType(chassisType) ? DeviceType::Laptop : DeviceType::Desktop;
    }

    // Some virtual machines and ARM devices do not expose chassis_type.
    return hasBattery() ? DeviceType::Laptop : DeviceType::Desktop;
}

} // namespace SystemInfo
