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

std::vector<RawGameController> controllers;

concurrency::critical_section joystickListLock{};
event_token                   controllerAddedToken;
event_token                   controllerRemovedToken;
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
    controllerAddedToken   = RawGameController::RawGameControllerAdded(ControllerAdded);
    controllerRemovedToken = RawGameController::RawGameControllerRemoved(ControllerRemoved);
}

////////////////////////////////////////////////////////////
void JoystickImpl::cleanup()
{
    // TODO: Clean up
}

JoystickState JoystickImpl::update()
{
    if (m_index < controllers.size())
        return m_state;

    auto& controller = controllers.at(m_index);

    auto   buttons = winrt::array_view<bool>(m_state.buttons, &m_state.buttons[31]);
    double axesArray[8];
    auto   axes = winrt::array_view<double>(axesArray, &axesArray[7]);

    auto switchCount = controller.SwitchCount();

    std::vector<GameControllerSwitchPosition> switchPoss(switchCount);
    auto switches = winrt::array_view<GameControllerSwitchPosition>(switchPoss.data(), &switchPoss[switchCount - 1]);

    controller.GetCurrentReading(buttons, switches, axes);

    std::cout << "FUCK! ";

    return m_state;
}


////////////////////////////////////////////////////////////
bool JoystickImpl::isConnected(unsigned int index)
{
    if (index < controllers.size())
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

void JoystickImpl::ControllerAdded(const winrt::Windows::Foundation::IInspectable, const RawGameController& controller)
{
    concurrency::critical_section::scoped_lock lock{joystickListLock};

    controllers.push_back(controller);

    std::cout << "Controller added\n\r";
}

bool JoystickImpl::open(int index)
{
    if (index < controllers.size())
        return true;
    return false;
}

void JoystickImpl::close()
{
    // stub
}

sf::Joystick::Identification JoystickImpl::getIdentification()
{
    return sf::Joystick::Identification();
}

void JoystickImpl::ControllerRemoved(const winrt::Windows::Foundation::IInspectable, const RawGameController& controller)
{
    concurrency::critical_section::scoped_lock lock{joystickListLock};

    for (int i = 0; i < controllers.size(); i++)
    {
        RawGameController& t_con = controllers.at(i);
        if (t_con == controller)
        {
            controllers.erase(controllers.begin() + i);
        }
    }

    std::cout << "Controller removed\n\r";
}

} // namespace sf::priv
