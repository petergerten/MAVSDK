#include <cmath>
#include "global_include.h"
#include "offboard_impl.h"
#include "mavsdk_impl.h"
#include "px4_custom_mode.h"

namespace mavsdk {

OffboardImpl::OffboardImpl(System& system) : PluginImplBase(system)
{
    _parent->register_plugin(this);
}

OffboardImpl::OffboardImpl(std::shared_ptr<System> system) : PluginImplBase(system)
{
    _parent->register_plugin(this);
}

OffboardImpl::~OffboardImpl()
{
    _parent->unregister_plugin(this);
}

void OffboardImpl::init()
{
    // We need the system state.
    _parent->register_mavlink_message_handler(
        MAVLINK_MSG_ID_HEARTBEAT,
        std::bind(&OffboardImpl::process_heartbeat, this, std::placeholders::_1),
        this);
}

void OffboardImpl::deinit()
{
    stop_sending_setpoints();
    _parent->unregister_all_mavlink_message_handlers(this);
}

void OffboardImpl::enable() {}

void OffboardImpl::disable() {}

Offboard::Result OffboardImpl::start()
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_mode == Mode::NotActive) {
            return Offboard::Result::NoSetpointSet;
        }
        _last_started = _time.steady_time();
    }

    return offboard_result_from_command_result(
        _parent->set_flight_mode(SystemImpl::FlightMode::Offboard));
}

Offboard::Result OffboardImpl::stop()
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_mode != Mode::NotActive) {
            stop_sending_setpoints();
        }
    }

    return offboard_result_from_command_result(
        _parent->set_flight_mode(SystemImpl::FlightMode::Hold));
}

void OffboardImpl::start_async(Offboard::ResultCallback callback)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_mode == Mode::NotActive) {
            if (callback) {
                callback(Offboard::Result::NoSetpointSet);
            }
            return;
        }
        _last_started = _time.steady_time();
    }

    _parent->set_flight_mode_async(
        SystemImpl::FlightMode::Offboard,
        std::bind(&OffboardImpl::receive_command_result, this, std::placeholders::_1, callback));
}

void OffboardImpl::stop_async(Offboard::ResultCallback callback)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_mode != Mode::NotActive) {
            stop_sending_setpoints();
        }
    }

    _parent->set_flight_mode_async(
        SystemImpl::FlightMode::Hold,
        std::bind(&OffboardImpl::receive_command_result, this, std::placeholders::_1, callback));
}

bool OffboardImpl::is_active()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return (_mode != Mode::NotActive);
}

void OffboardImpl::receive_command_result(
    MavlinkCommandSender::Result result, const Offboard::ResultCallback& callback)
{
    Offboard::Result offboard_result = offboard_result_from_command_result(result);
    if (callback) {
        callback(offboard_result);
    }
}

Offboard::Result OffboardImpl::set_position_ned(Offboard::PositionNedYaw position_ned_yaw)
{
    _mutex.lock();
    _position_ned_yaw = position_ned_yaw;

    if (_mode != Mode::PositionNed) {
        if (_call_every_cookie) {
            // If we're already sending other setpoints, stop that now.
            _parent->remove_call_every(_call_every_cookie);
            _call_every_cookie = nullptr;
        }
        // We automatically send Ned setpoints from now on.
        _parent->add_call_every(
            [this]() { send_position_ned(); }, SEND_INTERVAL_S, &_call_every_cookie);

        _mode = Mode::PositionNed;
    } else {
        // We're already sending these kind of setpoints. Since the setpoint change, let's
        // reschedule the next call, so we don't send setpoints too often.
        _parent->reset_call_every(_call_every_cookie);
    }
    _mutex.unlock();

    // also send it right now to reduce latency
    return send_position_ned();
}

Offboard::Result OffboardImpl::set_velocity_ned(Offboard::VelocityNedYaw velocity_ned_yaw)
{
    _mutex.lock();
    _velocity_ned_yaw = velocity_ned_yaw;

    if (_mode != Mode::VelocityNed) {
        if (_call_every_cookie) {
            // If we're already sending other setpoints, stop that now.
            _parent->remove_call_every(_call_every_cookie);
            _call_every_cookie = nullptr;
        }
        // We automatically send Ned setpoints from now on.
        _parent->add_call_every(
            [this]() { send_velocity_ned(); }, SEND_INTERVAL_S, &_call_every_cookie);

        _mode = Mode::VelocityNed;
    } else {
        // We're already sending these kind of setpoints. Since the setpoint change, let's
        // reschedule the next call, so we don't send setpoints too often.
        _parent->reset_call_every(_call_every_cookie);
    }
    _mutex.unlock();

    // also send it right now to reduce latency
    return send_velocity_ned();
}


