// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#include "Pomdog/Network/TCPStream.hpp"
#if defined(POMDOG_PLATFORM_MACOSX) \
    || defined(POMDOG_PLATFORM_APPLE_IOS) \
    || defined(POMDOG_PLATFORM_LINUX)
#include "../Network.POSIX/TCPStreamPOSIX.hpp"
#elif defined(POMDOG_PLATFORM_WIN32) || defined(POMDOG_PLATFORM_XBOX_ONE)
#include "../Network.Win32/TCPStreamWin32.hpp"
#endif
#include "AddressParser.hpp"
#include "Pomdog/Utility/Assert.hpp"
#include <utility>

namespace Pomdog {

TCPStream::TCPStream() = default;

TCPStream::TCPStream(IOService* service)
    : nativeStream(std::make_unique<Detail::NativeTCPStream>(service))
{
}

TCPStream::~TCPStream()
{
}

TCPStream::TCPStream(TCPStream&& other) = default;
TCPStream& TCPStream::operator=(TCPStream&& other) = default;

std::tuple<TCPStream, std::shared_ptr<Error>>
TCPStream::Connect(IOService* service, const std::string_view& address)
{
    POMDOG_ASSERT(service != nullptr);

    TCPStream stream{service};
    POMDOG_ASSERT(stream.nativeStream != nullptr);

    const auto [family, host, port] = Detail::AddressParser::TransformAddress(address);

    if (auto err = stream.nativeStream->Connect(host, port, std::chrono::seconds{5}); err != nullptr) {
        return std::make_tuple(std::move(stream), std::move(err));
    }
    return std::make_tuple(std::move(stream), nullptr);
}

std::tuple<TCPStream, std::shared_ptr<Error>>
TCPStream::Connect(IOService* service, const std::string_view& address, const Duration& timeout)
{
    POMDOG_ASSERT(service != nullptr);

    TCPStream stream{service};
    POMDOG_ASSERT(stream.nativeStream != nullptr);

    const auto [family, host, port] = Detail::AddressParser::TransformAddress(address);

    if (auto err = stream.nativeStream->Connect(host, port, timeout); err != nullptr) {
        return std::make_tuple(std::move(stream), std::move(err));
    }
    return std::make_tuple(std::move(stream), nullptr);
}

void TCPStream::Disconnect()
{
    POMDOG_ASSERT(nativeStream != nullptr);
    nativeStream->Close();
}

Connection TCPStream::OnConnected(std::function<void(const std::shared_ptr<Error>&)>&& callback)
{
    POMDOG_ASSERT(nativeStream != nullptr);
    return nativeStream->OnConnected.Connect(std::move(callback));
}

Connection TCPStream::OnDisconnect(std::function<void()>&& callback)
{
    POMDOG_ASSERT(nativeStream != nullptr);
    return nativeStream->OnDisconnect.Connect(std::move(callback));
}

Connection TCPStream::OnRead(std::function<void(const ArrayView<std::uint8_t>&, const std::shared_ptr<Error>&)>&& callback)
{
    POMDOG_ASSERT(nativeStream != nullptr);
    return nativeStream->OnRead.Connect(std::move(callback));
}

std::shared_ptr<Error> TCPStream::Write(const ArrayView<std::uint8_t const>& data)
{
    POMDOG_ASSERT(nativeStream != nullptr);
    return nativeStream->Write(data);
}

bool TCPStream::IsConnected() const noexcept
{
    if (nativeStream == nullptr) {
        return false;
    }
    return nativeStream->IsConnected();
}

void TCPStream::SetTimeout(const Duration& timeout)
{
    POMDOG_ASSERT(nativeStream != nullptr);
    nativeStream->SetTimeout(timeout);
}

} // namespace Pomdog
