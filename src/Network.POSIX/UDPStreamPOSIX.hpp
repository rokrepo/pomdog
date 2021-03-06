// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#pragma once

#include "Pomdog/Application/Duration.hpp"
#include "Pomdog/Network/detail/ForwardDeclarations.hpp"
#include "Pomdog/Signals/Delegate.hpp"
#include "Pomdog/Signals/ScopedConnection.hpp"
#include "Pomdog/Utility/Errors.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>
#include <thread>

namespace Pomdog::Detail {

class UDPStreamPOSIX final {
public:
    UDPStreamPOSIX() = delete;

    explicit UDPStreamPOSIX(IOService* service);

    ~UDPStreamPOSIX();

    UDPStreamPOSIX(const UDPStreamPOSIX&) = delete;
    UDPStreamPOSIX& operator=(const UDPStreamPOSIX&) = delete;

    UDPStreamPOSIX(UDPStreamPOSIX&&) = delete;
    UDPStreamPOSIX& operator=(UDPStreamPOSIX&&) = delete;

    /// Opens a UDP connection over UDP to a remote host.
    [[nodiscard]] std::shared_ptr<Error>
    Connect(const std::string_view& host, const std::string_view& port, const Duration& timeout);

    /// Starts listening for incoming datagrams.
    [[nodiscard]] std::shared_ptr<Error>
    Listen(const std::string_view& host, const std::string_view& port);

    /// Closes the connection.
    void Close();

    /// Writes data to the connection.
    [[nodiscard]] std::shared_ptr<Error>
    Write(const ArrayView<std::uint8_t const>& data);

    /// Writes data to address.
    [[nodiscard]] std::shared_ptr<Error>
    WriteTo(const ArrayView<std::uint8_t const>& data, const std::string_view& address);

    /// Returns the native socket handle.
    [[nodiscard]] int GetHandle() const noexcept;

    /// Delegate that fires when a connection is successfully established.
    Delegate<void(const std::shared_ptr<Error>&)> OnConnected;

    /// Delegate that fires when a data packet is received.
    Delegate<void(const ArrayView<std::uint8_t>&, const std::shared_ptr<Error>&)> OnRead;

    /// Delegate that fires when a data packet is received from the connection.
    Delegate<void(const ArrayView<std::uint8_t>& view, const std::string_view& address, const std::shared_ptr<Error>&)> OnReadFrom;

private:
    void ReadEventLoop();

    void ReadFromEventLoop();

private:
    std::thread blockingThread;
    IOService* service = nullptr;
    ScopedConnection eventLoopConn;
    ScopedConnection errorConn;
    int descriptor = -1;
};

} // namespace Pomdog::Detail