Offboard::Result OffboardImpl::set_position_global(Offboard::PositionGlobalYaw position_global_yaw)
{
    _mutex.lock();
    _position_global_yaw = position_global_yaw;

    if (_mode != Mode::PositionGlobal) {
        if (_call_every_cookie) {
            // If we're already sending other setpoints, stop that now.
            _parent->remove_call_every(_call_every_cookie);
            _call_every_cookie = nullptr;
        }
        // We automatically send Ned setpoints from now on.
        _parent->add_call_every(
            [this]() { send_position_global(); }, SEND_INTERVAL_S, &_call_every_cookie);

        _mode = Mode::PositionGlobal;
    } else {
        // We're already sending these kind of setpoints. Since the setpoint change, let's
        // reschedule the next call, so we don't send setpoints too often.
        _parent->reset_call_every(_call_every_cookie);
    }
    _mutex.unlock();

    // also send it right now to reduce latency
    return send_position_global();
}

Offboard::Result OffboardImpl::set_velocity_global(Offboard::VelocityGlobalYaw velocity_global_yaw)
{
    _mutex.lock();
    _velocity_global_yaw = velocity_global_yaw;

    if (_mode != Mode::VelocityGlobal) {
        if (_call_every_cookie) {
            // If we're already sending other setpoints, stop that now.
            _parent->remove_call_every(_call_every_cookie);
            _call_every_cookie = nullptr;
        }
        // We automatically send Ned setpoints from now on.
        _parent->add_call_every(
            [this]() { send_velocity_global(); }, SEND_INTERVAL_S, &_call_every_cookie);

        _mode = Mode::VelocityGlobal;
    } else {
        // We're already sending these kind of setpoints. Since the setpoint change, let's
        // reschedule the next call, so we don't send setpoints too often.
        _parent->reset_call_every(_call_every_cookie);
    }
    _mutex.unlock();

    // also send it right now to reduce latency
    return send_velocity_global();
}

  

Offboard::Result
OffboardImpl::set_velocity_body(Offboard::VelocityBodyYawspeed velocity_body_yawspeed)
{
    _mutex.lock();
    _velocity_body_yawspeed = velocity_body_yawspeed;

    if (_mode != Mode::VelocityBody) {
        if (_call_every_cookie) {
            // If we're already sending other setpoints, stop that now.
            _parent->remove_call_every(_call_every_cookie);
            _call_every_cookie = nullptr;
        }
        // We automatically send body setpoints from now on.
        _parent->add_call_every(
            [this]() { send_velocity_body(); }, SEND_INTERVAL_S, &_call_every_cookie);

        _mode = Mode::VelocityBody;
    } else {
        // We're already sending these kind of setpoints. Since the setpoint change, let's
        // reschedule the next call, so we don't send setpoints too often.
        _parent->reset_call_every(_call_every_cookie);
    }
    _mutex.unlock();

    // also send it right now to reduce latency
    return send_velocity_body();
}

Offboard::Result OffboardImpl::set_attitude(Offboard::Attitude attitude)
{
    _mutex.lock();
    _attitude = attitude;

    if (_mode != Mode::Attitude) {
        if (_call_every_cookie) {
            // If we're already sending other setpoints, stop that now.
            _parent->remove_call_every(_call_every_cookie);
            _call_every_cookie = nullptr;
        }
        // We automatically send body setpoints from now on.
        _parent->add_call_every(
            [this]() { send_attitude(); }, SEND_INTERVAL_S, &_call_every_cookie);

        _mode = Mode::Attitude;
    } else {
        // We're already sending these kind of setpoints. Since the setpoint change, let's
        // reschedule the next call, so we don't send setpoints too often.
        _parent->reset_call_every(_call_every_cookie);
    }
    _mutex.unlock();

    // also send it right now to reduce latency
    return send_attitude();
}

