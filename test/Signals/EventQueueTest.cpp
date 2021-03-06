// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#include "Pomdog/Signals/Connection.hpp"
#include "Pomdog/Signals/Event.hpp"
#include "Pomdog/Signals/EventQueue.hpp"
#include "catch.hpp"
#include <utility>

using Pomdog::Event;
using Pomdog::EventQueue;
using Pomdog::Connection;

TEST_CASE("EventQueue", "[EventQueue]")
{
    std::vector<int> integers;
    std::vector<std::string> names;
    std::function<void(Event const&)> slot;

    integers.clear();
    names.clear();

    slot = [&](Event const& event) {
        REQUIRE(event.Is<int>());
        integers.push_back(*event.As<int>());
    };

    SECTION("Invoke with int")
    {
        EventQueue eventQueue;
        auto connection = eventQueue.Connect(slot);

        eventQueue.Enqueue<int>(42);
        eventQueue.Enqueue<int>(43);
        eventQueue.Enqueue<int>(44);
        REQUIRE(integers.empty());

        eventQueue.Emit();
        REQUIRE(integers.size() == 3);
        REQUIRE(integers[0] == 42);
        REQUIRE(integers[1] == 43);
        REQUIRE(integers[2] == 44);
    }
    SECTION("Invoke with POD struct")
    {
        struct User
        {
            std::string name;
            int id;
        };

        EventQueue eventQueue;
        auto conn = eventQueue.Connect([&](Event const& event){
            REQUIRE(event.Is<User>());
            auto user = event.As<User>();
            names.push_back(user->name);
            integers.push_back(user->id);
        });
        REQUIRE(conn.IsConnected());

        eventQueue.Enqueue<User>("Donald", 42);
        eventQueue.Enqueue<User>("Goofy", 43);
        REQUIRE(names.empty());
        REQUIRE(integers.empty());

        eventQueue.Emit();
        REQUIRE(names.size() == 2);
        REQUIRE(names[0] == "Donald");
        REQUIRE(names[1] == "Goofy");
        REQUIRE(integers.size() == 2);
        REQUIRE(integers[0] == 42);
        REQUIRE(integers[1] == 43);
    }
    SECTION("Disconnect")
    {
        EventQueue eventQueue;
        auto connection = eventQueue.Connect(slot);

        eventQueue.Enqueue<int>(42);
        REQUIRE(integers.empty());

        connection.Disconnect();
        eventQueue.Emit();
        REQUIRE(integers.empty());
    }
    SECTION("Conenct")
    {
        EventQueue eventQueue;
        auto conn1 = eventQueue.Connect(slot);
        auto conn2 = eventQueue.Connect(slot);
        auto conn3 = eventQueue.Connect(slot);

        REQUIRE(conn1.IsConnected());
        REQUIRE(conn2.IsConnected());
        REQUIRE(conn3.IsConnected());

        eventQueue.Enqueue<int>(42);
        REQUIRE(integers.empty());

        eventQueue.Emit();
        REQUIRE(integers.size() == 3);
        REQUIRE(integers[0] == 42);
        REQUIRE(integers[1] == 42);
        REQUIRE(integers[2] == 42);
    }
    SECTION("RecursiveConnection")
    {
        EventQueue eventQueue;
        auto conn = eventQueue.Connect([&](Event const&) {
            auto conn2 = eventQueue.Connect(slot);
            REQUIRE(conn2.IsConnected());
        });
        REQUIRE(conn.IsConnected());

        eventQueue.Enqueue<int>(42);
        REQUIRE(integers.empty());
        eventQueue.Emit();
        REQUIRE(integers.empty());

        eventQueue.Enqueue<int>(43);
        REQUIRE(integers.empty());

        eventQueue.Emit();
        REQUIRE(integers.size() == 1);
        REQUIRE(integers[0] == 43);
    }
    SECTION("CallingDisconnect")
    {
        EventQueue eventQueue;
        Connection conn;
        REQUIRE(!conn.IsConnected());
        conn = eventQueue.Connect([&](Event const& event){
            slot(event);
            REQUIRE(conn.IsConnected());
            conn.Disconnect();
            REQUIRE(!conn.IsConnected());
        });
        REQUIRE(conn.IsConnected());

        eventQueue.Enqueue<int>(42);
        REQUIRE(conn.IsConnected());
        REQUIRE(integers.empty());
        eventQueue.Emit();
        REQUIRE(!conn.IsConnected());
        REQUIRE(integers.size() == 1);
        REQUIRE(integers[0] == 42);

        integers.clear();
        eventQueue.Enqueue<int>(43);
        REQUIRE(integers.empty());
        eventQueue.Emit();
        REQUIRE(integers.empty());
    }
    SECTION("ArgumentPerfectForwarding")
    {
        EventQueue eventQueue;
        auto conn = eventQueue.Connect([&](Event const& event){
            REQUIRE(event.Is<std::shared_ptr<int>>());
        });
        REQUIRE(conn.IsConnected());

        {
            auto pointer = std::make_shared<int>(42);
            REQUIRE(pointer);
            eventQueue.Enqueue<std::shared_ptr<int>>(pointer);
            REQUIRE(pointer);
        }
        {
            auto pointer = std::make_shared<int>(42);
            REQUIRE(pointer);
            eventQueue.Enqueue<std::shared_ptr<int>>(std::move(pointer));
            REQUIRE_FALSE(pointer);
        }
        {
            auto const pointer = std::make_shared<int>(42);
            REQUIRE(pointer);
            eventQueue.Enqueue<std::shared_ptr<int>>(pointer);
            REQUIRE(pointer);
        }
    }
}
