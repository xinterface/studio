/****************************************************************************
**
** Copyright 2019 neuromore co
** Contact: https://neuromore.com/contact
**
** Commercial License Usage
** Licensees holding valid commercial neuromore licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and neuromore. For licensing terms
** and conditions see https://neuromore.com/licensing. For further
** information use the contact form at https://neuromore.com/contact.
**
** neuromore Public License Usage
** Alternatively, this file may be used under the terms of the neuromore
** Public License version 1 as published by neuromore co with exceptions as 
** appearing in the file neuromore-class-exception.md included in the 
** packaging of this file. Please review the following information to 
** ensure the neuromore Public License requirements will be met: 
** https://neuromore.com/npl
**
****************************************************************************/

// include required files
#include "NotionDevices.h"
#include "../../EngineManager.h"
#include "../../Core/LogManager.h"

#ifdef INCLUDE_DEVICE_NEUROSITY_NOTION

#define BUFFERSIZE_IN_SECONDS 60.0

using namespace Core;

//constructor
NotionDevice::NotionDevice(DeviceDriver* driver) : BciDevice(driver), mSubType(SubType::NOTION2)
{
    LogDetailedInfo("Constructing Notion headset...");
    mOscPathPattern = "/neurosity/notion/*/*";
    CreateSensors();
}

// destructor
NotionDevice::~NotionDevice()
{
    LogDetailedInfo("Destructing Notion headset ...");
}

// get the available electrodes of the headset
void NotionDevice::CreateElectrodes()
{
    mElectrodes.Clear();
    mElectrodes.Reserve(8);
    mElectrodes.Add(GetEEGElectrodes()->GetElectrodeByID("CP5")); // 0
    mElectrodes.Add(GetEEGElectrodes()->GetElectrodeByID("F5"));  // 1
    mElectrodes.Add(GetEEGElectrodes()->GetElectrodeByID("C3"));  // 2
    mElectrodes.Add(GetEEGElectrodes()->GetElectrodeByID("CP3")); // 3
    mElectrodes.Add(GetEEGElectrodes()->GetElectrodeByID("CP6")); // 4
    mElectrodes.Add(GetEEGElectrodes()->GetElectrodeByID("F6"));  // 5
    mElectrodes.Add(GetEEGElectrodes()->GetElectrodeByID("C4"));  // 6
    mElectrodes.Add(GetEEGElectrodes()->GetElectrodeByID("CP4")); // 7
}

void NotionDevice::CreateSensors()
{
    // create EEG sensors first
   BciDevice::CreateSensors();

   const uint32 numSensors = GetNumSensors();
   for (uint32_t i = 0; i < numSensors; i++)
   {
      // extend buffers so they can hold up to 60s of samples
      GetSensor(i)->GetInput()->SetBufferSizeInSeconds(BUFFERSIZE_IN_SECONDS);
      GetSensor(i)->GetOutput()->SetBufferSizeInSeconds(BUFFERSIZE_IN_SECONDS);
   }
}

void NotionDevice::ProcessMessage(OscMessageParser* message)
{
    // do nothing is device was disabled or not enough sensors for code
    if (IsEnabled() == false || GetNumSensors() < 8)
        return;

    // raw 8 channel eeg
    if (message->MatchAddress("/neurosity/notion/*/raw") == true)
    {
        if (message->GetNumArguments() < 13)
            return;

        // ignore the array start (TODO: This is wrong type)
        char v0; (*message) >> v0;

        // parse the array values
        float v1; (*message) >> v1;
        float v2; (*message) >> v2;
        float v3; (*message) >> v3;
        float v4; (*message) >> v4;
        float v5; (*message) >> v5;
        float v6; (*message) >> v6;
        float v7; (*message) >> v7;
        float v8; (*message) >> v8;
        
        // ignore the array end (TODO: This is wrong type)
        //char v9; (*message) >> v9;

        // we don't use these threre
        //int v10; (*message) >> v10; //c-string
        //int v11; (*message) >> v11; //int32
        //int v12; (*message) >> v12; //c-string

        if (mSubType == NOTION2)
        {
           GetSensor(0)->AddQueuedSample(v1); // v1 = CP5
           GetSensor(1)->AddQueuedSample(v2); // v2 = F5
           GetSensor(2)->AddQueuedSample(v3); // v3 = C3
           GetSensor(3)->AddQueuedSample(v4); // v4 = CP3
           GetSensor(4)->AddQueuedSample(v5); // v5 = CP6
           GetSensor(5)->AddQueuedSample(v6); // v6 = F6
           GetSensor(6)->AddQueuedSample(v7); // v7 = C4
           GetSensor(7)->AddQueuedSample(v8); // v8 = CP4
        }
        else if (mSubType == NOTION1)
        {
           GetSensor(4)->AddQueuedSample(v1); // v1 = CP6
           GetSensor(5)->AddQueuedSample(v2); // v2 = F6
           GetSensor(6)->AddQueuedSample(v3); // v3 = C4
           GetSensor(7)->AddQueuedSample(v4); // v4 = CP4
           GetSensor(3)->AddQueuedSample(v5); // v5 = CP3
           GetSensor(1)->AddQueuedSample(v6); // v6 = F5
           GetSensor(2)->AddQueuedSample(v7); // v7 = C3
           GetSensor(0)->AddQueuedSample(v8); // v8 = CP5
        }
    }
    else if (message->MatchAddress("/neurosity/notion/*/info") == true)
    {
       // TODO: Read type (and may be otherrs, sample rate from it?)
    }
}

void NotionDevice::SetDeviceId(uint32 deviceId)
{
    mDeviceID = deviceId;
}

void NotionDevice::SetDeviceString(const Core::String& deviceString)
{
    Device::SetDeviceString(deviceString);
    mOscAddress = "/neurosity/notion/" + deviceString + "/*";
}

int32 NotionDevice::GetOscPathDeviceId(const Core::String& address) const
{
    const Array<String> elements = address.Split(StringCharacter::forwardSlash);
    if (elements.Size() >= 4)
    {
        // should look like: 'local7cca794fb5f4675a69371e949b2'
        ::std::string s = elements[3];

        // must have at least 4 symbols
        if (s.length() >= 4)
        {
            // get last 4 hex symbols (16-bit)
            s = s.substr(s.length() - 4, 4);

            // try hex to bin
            int32 x;
            std::stringstream ss;
            ss << std::hex << s;
            ss >> x;

            // x = 0 if non-hex symbol
            return x != 0 ? x : -1;
        }
    }

    // fail
    return -1;
}

Core::String NotionDevice::GetOscPathDeviceString(const Core::String& address) const
{
    // e.g. returns 'local7cca794fb5f4675a69371e949b2'
    // for '/neurosity/notion/local7cca794fb5f4675a69371e949b2/raw'
    const Array<String> elements = address.Split(StringCharacter::forwardSlash);
    return (elements.Size() >= 4) ? elements[3] : "";
}

#endif