Offboard::Result OffboardImpl::set_attitude_rate(Offboard::AttitudeRate attitude_rate)
{
    _mutex.lock();
    _attitude_rate = attitude_rate;

    if (_mode != Mode::AttitudeRate) {
        if (_call_every_cookie) {
            // If we're already sending other setpoints, stop that now.
            _parent->remove_call_every(_call_every_cookie);
            _call_every_cookie = nullptr;
        }
        // We automatically send body setpoints from now on.
        _parent->add_call_every(
            [this]() { send_attitude_rate(); }, SEND_INTERVAL_S, &_call_every_cookie);

        _mode = Mode::AttitudeRate;
    } else {
        // We're already sending these kind of setpoints. Since the setpoint change, let's
        // reschedule the next call, so we don't send setpoints too often.
        _parent->reset_call_every(_call_every_cookie);
    }
    _mutex.unlock();

    // also send it right now to reduce latency
    return send_attitude_rate();
}

Offboard::Result OffboardImpl::set_actuator_control(Offboard::ActuatorControl actuator_control)
{
    _mutex.lock();
    _actuator_control = actuator_control;

    if (_mode != Mode::ActuatorControl) {
        if (_call_every_cookie) {
            // If we're already sending other setpoints, stop that now.
            _parent->remove_call_every(_call_every_cookie);
            _call_every_cookie = nullptr;
        }
        // We automatically send motor rate values from now on.
        _parent->add_call_every(
            [this]() { send_actuator_control(); }, SEND_INTERVAL_S, &_call_every_cookie);

        _mode = Mode::ActuatorControl;
    } else {
        // We're already sending these kind of values. Since the value changes, let's
        // reschedule the next call, so we don't send values too often.
        _parent->reset_call_every(_call_every_cookie);
    }
    _mutex.unlock();

    // also send it right now to reduce latency
    return send_actuator_control();
}

Offboard::Result OffboardImpl::send_position_global()
{
    // const static uint16_t IGNORE_X = (1 << 0);
    // const static uint16_t IGNORE_Y = (1 << 1);
    // const static uint16_t IGNORE_Z = (1 << 2);
    const static uint16_t IGNORE_VX = (1 << 3);
    const static uint16_t IGNORE_VY = (1 << 4);
    const static uint16_t IGNORE_VZ = (1 << 5);
    const static uint16_t IGNORE_AX = (1 << 6);
    const static uint16_t IGNORE_AY = (1 << 7);
    const static uint16_t IGNORE_AZ = (1 << 8);
    // const static uint16_t IS_FORCE = (1 << 9);
    // const static uint16_t IGNORE_YAW = (1 << 10);
    const static uint16_t IGNORE_YAW_RATE = (1 << 11);

    _mutex.lock();
    const float yaw = to_rad_from_deg(_position_global_yaw.yaw_deg);
    const float yaw_rate = 0.0f;
    const float lat_int = _position_global_yaw.lat;
    const float lon_int = _position_global_yaw.lon;
    const float alt = _position_global_yaw.alt;
    const float vx = 0.0f;
    const float vy = 0.0f;
    const float vz = 0.0f;
    const float afx = 0.0f;
    const float afy = 0.0f;
    const float afz = 0.0f;
    _mutex.unlock();

    mavlink_message_t message;
    mavlink_msg_set_position_target_global_int_pack(
        _parent->get_own_system_id(),
        _parent->get_own_component_id(),
        &message,
        static_cast<uint32_t>(_parent->get_time().elapsed_s() * 1e3),
        _parent->get_system_id(),
        _parent->get_autopilot_id(),
        MAV_FRAME_GLOBAL_INT,
        IGNORE_VX | IGNORE_VY | IGNORE_VZ | IGNORE_AX | IGNORE_AY | IGNORE_AZ | IGNORE_YAW_RATE,
        lat_int,
        lon_int,
        alt,
        vx,
        vy,
        vz,
        afx,
        afy,
        afz,
        yaw,
        yaw_rate);
    return _parent->send_message(message) ? Offboard::Result::Success :
                                            Offboard::Result::ConnectionError;
}

