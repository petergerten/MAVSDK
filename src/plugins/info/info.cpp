// WARNING: THIS FILE IS AUTOGENERATED! As such, it should not be edited.
// Edits need to be made to the proto files
// (see https://github.com/mavlink/MAVSDK-Proto/blob/master/protos/info/info.proto)

#include <iomanip>

#include "info_impl.h"
#include "plugins/info/info.h"

namespace mavsdk {

using FlightInfo = Info::FlightInfo;
using Identification = Info::Identification;
using Product = Info::Product;
using Version = Info::Version;



Info::Info(System& system) : PluginBase(), _impl{new InfoImpl(system)} {}

Info::Info(std::shared_ptr<System> system) : PluginBase(), _impl{new InfoImpl(system)} {}

Info::~Info() {}





std::pair<Info::Result, Info::FlightInfo> Info::get_flight_information() const
{
    return _impl->get_flight_information();
}





std::pair<Info::Result, Info::Identification> Info::get_identification() const
{
    return _impl->get_identification();
}





std::pair<Info::Result, Info::Product> Info::get_product() const
{
    return _impl->get_product();
}





std::pair<Info::Result, Info::Version> Info::get_version() const
{
    return _impl->get_version();
}





std::pair<Info::Result, double> Info::get_speed_factor() const
{
    return _impl->get_speed_factor();
}



bool operator==(const Info::FlightInfo& lhs, const Info::FlightInfo& rhs)
{
    return
        (rhs.time_boot_ms == lhs.time_boot_ms) &&
        (rhs.flight_uid == lhs.flight_uid);
}

std::ostream& operator<<(std::ostream& str, Info::FlightInfo const& flight_info)
{
    str << std::setprecision(15);
    str << "flight_info:" << '\n'
        << "{\n";
    str << "    time_boot_ms: " << flight_info.time_boot_ms << '\n';
    str << "    flight_uid: " << flight_info.flight_uid << '\n';
    str << '}';
    return str;
}


bool operator==(const Info::Identification& lhs, const Info::Identification& rhs)
{
    return
        (rhs.hardware_uid == lhs.hardware_uid);
}

std::ostream& operator<<(std::ostream& str, Info::Identification const& identification)
{
    str << std::setprecision(15);
    str << "identification:" << '\n'
        << "{\n";
    str << "    hardware_uid: " << identification.hardware_uid << '\n';
    str << '}';
    return str;
}


bool operator==(const Info::Product& lhs, const Info::Product& rhs)
{
    return
        (rhs.vendor_id == lhs.vendor_id) &&
        (rhs.vendor_name == lhs.vendor_name) &&
        (rhs.product_id == lhs.product_id) &&
        (rhs.product_name == lhs.product_name);
}

std::ostream& operator<<(std::ostream& str, Info::Product const& product)
{
    str << std::setprecision(15);
    str << "product:" << '\n'
        << "{\n";
    str << "    vendor_id: " << product.vendor_id << '\n';
    str << "    vendor_name: " << product.vendor_name << '\n';
    str << "    product_id: " << product.product_id << '\n';
    str << "    product_name: " << product.product_name << '\n';
    str << '}';
    return str;
}


bool operator==(const Info::Version& lhs, const Info::Version& rhs)
{
    return
        (rhs.flight_sw_major == lhs.flight_sw_major) &&
        (rhs.flight_sw_minor == lhs.flight_sw_minor) &&
        (rhs.flight_sw_patch == lhs.flight_sw_patch) &&
        (rhs.flight_sw_vendor_major == lhs.flight_sw_vendor_major) &&
        (rhs.flight_sw_vendor_minor == lhs.flight_sw_vendor_minor) &&
        (rhs.flight_sw_vendor_patch == lhs.flight_sw_vendor_patch) &&
        (rhs.os_sw_major == lhs.os_sw_major) &&
        (rhs.os_sw_minor == lhs.os_sw_minor) &&
        (rhs.os_sw_patch == lhs.os_sw_patch) &&
        (rhs.flight_sw_git_hash == lhs.flight_sw_git_hash) &&
        (rhs.os_sw_git_hash == lhs.os_sw_git_hash);
}

std::ostream& operator<<(std::ostream& str, Info::Version const& version)
{
    str << std::setprecision(15);
    str << "version:" << '\n'
        << "{\n";
    str << "    flight_sw_major: " << version.flight_sw_major << '\n';
    str << "    flight_sw_minor: " << version.flight_sw_minor << '\n';
    str << "    flight_sw_patch: " << version.flight_sw_patch << '\n';
    str << "    flight_sw_vendor_major: " << version.flight_sw_vendor_major << '\n';
    str << "    flight_sw_vendor_minor: " << version.flight_sw_vendor_minor << '\n';
    str << "    flight_sw_vendor_patch: " << version.flight_sw_vendor_patch << '\n';
    str << "    os_sw_major: " << version.os_sw_major << '\n';
    str << "    os_sw_minor: " << version.os_sw_minor << '\n';
    str << "    os_sw_patch: " << version.os_sw_patch << '\n';
    str << "    flight_sw_git_hash: " << version.flight_sw_git_hash << '\n';
    str << "    os_sw_git_hash: " << version.os_sw_git_hash << '\n';
    str << '}';
    return str;
}



std::ostream& operator<<(std::ostream& str, Info::Result const& result)
{
    switch (result) {
        case Info::Result::Unknown:
            return str << "Unknown";
        case Info::Result::Success:
            return str << "Success";
        case Info::Result::InformationNotReceivedYet:
            return str << "Information Not Received Yet";
        default:
            return str << "Unknown";
    }
}




} // namespace mavsdk