#include "systeminfo.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QRegularExpression>
#include <QStringList>
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

QString cleanDmiValue(QString value)
{
    value = value.trimmed();
    const QString normalized = value.toLower();

    const QStringList placeholders = {
        QStringLiteral("default string"),
        QStringLiteral("to be filled by o.e.m."),
        QStringLiteral("to be filled by oem"),
        QStringLiteral("system product name"),
        QStringLiteral("system manufacturer"),
        QStringLiteral("not specified"),
        QStringLiteral("unknown")
    };

    for (const QString &placeholder : placeholders) {
        if (normalized == placeholder) {
            return {};
        }
    }

    return value;
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

QString readCpuModel()
{
    QFile file(QStringLiteral("/proc/cpuinfo"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream stream(&file);
    QString fallback;
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const qsizetype separator = line.indexOf(QLatin1Char(':'));
        if (separator < 0) {
            continue;
        }

        const QString key = line.left(separator).trimmed();
        const QString value = line.mid(separator + 1).trimmed();
        if (value.isEmpty()) {
            continue;
        }

        if (key.compare(QStringLiteral("model name"), Qt::CaseInsensitive) == 0) {
            return value;
        }

        if (key.compare(QStringLiteral("hardware"), Qt::CaseInsensitive) == 0
            || key.compare(QStringLiteral("model"), Qt::CaseInsensitive) == 0) {
            fallback = value;
        } else if (key.compare(QStringLiteral("processor"), Qt::CaseInsensitive) == 0
                   && !value.contains(QRegularExpression(QStringLiteral("^\\d+$")))) {
            fallback = value;
        }
    }

    return fallback;
}

QString readMemorySize()
{
    QFile file(QStringLiteral("/proc/meminfo"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream stream(&file);
    const QRegularExpression whitespace(QStringLiteral("\\s+"));
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (!line.startsWith(QStringLiteral("MemTotal:"))) {
            continue;
        }

        const QStringList fields = line.split(whitespace, Qt::SkipEmptyParts);
        if (fields.size() < 2) {
            break;
        }

        bool ok = false;
        const qulonglong kibibytes = fields.at(1).toULongLong(&ok);
        if (ok && kibibytes > 0) {
            const double gibibytes = static_cast<double>(kibibytes) / (1024.0 * 1024.0);
            return QStringLiteral("%1 GiB").arg(QLocale().toString(gibibytes, 'f', 1));
        }
        break;
    }

    return {};
}

QString readGraphicsDriver()
{
    const QDir drmDirectory(QStringLiteral("/sys/class/drm"));
    const QStringList entries = drmDirectory.entryList(
        QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    static const QRegularExpression cardName(QStringLiteral("^card[0-9]+$"));

    for (const QString &entry : entries) {
        if (!cardName.match(entry).hasMatch()) {
            continue;
        }

        const QString driverPath = drmDirectory.filePath(
            entry + QStringLiteral("/device/driver"));
        const QString canonicalPath = QFileInfo(driverPath).canonicalFilePath();
        if (!canonicalPath.isEmpty()) {
            const QString driver = QFileInfo(canonicalPath).fileName();
            if (!driver.isEmpty()) {
                return driver;
            }
        }
    }

    return {};
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

HardwareSummary collectHardwareSummary()
{
    HardwareSummary summary;

    const QString vendor = cleanDmiValue(
        readFirstLine(QStringLiteral("/sys/class/dmi/id/sys_vendor")));
    const QString product = cleanDmiValue(
        readFirstLine(QStringLiteral("/sys/class/dmi/id/product_name")));

    if (!vendor.isEmpty() && !product.isEmpty()
        && vendor.compare(product, Qt::CaseInsensitive) != 0) {
        summary.model = vendor + QLatin1Char(' ') + product;
    } else {
        summary.model = product.isEmpty() ? vendor : product;
    }

    summary.cpu = readCpuModel();
    summary.memory = readMemorySize();
    summary.graphicsDriver = readGraphicsDriver();

    return summary;
}

QString HardwareSummary::displayText() const
{
    QStringList lines;

    if (!model.isEmpty()) {
        lines << model;
    }
    if (!cpu.isEmpty()) {
        lines << QStringLiteral("CPU · %1").arg(cpu);
    }
    if (!memory.isEmpty()) {
        lines << QStringLiteral("RAM · %1").arg(memory);
    }
    if (!graphicsDriver.isEmpty()) {
        lines << QStringLiteral("GPU · %1").arg(graphicsDriver);
    }

    return lines.isEmpty() ? QStringLiteral("—") : lines.join(QLatin1Char('\n'));
}

} // namespace SystemInfo