Offboard::Result OffboardImpl::send_velocity_global()
{
    const static uint16_t IGNORE_X = (1 << 0);
    const static uint16_t IGNORE_Y = (1 << 1);
    const static uint16_t IGNORE_Z = (1 << 2);
    // const static uint16_t IGNORE_VX = (1 << 3);
    // const static uint16_t IGNORE_VY = (1 << 4);
    // const static uint16_t IGNORE_VZ = (1 << 5);
    const static uint16_t IGNORE_AX = (1 << 6);
    const static uint16_t IGNORE_AY = (1 << 7);
    const static uint16_t IGNORE_AZ = (1 << 8);
    // const static uint16_t IS_FORCE = (1 << 9);
    // const static uint16_t IGNORE_YAW = (1 << 10);
    const static uint16_t IGNORE_YAW_RATE = (1 << 11);

    _mutex.lock();
    const float yaw = to_rad_from_deg(_velocity_global_yaw.yaw_deg);
    const float yaw_rate = 0.0f;
    const float lat_int = 0.0f;
    const float lon_int = 0.0f;
    const float alt = 0.0f;
    const float vx = _velocity_global_yaw.north_m_s;
    const float vy = _velocity_global_yaw.east_m_s;
    const float vz = _velocity_global_yaw.down_m_s;
    const float afx = 0.0f;
    const float afy = 0.0f;
    const float afz = 0.0f;
    _mutex.unlock();

    mavlink_message_t message;
    mavlink_msg_set_position_target_global_int_pack(
        _parent->get_own_system_id(),
        _parent->get_own_component_id(),
        &message,
        static_cast<uint32_t>(_parent->get_time().elapsed_s() * 1e3),
        _parent->get_system_id(),
        _parent->get_autopilot_id(),
        MAV_FRAME_GLOBAL_INT,
        IGNORE_X | IGNORE_Y | IGNORE_Z | IGNORE_AX | IGNORE_AY | IGNORE_AZ | IGNORE_YAW_RATE,
        lat_int,
        lon_int,
        alt,
        vx,
        vy,
        vz,
        afx,
        afy,
        afz,
        yaw,
        yaw_rate);
    return _parent->send_message(message) ? Offboard::Result::Success :
                                            Offboard::Result::ConnectionError;
}

 Offboard::Result OffboardImpl::send_position_ned()
{
    // const static uint16_t IGNORE_X = (1 << 0);
    // const static uint16_t IGNORE_Y = (1 << 1);
    // const static uint16_t IGNORE_Z = (1 << 2);
    const static uint16_t IGNORE_VX = (1 << 3);
    const static uint16_t IGNORE_VY = (1 << 4);
    const static uint16_t IGNORE_VZ = (1 << 5);
    const static uint16_t IGNORE_AX = (1 << 6);
    const static uint16_t IGNORE_AY = (1 << 7);
    const static uint16_t IGNORE_AZ = (1 << 8);
    // const static uint16_t IS_FORCE = (1 << 9);
    // const static uint16_t IGNORE_YAW = (1 << 10);
    const static uint16_t IGNORE_YAW_RATE = (1 << 11);

    _mutex.lock();
    const float yaw = to_rad_from_deg(_position_ned_yaw.yaw_deg);
    const float yaw_rate = 0.0f;
    const float x = _position_ned_yaw.north_m;
    const float y = _position_ned_yaw.east_m;
    const float z = _position_ned_yaw.down_m;
    const float vx = 0.0f;
    const float vy = 0.0f;
    const float vz = 0.0f;
    const float afx = 0.0f;
    const float afy = 0.0f;
    const float afz = 0.0f;
    _mutex.unlock();

    mavlink_message_t message;
    mavlink_msg_set_position_target_local_ned_pack(
        _parent->get_own_system_id(),
        _parent->get_own_component_id(),
        &message,
        static_cast<uint32_t>(_parent->get_time().elapsed_s() * 1e3),
        _parent->get_system_id(),
        _parent->get_autopilot_id(),
        MAV_FRAME_LOCAL_NED,
        IGNORE_VX | IGNORE_VY | IGNORE_VZ | IGNORE_AX | IGNORE_AY | IGNORE_AZ | IGNORE_YAW_RATE,
        x,
        y,
        z,
        vx,
        vy,
        vz,
        afx,
        afy,
        afz,
        yaw,
        yaw_rate);
    return _parent->send_message(message) ? Offboard::Result::Success :
                                            Offboard::Result::ConnectionError;
}

