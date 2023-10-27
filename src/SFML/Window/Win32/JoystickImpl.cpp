////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2023 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "concrt.h"
#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.Gaming.Input.h"

#include <SFML/Window/JoystickImpl.hpp>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Win32/WindowsHeader.hpp>

#include <algorithm>
#include <inspectable.h>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <regstr.h>
#include <sstream>
#include <string>
#include <tchar.h>
#include <vector>

using namespace winrt;
using namespace Windows::Gaming::Input;
using namespace Windows::Foundation::Collections;

namespace
{

const RawGameController** rawGameControllers;
const Gamepad**           gamepads;
winrt::hstring**           controllerStrings;

concurrency::critical_section joystickListLock{};
event_token                   rawControllerAddedToken;
event_token                   rawControllerRemovedToken;
event_token                   gamepadAddedToken;
event_token                   gamepadRemovedToken;
unsigned int                  globalJoystickCount;

} // namespace

namespace sf::priv
{

JoystickImpl::JoystickImpl()
{
    concurrency::critical_section::scoped_lock lock{joystickListLock};

    m_index = globalJoystickCount;
    globalJoystickCount++;
}

////////////////////////////////////////////////////////////
void JoystickImpl::initialize()
{
    // TODO: Initialize
    // lock the collection, avoid race conditions
    concurrency::critical_section::scoped_lock lock{joystickListLock};

    rawGameControllers = new const RawGameController*[Joystick::Count];
    gamepads           = new const Gamepad*[Joystick::Count];
    controllerStrings  = new winrt::hstring*[Joystick::Count];

    ZeroMemory(rawGameControllers, Joystick::Count * sizeof(RawGameController*));
    ZeroMemory(gamepads, Joystick::Count * sizeof(Gamepad*));
    ZeroMemory(controllerStrings, Joystick::Count * sizeof(winrt::hstring*));

    gamepadAddedToken = Gamepad::GamepadAdded(GamepadAdded);
    gamepadRemovedToken       = Gamepad::GamepadRemoved(GamepadRemoved);
    rawControllerAddedToken   = RawGameController::RawGameControllerAdded(RawControllerAdded);
    rawControllerRemovedToken = RawGameController::RawGameControllerRemoved(RawControllerRemoved);
}

////////////////////////////////////////////////////////////
void JoystickImpl::cleanup()
{
    // TODO: Clean up
}

JoystickState JoystickImpl::update()
{
    concurrency::critical_section::scoped_lock lock{joystickListLock};

    auto gamepad = gamepads[m_index];

    if (gamepad != nullptr)
    {
        SetStateFromGamepad(*gamepad);
    }
    else
    {
        auto rawCon = rawGameControllers[m_index];
        if (rawCon != nullptr)
        {
            SetStateFromRawController(*rawCon);
        }
    }

    return m_state;
}

void JoystickImpl::SetStateFromGamepad(const Gamepad& gamepad)
{
    std::cout << "Gamepad State Set\n\r";

    auto state = gamepad.GetCurrentReading();

    std::cout << "left trigger: " << state.LeftTrigger << " right trigger: " << state.RightTrigger << "\n\r";

    m_state = JoystickState();
}

void JoystickImpl::SetStateFromRawController(const RawGameController& controller)
{
    std::cout << "Raw Controller State Set\n\r";

    auto   buttons = winrt::array_view<bool>(m_state.buttons, &m_state.buttons[31]);
    double axesArray[8];
    auto   axes = winrt::array_view<double>(axesArray, &axesArray[7]);

    auto switchCount = controller.SwitchCount();

    std::vector<GameControllerSwitchPosition> switchPoss(switchCount);
    auto switches = winrt::array_view<GameControllerSwitchPosition>(switchPoss.data(), &switchPoss[switchCount - 1]);

    controller.GetCurrentReading(buttons, switches, axes);

    

    m_state = JoystickState();
}


////////////////////////////////////////////////////////////
bool JoystickImpl::isConnected(unsigned int index)
{
    concurrency::critical_section::scoped_lock lock{joystickListLock};

    auto gamepad = gamepads[index];

    if (gamepad == nullptr)
    {
        auto rawCon = rawGameControllers[index];

        if (rawCon == nullptr)
        {
            return false;
        }
        return true;
    }
    else
    {
        return true;
    }

    return false;
}

JoystickState JoystickImpl::getState()
{
    return m_state;
}

JoystickCaps JoystickImpl::getCapabilities()
{
    return m_caps;
}

void JoystickImpl::updateConnections()
{
    // Do nothing
}

void JoystickImpl::setLazyUpdates(bool lazy)
{
    // stub?

    if (lazy)
    {
        // todo: be lazy
    }
    else
    {
        // todo: don't be lazy
    }
}

bool JoystickImpl::open(int index)
{
    m_index = index;
    return true;
}

void JoystickImpl::close()
{
    // stub
}

sf::Joystick::Identification JoystickImpl::getIdentification()
{
    return sf::Joystick::Identification();
}

void JoystickImpl::RawControllerAdded(const winrt::Windows::Foundation::IInspectable, const RawGameController& controller)
{
    concurrency::critical_section::scoped_lock lock{joystickListLock};

    for (int i = 0; i < sf::Joystick::Count; i++)
    {
        auto t_con = rawGameControllers[i];
        if (t_con == nullptr)
        {
            auto identity = controller.NonRoamableId();

            rawGameControllers[i] = &controller;
            break;
        }
        else
        {
            continue;
        }
    }

    std::cout << "Controller added\n\r";
}

void JoystickImpl::RawControllerRemoved(const winrt::Windows::Foundation::IInspectable, const RawGameController& controller)
{
    concurrency::critical_section::scoped_lock lock{joystickListLock};

    for (int i = 0; i < sf::Joystick::Count; i++)
    {
        auto t_con = rawGameControllers[i];
        if (t_con == nullptr)
        {
            continue;
        }

        const RawGameController& t_con_ref = *t_con;

        if (t_con_ref == controller)
        {
            rawGameControllers[i] = nullptr;
        }
    }

    std::cout << "Controller removed\n\r";
}

void JoystickImpl::GamepadAdded(const winrt::Windows::Foundation::IInspectable, const Gamepad& controller)
{
    concurrency::critical_section::scoped_lock lock{joystickListLock};

    for (int i = 0; i < sf::Joystick::Count; i++)
    {
        auto* t_con = gamepads[i];
        if (t_con == nullptr)
        {
            gamepads[i] = &controller;

            break;
        }
    }

    std::cout << "Gamepad added " << &controller << "\n\r";
}
void JoystickImpl::GamepadRemoved(const winrt::Windows::Foundation::IInspectable, const Gamepad& controller)
{
    concurrency::critical_section::scoped_lock lock{joystickListLock};

    for (int i = 0; i < sf::Joystick::Count; i++)
    {
        auto* t_con = gamepads[i];
        if (t_con == nullptr)
        {
            continue;
        }

        auto& t_con_ref = *t_con;

        if (t_con_ref == controller)
        {
            gamepads[i] = nullptr;
        }
    }

    std::cout << "Gamepad removed " << &controller << "\n\r";
}

} // namespace sf::priv
