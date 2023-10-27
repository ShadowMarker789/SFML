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

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/JoystickImpl.hpp>

#include <SFML/System/Win32/WindowsHeader.hpp>

#include <mmsystem.h>

#include "winrt/Windows.Gaming.Input.h"
#include <inspectable.h>

using namespace winrt;
using namespace Windows::Gaming::Input;

namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Windows implementation of joysticks
///
////////////////////////////////////////////////////////////
class JoystickImpl
{
public:

    JoystickImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Perform the global initialization of the joystick module
    ///
    ////////////////////////////////////////////////////////////
    static void initialize();

    ////////////////////////////////////////////////////////////
    /// \brief Perform the global cleanup of the joystick module
    ///
    ////////////////////////////////////////////////////////////
    static void cleanup();

    ////////////////////////////////////////////////////////////
    /// \brief Check if a joystick is currently connected
    ///
    /// \param index Index of the joystick to check
    ///
    /// \return True if the joystick is connected, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    static bool isConnected(unsigned int index);

    ////////////////////////////////////////////////////////////
    /// \brief Obtain the state of a joystick by index 
    ///
    ///
    /// \return The current state of the joystick 
    ///
    ////////////////////////////////////////////////////////////
    JoystickState getState();

    ////////////////////////////////////////////////////////////
    /// \brief Obtain the capabilities of a joystick by index
    ///
    /// \param index Index of the joystick to query capabilities of
    ///
    /// \return The capabilities of the joystick
    ///
    ////////////////////////////////////////////////////////////
    JoystickCaps getCapabilities();

    ////////////////////////////////////////////////////////////
    /// \brief Updates the connections to the devices where applicable
    ///
    ///
    ////////////////////////////////////////////////////////////
    static void updateConnections();

    ////////////////////////////////////////////////////////////
    /// \brief Sets the laziness of the Joystick implementation. 
    ///
    /// \param lazy True for lazy, False for not lazy. 
    ////////////////////////////////////////////////////////////
    static void setLazyUpdates(bool lazy);

    JoystickState update();

    void close();

    bool open(int index);

    sf::Joystick::Identification getIdentification();

private:

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////

    static void ControllerAdded(const winrt::Windows::Foundation::IInspectable, RawGameController const& controller);
    static void ControllerRemoved(const winrt::Windows::Foundation::IInspectable, RawGameController const& controller);

    unsigned int             m_index;
    JoystickState            m_state;
    JoystickCaps             m_caps;
    Joystick::Identification m_identification;
};

} // namespace sf::priv