Offboard::Result OffboardImpl::send_velocity_ned()
{
    const static uint16_t IGNORE_X = (1 << 0);
    const static uint16_t IGNORE_Y = (1 << 1);
    const static uint16_t IGNORE_Z = (1 << 2);
    // const static uint16_t IGNORE_VX = (1 << 3);
    // const static uint16_t IGNORE_VY = (1 << 4);
    // const static uint16_t IGNORE_VZ = (1 << 5);
    const static uint16_t IGNORE_AX = (1 << 6);
    const static uint16_t IGNORE_AY = (1 << 7);
    const static uint16_t IGNORE_AZ = (1 << 8);
    // const static uint16_t IS_FORCE = (1 << 9);
    // const static uint16_t IGNORE_YAW = (1 << 10);
    const static uint16_t IGNORE_YAW_RATE = (1 << 11);

    _mutex.lock();
    const float yaw = to_rad_from_deg(_velocity_ned_yaw.yaw_deg);
    const float yaw_rate = 0.0f;
    const float x = 0.0f;
    const float y = 0.0f;
    const float z = 0.0f;
    const float vx = _velocity_ned_yaw.north_m_s;
    const float vy = _velocity_ned_yaw.east_m_s;
    const float vz = _velocity_ned_yaw.down_m_s;
    const float afx = 0.0f;
    const float afy = 0.0f;
    const float afz = 0.0f;
    _mutex.unlock();

    mavlink_message_t message;
    mavlink_msg_set_position_target_local_ned_pack(
        _parent->get_own_system_id(),
        _parent->get_own_component_id(),
        &message,
        static_cast<uint32_t>(_parent->get_time().elapsed_s() * 1e3),
        _parent->get_system_id(),
        _parent->get_autopilot_id(),
        MAV_FRAME_LOCAL_NED,
        IGNORE_X | IGNORE_Y | IGNORE_Z | IGNORE_AX | IGNORE_AY | IGNORE_AZ | IGNORE_YAW_RATE,
        x,
        y,
        z,
        vx,
        vy,
        vz,
        afx,
        afy,
        afz,
        yaw,
        yaw_rate);
    return _parent->send_message(message) ? Offboard::Result::Success :
                                            Offboard::Result::ConnectionError;
}

Offboard::Result OffboardImpl::send_velocity_body()
{
    const static uint16_t IGNORE_X = (1 << 0);
    const static uint16_t IGNORE_Y = (1 << 1);
    const static uint16_t IGNORE_Z = (1 << 2);
    // const static uint16_t IGNORE_VX = (1 << 3);
    // const static uint16_t IGNORE_VY = (1 << 4);
    // const static uint16_t IGNORE_VZ = (1 << 5);
    const static uint16_t IGNORE_AX = (1 << 6);
    const static uint16_t IGNORE_AY = (1 << 7);
    const static uint16_t IGNORE_AZ = (1 << 8);
    // const static uint16_t IS_FORCE = (1 << 9);
    const static uint16_t IGNORE_YAW = (1 << 10);
    // const static uint16_t IGNORE_YAW_RATE = (1 << 11);

    _mutex.lock();
    const float yaw = 0.0f;
    const float yaw_rate = to_rad_from_deg(_velocity_body_yawspeed.yawspeed_deg_s);
    const float x = 0.0f;
    const float y = 0.0f;
    const float z = 0.0f;
    const float vx = _velocity_body_yawspeed.forward_m_s;
    const float vy = _velocity_body_yawspeed.right_m_s;
    const float vz = _velocity_body_yawspeed.down_m_s;
    const float afx = 0.0f;
    const float afy = 0.0f;
    const float afz = 0.0f;
    _mutex.unlock();

    mavlink_message_t message;
    mavlink_msg_set_position_target_local_ned_pack(
        _parent->get_own_system_id(),
        _parent->get_own_component_id(),
        &message,
        static_cast<uint32_t>(_parent->get_time().elapsed_s() * 1e3),
        _parent->get_system_id(),
        _parent->get_autopilot_id(),
        MAV_FRAME_BODY_NED,
        IGNORE_X | IGNORE_Y | IGNORE_Z | IGNORE_AX | IGNORE_AY | IGNORE_AZ | IGNORE_YAW,
        x,
        y,
        z,
        vx,
        vy,
        vz,
        afx,
        afy,
        afz,
        yaw,
        yaw_rate);
    return _parent->send_message(message) ? Offboard::Result::Success :
                                            Offboard::Result::ConnectionError;
}

