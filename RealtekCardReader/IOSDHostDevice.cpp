//
//  IOSDHostDevice.cpp
//  RealtekCardReader
//
//  Created by FireWolf on 6/5/21.
//

#include "IOSDHostDevice.hpp"
#include "IOSDHostDriver.hpp"

//
// MARK: - Meta Class Definitions
//

OSDefineMetaClassAndAbstractStructors(IOSDHostDevice, IOService);

//
// MARK: - SD Request Processors
//

///
/// Preprocess the given SD command request
///
/// @param request A SD command request
/// @return `kIOReturnSuccess` on success, other values otherwise.
/// @note Port: This function replaces `sdmmc_pre_req()` defined in `rtsx_pci_sdmmc.c`.
///
IOReturn IOSDHostDevice::preprocessRequest(RealtekSDRequest& request)
{
    return kIOReturnSuccess;
}

///
/// Postprocess the given SD command request
///
/// @param request A SD command request
/// @return `kIOReturnSuccess` on success, other values otherwise.
/// @note Port: This function replaces `sdmmc_post_req()` defined in `rtsx_pci_sdmmc.c`.
///
IOReturn IOSDHostDevice::postprocessRequest(RealtekSDRequest& request)
{
    return kIOReturnSuccess;
}

//
// MARK: - Card Events Callbacks
//

///
/// [UPCALL] Notify the host device when a SD card is inserted
///
/// @note This callback function runs in a gated context provided by the underlying card reader controller.
///       The host device should implement this function without any blocking operations.
///       A default implementation that notifies the host driver is provided.
///
void IOSDHostDevice::onSDCardInsertedGated()
{
    this->driver->onSDCardInsertedGated();
    
    this->setProperty("Card Present", true);
}

///
/// [UPCALL] Notify the host device when a SD card is removed
///
/// @note This callback function runs in a gated context provided by the underlying card reader controller.
///       The host device should implement this function without any blocking operations.
///       A default implementation that notifies the host driver is provided.
///
void IOSDHostDevice::onSDCardRemovedGated()
{
    this->driver->onSDCardRemovedGated();
    
    this->setProperty("Card Present", false);
}

//
// MARK: - Power Management
//

///
/// Adjust the power state in response to system-wide power events
///
/// @param powerStateOrdinal The number in the power state array of the state the driver is being instructed to switch to
/// @param whatDevice A pointer to the power management object which registered to manage power for this device
/// @return `kIOPMAckImplied` always.
///
IOReturn IOSDHostDevice::setPowerState(unsigned long powerStateOrdinal, IOService* whatDevice)
{
    return kIOPMAckImplied;
}

//
// MARK: - IOService Implementation
//

///
/// Initialize the host device
///
/// @param dictionary A nullable matching dictionary
/// @return `true` on success, `false` otherwise.
///
bool IOSDHostDevice::init(OSDictionary* dictionary)
{
    if (!super::init(dictionary))
    {
        return false;
    }
    
    this->setProperty("Card Present", false);
    
    return true;
}

///
/// Start the host device
///
/// @param provider An instance of card reader controller
/// @return `true` on success, `false` otherwise.
///
bool IOSDHostDevice::start(IOService* provider)
{
    pinfo("Starting the host device...");
    
    if (!super::start(provider))
    {
        return false;
    }
    
    pinfo("Setting up the power management...");
    
    this->PMinit();
    
    provider->joinPMtree(this);
    
    static IOPMPowerState kPowerStates[] =
    {
        { kIOPMPowerStateVersion1, kIOPMPowerOff, kIOPMPowerOff, kIOPMPowerOff, 0, 0, 0, 0, 0, 0, 0, 0 },
        { kIOPMPowerStateVersion1, kIOPMPowerOn | kIOPMDeviceUsable | kIOPMInitialDeviceState, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0 },
    };
    
    return this->registerPowerDriver(this, kPowerStates, arrsize(kPowerStates)) == kIOPMNoErr;
}

///
/// Stop the host device
///
/// @param provider An instance of card reader controller
///
void IOSDHostDevice::stop(IOService* provider)
{
    this->PMstop();
    
    super::stop(provider);
}