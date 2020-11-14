// WARNING: THIS FILE IS AUTOGENERATED! As such, it should not be edited.
// Edits need to be made to the proto files
// (see https://github.com/mavlink/MAVSDK-Proto/blob/master/protos/info/info.proto)

#pragma once

#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "plugin_base.h"

namespace mavsdk {

class System;
class InfoImpl;

/**
 * @brief Provide information about the hardware and/or software of a system.
 */
class Info : public PluginBase {
public:
    /**
     * @brief Constructor. Creates the plugin for a specific System.
     *
     * The plugin is typically created as shown below:
     *
     *     ```cpp
     *     auto info = Info(system);
     *     ```
     *
     * @param system The specific system associated with this plugin.
     */
    explicit Info(System& system); // deprecated

    /**
     * @brief Constructor. Creates the plugin for a specific System.
     *
     * The plugin is typically created as shown below:
     *
     *     ```cpp
     *     auto info = Info(system);
     *     ```
     *
     * @param system The specific system associated with this plugin.
     */
    explicit Info(std::shared_ptr<System> system); // new

    /**
     * @brief Destructor (internal use only).
     */
    ~Info();





    /**
     * @brief System flight information.
     */
    struct FlightInfo {
        
        uint32_t time_boot_ms{}; /**< @brief Time since system boot */
        uint64_t flight_uid{}; /**< @brief Flight counter. Starts from zero, is incremented at every disarm and is never reset (even after reboot) */
    };

    /**
     * @brief Equal operator to compare two `Info::FlightInfo` objects.
     *
     * @return `true` if items are equal.
     */
    friend bool operator==(const Info::FlightInfo& lhs, const Info::FlightInfo& rhs);

    /**
     * @brief Stream operator to print information about a `Info::FlightInfo`.
     *
     * @return A reference to the stream.
     */
    friend std::ostream& operator<<(std::ostream& str, Info::FlightInfo const& flight_info);




    /**
     * @brief System identification.
     */
    struct Identification {
        
        std::string hardware_uid{}; /**< @brief UID of the hardware. This refers to uid2 of MAVLink. If the system does not support uid2 yet, this is all zeros. */
    };

    /**
     * @brief Equal operator to compare two `Info::Identification` objects.
     *
     * @return `true` if items are equal.
     */
    friend bool operator==(const Info::Identification& lhs, const Info::Identification& rhs);

    /**
     * @brief Stream operator to print information about a `Info::Identification`.
     *
     * @return A reference to the stream.
     */
    friend std::ostream& operator<<(std::ostream& str, Info::Identification const& identification);




    /**
     * @brief System product information.
     */
    struct Product {
        
        int32_t vendor_id{}; /**< @brief ID of the board vendor */
        std::string vendor_name{}; /**< @brief Name of the vendor */
        int32_t product_id{}; /**< @brief ID of the product */
        std::string product_name{}; /**< @brief Name of the product */
    };

    /**
     * @brief Equal operator to compare two `Info::Product` objects.
     *
     * @return `true` if items are equal.
     */
    friend bool operator==(const Info::Product& lhs, const Info::Product& rhs);

    /**
     * @brief Stream operator to print information about a `Info::Product`.
     *
     * @return A reference to the stream.
     */
    friend std::ostream& operator<<(std::ostream& str, Info::Product const& product);




    /**
     * @brief System version information.
     */
    struct Version {
        
        int32_t flight_sw_major{}; /**< @brief Flight software major version */
        int32_t flight_sw_minor{}; /**< @brief Flight software minor version */
        int32_t flight_sw_patch{}; /**< @brief Flight software patch version */
        int32_t flight_sw_vendor_major{}; /**< @brief Flight software vendor major version */
        int32_t flight_sw_vendor_minor{}; /**< @brief Flight software vendor minor version */
        int32_t flight_sw_vendor_patch{}; /**< @brief Flight software vendor patch version */
        int32_t os_sw_major{}; /**< @brief Operating system software major version */
        int32_t os_sw_minor{}; /**< @brief Operating system software minor version */
        int32_t os_sw_patch{}; /**< @brief Operating system software patch version */
        std::string flight_sw_git_hash{}; /**< @brief Flight software git hash */
        std::string os_sw_git_hash{}; /**< @brief Operating system software git hash */
    };

    /**
     * @brief Equal operator to compare two `Info::Version` objects.
     *
     * @return `true` if items are equal.
     */
    friend bool operator==(const Info::Version& lhs, const Info::Version& rhs);

    /**
     * @brief Stream operator to print information about a `Info::Version`.
     *
     * @return A reference to the stream.
     */
    friend std::ostream& operator<<(std::ostream& str, Info::Version const& version);





    /**
     * @brief Possible results returned for info requests.
     */
    enum class Result {
        Unknown, /**< @brief Unknown result. */
        Success, /**< @brief Request succeeded. */
        InformationNotReceivedYet, /**< @brief Information has not been received yet. */
    };

    /**
     * @brief Stream operator to print information about a `Info::Result`.
     *
     * @return A reference to the stream.
     */
    friend std::ostream& operator<<(std::ostream& str, Info::Result const& result);



    /**
     * @brief Callback type for asynchronous Info calls.
     */
    using ResultCallback = std::function<void(Result)>;






    /**
     * @brief Get flight information of the system.
     *
     * This function is blocking.
     *
     * @return Result of request.
     */
    std::pair<Result, Info::FlightInfo> get_flight_information() const;






    /**
     * @brief Get the identification of the system.
     *
     * This function is blocking.
     *
     * @return Result of request.
     */
    std::pair<Result, Info::Identification> get_identification() const;






    /**
     * @brief Get product information of the system.
     *
     * This function is blocking.
     *
     * @return Result of request.
     */
    std::pair<Result, Info::Product> get_product() const;






    /**
     * @brief Get the version information of the system.
     *
     * This function is blocking.
     *
     * @return Result of request.
     */
    std::pair<Result, Info::Version> get_version() const;






    /**
     * @brief Get the speed factor of a simulation (with lockstep a simulation can run faster or slower than realtime).
     *
     * This function is blocking.
     *
     * @return Result of request.
     */
    std::pair<Result, double> get_speed_factor() const;




    /**
     * @brief Copy constructor.
     */
    Info(const Info& other);

    /**
     * @brief Equality operator (object is not copyable).
     */
    const Info& operator=(const Info&) = delete;

private:
    /** @private Underlying implementation, set at instantiation */
    std::unique_ptr<InfoImpl> _impl;
};

} // namespace mavsdk