Offboard::Result OffboardImpl::send_attitude()
{
    const static uint8_t IGNORE_BODY_ROLL_RATE = (1 << 0);
    const static uint8_t IGNORE_BODY_PITCH_RATE = (1 << 1);
    const static uint8_t IGNORE_BODY_YAW_RATE = (1 << 2);
    // const static uint8_t IGNORE_4 = (1 << 3);
    // const static uint8_t IGNORE_5 = (1 << 4);
    // const static uint8_t IGNORE_6 = (1 << 5);
    // const static uint8_t IGNORE_THRUST = (1 << 6);
    // const static uint8_t IGNORE_ATTITUDE = (1 << 7);

    _mutex.lock();
    const float thrust = _attitude.thrust_value;
    const float roll = to_rad_from_deg(_attitude.roll_deg);
    const float pitch = to_rad_from_deg(_attitude.pitch_deg);
    const float yaw = to_rad_from_deg(_attitude.yaw_deg);
    _mutex.unlock();

    const double cos_phi_2 = cos(double(roll) / 2.0);
    const double sin_phi_2 = sin(double(roll) / 2.0);
    const double cos_theta_2 = cos(double(pitch) / 2.0);
    const double sin_theta_2 = sin(double(pitch) / 2.0);
    const double cos_psi_2 = cos(double(yaw) / 2.0);
    const double sin_psi_2 = sin(double(yaw) / 2.0);

    float q[4];

    q[0] = float(cos_phi_2 * cos_theta_2 * cos_psi_2 + sin_phi_2 * sin_theta_2 * sin_psi_2);
    q[1] = float(sin_phi_2 * cos_theta_2 * cos_psi_2 - cos_phi_2 * sin_theta_2 * sin_psi_2);
    q[2] = float(cos_phi_2 * sin_theta_2 * cos_psi_2 + sin_phi_2 * cos_theta_2 * sin_psi_2);
    q[3] = float(cos_phi_2 * cos_theta_2 * sin_psi_2 - sin_phi_2 * sin_theta_2 * cos_psi_2);

    mavlink_message_t message;
    mavlink_msg_set_attitude_target_pack(
        _parent->get_own_system_id(),
        _parent->get_own_component_id(),
        &message,
        static_cast<uint32_t>(_parent->get_time().elapsed_s() * 1e3),
        _parent->get_system_id(),
        _parent->get_autopilot_id(),
        IGNORE_BODY_ROLL_RATE | IGNORE_BODY_PITCH_RATE | IGNORE_BODY_YAW_RATE,
        q,
        0,
        0,
        0,
        thrust);
    return _parent->send_message(message) ? Offboard::Result::Success :
                                            Offboard::Result::ConnectionError;
}

Offboard::Result OffboardImpl::send_attitude_rate()
{
    // const static uint8_t IGNORE_BODY_ROLL_RATE = (1 << 0);
    // const static uint8_t IGNORE_BODY_PITCH_RATE = (1 << 1);
    // const static uint8_t IGNORE_BODY_YAW_RATE = (1 << 2);
    // const static uint8_t IGNORE_4 = (1 << 3);
    // const static uint8_t IGNORE_5 = (1 << 4);
    // const static uint8_t IGNORE_6 = (1 << 5);
    // const static uint8_t IGNORE_THRUST = (1 << 6);
    const static uint8_t IGNORE_ATTITUDE = (1 << 7);

    _mutex.lock();
    const float thrust = _attitude_rate.thrust_value;
    const float body_roll_rate = to_rad_from_deg(_attitude_rate.roll_deg_s);
    const float body_pitch_rate = to_rad_from_deg(_attitude_rate.pitch_deg_s);
    const float body_yaw_rate = to_rad_from_deg(_attitude_rate.yaw_deg_s);
    _mutex.unlock();

    mavlink_message_t message;
    mavlink_msg_set_attitude_target_pack(
        _parent->get_own_system_id(),
        _parent->get_own_component_id(),
        &message,
        static_cast<uint32_t>(_parent->get_time().elapsed_s() * 1e3),
        _parent->get_system_id(),
        _parent->get_autopilot_id(),
        IGNORE_ATTITUDE,
        0,
        body_roll_rate,
        body_pitch_rate,
        body_yaw_rate,
        thrust);
    return _parent->send_message(message) ? Offboard::Result::Success :
                                            Offboard::Result::ConnectionError;
}

