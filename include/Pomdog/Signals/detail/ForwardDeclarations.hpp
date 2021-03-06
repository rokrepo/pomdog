// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#pragma once

#include <functional>

namespace Pomdog {
namespace Detail::Signals {

template <typename Function>
using Slot = std::function<Function>;

template <typename Function>
class SignalBody;

template <typename Function>
class DelegateBody;

} // namespace Detail::Signals

class Event;
class EventQueue;

class Connection;
class ScopedConnection;

class ConnectionList;

template <typename Function>
class Signal;

template <typename Function>
class Delegate;

} // namespace Pomdog