Offboard::Result
OffboardImpl::send_actuator_control_message(const float* controls, uint8_t group_number)
{
    mavlink_message_t message;
    mavlink_msg_set_actuator_control_target_pack(
        _parent->get_own_system_id(),
        _parent->get_own_component_id(),
        &message,
        static_cast<uint32_t>(_parent->get_time().elapsed_s() * 1e3),
        group_number,
        _parent->get_system_id(),
        _parent->get_autopilot_id(),
        controls);
    return _parent->send_message(message) ? Offboard::Result::Success :
                                            Offboard::Result::ConnectionError;
}

Offboard::Result OffboardImpl::send_actuator_control()
{
    _mutex.lock();
    Offboard::ActuatorControl actuator_control = _actuator_control;
    _mutex.unlock();

    for (int i = 0; i < 2; i++) {
        int nan_count = 0;
        for (int j = 0; j < 8; j++) {
            if (std::isnan(actuator_control.groups[i].controls[j])) {
                nan_count++;
                actuator_control.groups[i].controls[j] = 0.0f;
            }
        }
        if (nan_count < 8) {
            auto result = send_actuator_control_message(&actuator_control.groups[i].controls[0], i);
            if (result != Offboard::Result::Success) {
                return result;
            }
        }
    }
    return Offboard::Result::Success;
}

void OffboardImpl::process_heartbeat(const mavlink_message_t& message)
{
    // Process only Heartbeat coming from the autopilot
    if (message.compid != MAV_COMP_ID_AUTOPILOT1) {
        return;
    }

    mavlink_heartbeat_t heartbeat;
    mavlink_msg_heartbeat_decode(&message, &heartbeat);

    bool offboard_mode_active = false;
    if (heartbeat.base_mode & MAV_MODE_FLAG_CUSTOM_MODE_ENABLED) {
        px4::px4_custom_mode px4_custom_mode;
        px4_custom_mode.data = heartbeat.custom_mode;

        if (px4_custom_mode.main_mode == px4::PX4_CUSTOM_MAIN_MODE_OFFBOARD) {
            offboard_mode_active = true;
        }
    }

    {
        // Right after we started offboard we might still be getting heartbeats
        // from earlier which don't indicate offboard mode yet.
        // Therefore, we make sure we don't stop too eagerly and ignore
        // possibly stale heartbeats for some time.
        std::lock_guard<std::mutex> lock(_mutex);
        if (!offboard_mode_active && _mode != Mode::NotActive &&
            _time.elapsed_since_s(_last_started) > 1.5) {
            // It seems that we are no longer in offboard mode but still trying to send
            // setpoints. Let's stop for now.
            stop_sending_setpoints();
        }
    }
}

void OffboardImpl::stop_sending_setpoints()
{
    // We assume that we already acquired the mutex in this function.

    if (_call_every_cookie != nullptr) {
        _parent->remove_call_every(_call_every_cookie);
        _call_every_cookie = nullptr;
    }
    _mode = Mode::NotActive;
}

Offboard::Result
OffboardImpl::offboard_result_from_command_result(MavlinkCommandSender::Result result)
{
    switch (result) {
        case MavlinkCommandSender::Result::Success:
            return Offboard::Result::Success;
        case MavlinkCommandSender::Result::NoSystem:
            return Offboard::Result::NoSystem;
        case MavlinkCommandSender::Result::ConnectionError:
            return Offboard::Result::ConnectionError;
        case MavlinkCommandSender::Result::Busy:
            return Offboard::Result::Busy;
        case MavlinkCommandSender::Result::CommandDenied:
            return Offboard::Result::CommandDenied;
        case MavlinkCommandSender::Result::Timeout:
            return Offboard::Result::Timeout;
        default:
            return Offboard::Result::Unknown;
    }
}

} // namespace mavsdk
