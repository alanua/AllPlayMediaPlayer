//-----------------------------------------------------------------------------
// <auto-generated> 
//   This code was generated by a tool. 
// 
//   Changes to this file may cause incorrect behavior and will be lost if  
//   the code is regenerated.
//
//   Tool: AllJoynCodeGenerator.exe
//
//   This tool is located in the Windows 10 SDK and the Windows 10 AllJoyn 
//   Visual Studio Extension in the Visual Studio Gallery.  
//
//   The generated code should be packaged in a Windows 10 C++/CX Runtime  
//   Component which can be consumed in any UWP-supported language using 
//   APIs that are available in Windows.Devices.AllJoyn.
//
//   Using AllJoynCodeGenerator - Invoke the following command with a valid 
//   Introspection XML file and a writable output directory:
//     AllJoynCodeGenerator -i <INPUT XML FILE> -o <OUTPUT DIRECTORY>
// </auto-generated>
//-----------------------------------------------------------------------------
#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace net::allplay::Control::Volume;

std::map<alljoyn_busobject, WeakReference*> VolumeProducer::SourceObjects;
std::map<alljoyn_interfacedescription, WeakReference*> VolumeProducer::SourceInterfaces;

VolumeProducer::VolumeProducer(AllJoynBusAttachment^ busAttachment)
    : m_busAttachment(busAttachment),
    m_sessionListener(nullptr),
    m_busObject(nullptr),
    m_sessionPort(0),
    m_sessionId(0)
{
    m_weak = new WeakReference(this);
    ServiceObjectPath = ref new String(L"/net/allplay/MediaPlayer");
    m_signals = ref new VolumeSignals();
    m_busAttachmentStateChangedToken.Value = 0;
}

VolumeProducer::~VolumeProducer()
{
    UnregisterFromBus();
    delete m_weak;
}

void VolumeProducer::UnregisterFromBus()
{
    if ((nullptr != m_busAttachment) && (0 != m_busAttachmentStateChangedToken.Value))
    {
        m_busAttachment->StateChanged -= m_busAttachmentStateChangedToken;
        m_busAttachmentStateChangedToken.Value = 0;
    }
    if ((nullptr != m_busAttachment) && (nullptr != SessionPortListener))
    {
        alljoyn_busattachment_unbindsessionport(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), m_sessionPort);
        alljoyn_sessionportlistener_destroy(SessionPortListener);
        SessionPortListener = nullptr;
    }
    if ((nullptr != m_busAttachment) && (nullptr != BusObject))
    {
        alljoyn_busattachment_unregisterbusobject(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), BusObject);
        alljoyn_busobject_destroy(BusObject);
        BusObject = nullptr;
    }
    if (nullptr != SessionListener)
    {
        alljoyn_sessionlistener_destroy(SessionListener);
        SessionListener = nullptr;
    }
}

bool VolumeProducer::OnAcceptSessionJoiner(_In_ alljoyn_sessionport sessionPort, _In_ PCSTR joiner, _In_ const alljoyn_sessionopts opts)
{
    UNREFERENCED_PARAMETER(sessionPort); UNREFERENCED_PARAMETER(joiner); UNREFERENCED_PARAMETER(opts);
    
    return true;
}

void VolumeProducer::InitializeSignals(_In_ alljoyn_sessionid id)
{
	m_signals->Initialize(BusObject, id);
	m_sessionId = id;

}
void VolumeProducer::OnSessionJoined(_In_ alljoyn_sessionport sessionPort, _In_ alljoyn_sessionid id, _In_ PCSTR joiner)
{
    UNREFERENCED_PARAMETER(joiner);

    // We initialize the Signals object after the session has been joined, because it needs
    // the session id.
    m_signals->Initialize(BusObject, id);
    m_sessionPort = sessionPort;
    m_sessionId = id;

    alljoyn_sessionlistener_callbacks callbacks =
    {
        AllJoynHelpers::SessionLostHandler<VolumeProducer>,
        AllJoynHelpers::SessionMemberAddedHandler<VolumeProducer>,
        AllJoynHelpers::SessionMemberRemovedHandler<VolumeProducer>
    };

    SessionListener = alljoyn_sessionlistener_create(&callbacks, m_weak);
    alljoyn_busattachment_setsessionlistener(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), id, SessionListener);
}

void VolumeProducer::OnSessionLost(_In_ alljoyn_sessionid sessionId, _In_ alljoyn_sessionlostreason reason)
{
    if (sessionId == m_sessionId)
    {
        AllJoynSessionLostEventArgs^ args = ref new AllJoynSessionLostEventArgs(static_cast<AllJoynSessionLostReason>(reason));
        SessionLost(this, args);
    }
}

void VolumeProducer::OnSessionMemberAdded(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberAddedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberAdded(this, args);
    }
}

void VolumeProducer::OnSessionMemberRemoved(_In_ alljoyn_sessionid sessionId, _In_ PCSTR uniqueName)
{
    if (sessionId == m_sessionId)
    {
        auto args = ref new AllJoynSessionMemberRemovedEventArgs(AllJoynHelpers::MultibyteToPlatformString(uniqueName));
        SessionMemberRemoved(this, args);
    }
}

void VolumeProducer::BusAttachmentStateChanged(_In_ AllJoynBusAttachment^ sender, _In_ AllJoynBusAttachmentStateChangedEventArgs^ args)
{
    if (args->State == AllJoynBusAttachmentState::Connected)
    {   
        QStatus result = AllJoynHelpers::CreateProducerSession<VolumeProducer>(m_busAttachment, m_weak);
        if (ER_OK != result)
        {
            StopInternal(result);
            return;
        }
    }
    else if (args->State == AllJoynBusAttachmentState::Disconnected)
    {
        StopInternal(ER_BUS_STOPPING);
    }
}

void VolumeProducer::CallAdjustVolumeHandler(_Inout_ alljoyn_busobject busObject, _In_ alljoyn_message message)
{
    auto source = SourceObjects.find(busObject);
    if (source == SourceObjects.end())
    {
        return;
    }

    VolumeProducer^ producer = source->second->Resolve<VolumeProducer>();
    if (producer->Service != nullptr)
    {
        AllJoynMessageInfo^ callInfo = ref new AllJoynMessageInfo(AllJoynHelpers::MultibyteToPlatformString(alljoyn_message_getsender(message)));

        int16 inputArg0;
        (void)TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 0), "n", &inputArg0);

        VolumeAdjustVolumeResult^ result = create_task(producer->Service->AdjustVolumeAsync(callInfo, inputArg0)).get();
        create_task([](){}).then([=]
        {
            int32 status;

            if (nullptr == result)
            {
                alljoyn_busobject_methodreply_status(busObject, message, ER_BUS_NO_LISTENER);
                return;
            }

            status = result->Status;
            if (AllJoynStatus::Ok != status)
            {
                alljoyn_busobject_methodreply_status(busObject, message, static_cast<QStatus>(status));
                return;
            }

            size_t argCount = 0;
            alljoyn_msgarg outputs = alljoyn_msgarg_array_create(argCount);

            alljoyn_busobject_methodreply_args(busObject, message, outputs, argCount);
            alljoyn_msgarg_destroy(outputs);
        }, result->m_creationContext).wait();
    }
}

void VolumeProducer::CallAdjustVolumePercentHandler(_Inout_ alljoyn_busobject busObject, _In_ alljoyn_message message)
{
    auto source = SourceObjects.find(busObject);
    if (source == SourceObjects.end())
    {
        return;
    }

    VolumeProducer^ producer = source->second->Resolve<VolumeProducer>();
    if (producer->Service != nullptr)
    {
        AllJoynMessageInfo^ callInfo = ref new AllJoynMessageInfo(AllJoynHelpers::MultibyteToPlatformString(alljoyn_message_getsender(message)));

        double inputArg0;
        (void)TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 0), "d", &inputArg0);

        VolumeAdjustVolumePercentResult^ result = create_task(producer->Service->AdjustVolumePercentAsync(callInfo, inputArg0)).get();
        create_task([](){}).then([=]
        {
            int32 status;

            if (nullptr == result)
            {
                alljoyn_busobject_methodreply_status(busObject, message, ER_BUS_NO_LISTENER);
                return;
            }

            status = result->Status;
            if (AllJoynStatus::Ok != status)
            {
                alljoyn_busobject_methodreply_status(busObject, message, static_cast<QStatus>(status));
                return;
            }

            size_t argCount = 0;
            alljoyn_msgarg outputs = alljoyn_msgarg_array_create(argCount);

            alljoyn_busobject_methodreply_args(busObject, message, outputs, argCount);
            alljoyn_msgarg_destroy(outputs);
        }, result->m_creationContext).wait();
    }
}

void VolumeProducer::CallEnabledChangedSignalHandler(_In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message message)
{
    auto source = SourceInterfaces.find(member->iface);
    if (source == SourceInterfaces.end())
    {
        return;
    }

    auto producer = source->second->Resolve<VolumeProducer>();
    if (producer->Signals != nullptr)
    {
        auto callInfo = ref new AllJoynMessageInfo(AllJoynHelpers::MultibyteToPlatformString(alljoyn_message_getsender(message)));
        auto eventArgs = ref new VolumeEnabledChangedReceivedEventArgs();
        eventArgs->MessageInfo = callInfo;

        bool argument0;
        (void)TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 0), "b", &argument0);
        eventArgs->Enabled = argument0;

        producer->Signals->CallEnabledChangedReceived(producer->Signals, eventArgs);
    }
}

void VolumeProducer::CallMuteChangedSignalHandler(_In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message message)
{
    auto source = SourceInterfaces.find(member->iface);
    if (source == SourceInterfaces.end())
    {
        return;
    }

    auto producer = source->second->Resolve<VolumeProducer>();
    if (producer->Signals != nullptr)
    {
        auto callInfo = ref new AllJoynMessageInfo(AllJoynHelpers::MultibyteToPlatformString(alljoyn_message_getsender(message)));
        auto eventArgs = ref new VolumeMuteChangedReceivedEventArgs();
        eventArgs->MessageInfo = callInfo;

        bool argument0;
        (void)TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 0), "b", &argument0);
        eventArgs->NewMute = argument0;

        producer->Signals->CallMuteChangedReceived(producer->Signals, eventArgs);
    }
}

void VolumeProducer::CallVolumeChangedSignalHandler(_In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message message)
{
    auto source = SourceInterfaces.find(member->iface);
    if (source == SourceInterfaces.end())
    {
        return;
    }

    auto producer = source->second->Resolve<VolumeProducer>();
    if (producer->Signals != nullptr)
    {
        auto callInfo = ref new AllJoynMessageInfo(AllJoynHelpers::MultibyteToPlatformString(alljoyn_message_getsender(message)));
        auto eventArgs = ref new VolumeVolumeChangedReceivedEventArgs();
        eventArgs->MessageInfo = callInfo;

        int16 argument0;
        (void)TypeConversionHelpers::GetAllJoynMessageArg(alljoyn_message_getarg(message, 0), "n", &argument0);
        eventArgs->NewVolume = argument0;

        producer->Signals->CallVolumeChangedReceived(producer->Signals, eventArgs);
    }
}

QStatus VolumeProducer::AddMethodHandler(_In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_methodhandler_ptr handler)
{
    alljoyn_interfacedescription_member member;
    if (!alljoyn_interfacedescription_getmember(interfaceDescription, methodName, &member))
    {
        return ER_BUS_INTERFACE_NO_SUCH_MEMBER;
    }

    return alljoyn_busobject_addmethodhandler(
        m_busObject,
        member,
        handler,
        m_weak);
}

QStatus VolumeProducer::AddSignalHandler(_In_ alljoyn_busattachment busAttachment, _In_ alljoyn_interfacedescription interfaceDescription, _In_ PCSTR methodName, _In_ alljoyn_messagereceiver_signalhandler_ptr handler)
{
    alljoyn_interfacedescription_member member;
    if (!alljoyn_interfacedescription_getmember(interfaceDescription, methodName, &member))
    {
        return ER_BUS_INTERFACE_NO_SUCH_MEMBER;
    }

    return alljoyn_busattachment_registersignalhandler(busAttachment, handler, member, NULL);
}

QStatus VolumeProducer::OnPropertyGet(_In_ PCSTR interfaceName, _In_ PCSTR propertyName, _Inout_ alljoyn_msgarg value)
{
    UNREFERENCED_PARAMETER(interfaceName);

    if (0 == strcmp(propertyName, "Enabled"))
    {
        auto task = create_task(Service->GetEnabledAsync(nullptr));
        auto result = task.get();

        return create_task([](){}).then([=]() -> QStatus
        {
            if (AllJoynStatus::Ok != result->Status)
            {
                return static_cast<QStatus>(result->Status);
            }
            return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "b", result->Enabled));
        }, result->m_creationContext).get();
    }
    if (0 == strcmp(propertyName, "Mute"))
    {
        auto task = create_task(Service->GetMuteAsync(nullptr));
        auto result = task.get();

        return create_task([](){}).then([=]() -> QStatus
        {
            if (AllJoynStatus::Ok != result->Status)
            {
                return static_cast<QStatus>(result->Status);
            }
            return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "b", result->Mute));
        }, result->m_creationContext).get();
    }
    if (0 == strcmp(propertyName, "Version"))
    {
        auto task = create_task(Service->GetVersionAsync(nullptr));
        auto result = task.get();

        return create_task([](){}).then([=]() -> QStatus
        {
            if (AllJoynStatus::Ok != result->Status)
            {
                return static_cast<QStatus>(result->Status);
            }
            return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "q", result->Version));
        }, result->m_creationContext).get();
    }
    if (0 == strcmp(propertyName, "Volume"))
    {
        auto task = create_task(Service->GetVolumeAsync(nullptr));
        auto result = task.get();

        return create_task([](){}).then([=]() -> QStatus
        {
            if (AllJoynStatus::Ok != result->Status)
            {
                return static_cast<QStatus>(result->Status);
            }
            return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "n", result->Volume));
        }, result->m_creationContext).get();
    }
    if (0 == strcmp(propertyName, "VolumeRange"))
    {
        auto task = create_task(Service->GetVolumeRangeAsync(nullptr));
        auto result = task.get();

        return create_task([](){}).then([=]() -> QStatus
        {
            if (AllJoynStatus::Ok != result->Status)
            {
                return static_cast<QStatus>(result->Status);
            }
            return static_cast<QStatus>(TypeConversionHelpers::SetAllJoynMessageArg(value, "(nnn)", result->VolumeRange));
        }, result->m_creationContext).get();
    }

    return ER_BUS_NO_SUCH_PROPERTY;
}

QStatus VolumeProducer::OnPropertySet(_In_ PCSTR interfaceName, _In_ PCSTR propertyName, _In_ alljoyn_msgarg value)
{
    UNREFERENCED_PARAMETER(interfaceName);

    if (0 == strcmp(propertyName, "Mute"))
    {
        bool argument;
        QStatus status = static_cast<QStatus>(TypeConversionHelpers::GetAllJoynMessageArg(value, "b", &argument));
        if (ER_OK == status)
        {
            auto task = create_task(Service->SetMuteAsync(nullptr, argument));
            auto result = task.get();
            status = static_cast<QStatus>(result->Status);
        }
        return status;
    }
    if (0 == strcmp(propertyName, "Volume"))
    {
        int16 argument;
        QStatus status = static_cast<QStatus>(TypeConversionHelpers::GetAllJoynMessageArg(value, "n", &argument));
        if (ER_OK == status)
        {
            auto task = create_task(Service->SetVolumeAsync(nullptr, argument));
            auto result = task.get();
            status = static_cast<QStatus>(result->Status);
        }
        return status;
    }
    return ER_BUS_NO_SUCH_PROPERTY;
}

void VolumeProducer::Start(alljoyn_busobject bus)
{
	m_busObject = bus;
    if (nullptr == m_busAttachment)
    {
        StopInternal(ER_FAIL);
        return;
    }

    QStatus result = AllJoynHelpers::CreateInterfaces(m_busAttachment, c_VolumeIntrospectionXml);
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

	if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }
	
    alljoyn_interfacedescription interfaceDescription = alljoyn_busattachment_getinterface(AllJoynHelpers::GetInternalBusAttachment(m_busAttachment), "org.alljoyn.Control.Volume");
    if (interfaceDescription == nullptr)
    {
        StopInternal(ER_FAIL);
        return;
    }
    alljoyn_busobject_addinterface_announced(BusObject, interfaceDescription);
	
    result = AddMethodHandler(
        interfaceDescription, 
        "AdjustVolume", 
        [](alljoyn_busobject busObject, const alljoyn_interfacedescription_member* member, alljoyn_message message) { UNREFERENCED_PARAMETER(member); CallAdjustVolumeHandler(busObject, message); });
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    result = AddMethodHandler(
        interfaceDescription, 
        "AdjustVolumePercent", 
        [](alljoyn_busobject busObject, const alljoyn_interfacedescription_member* member, alljoyn_message message) { UNREFERENCED_PARAMETER(member); CallAdjustVolumePercentHandler(busObject, message); });
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }

    result = AddSignalHandler(
        AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
        interfaceDescription,
        "EnabledChanged",
        [](const alljoyn_interfacedescription_member* member, PCSTR srcPath, alljoyn_message message) { UNREFERENCED_PARAMETER(srcPath); CallEnabledChangedSignalHandler(member, message); });
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }
    result = AddSignalHandler(
        AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
        interfaceDescription,
        "MuteChanged",
        [](const alljoyn_interfacedescription_member* member, PCSTR srcPath, alljoyn_message message) { UNREFERENCED_PARAMETER(srcPath); CallMuteChangedSignalHandler(member, message); });
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }
    result = AddSignalHandler(
        AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
        interfaceDescription,
        "VolumeChanged",
        [](const alljoyn_interfacedescription_member* member, PCSTR srcPath, alljoyn_message message) { UNREFERENCED_PARAMETER(srcPath); CallVolumeChangedSignalHandler(member, message); });
    if (result != ER_OK)
    {
        StopInternal(result);
        return;
    }
    
    SourceObjects[m_busObject] = m_weak;
    SourceInterfaces[interfaceDescription] = m_weak;    
}

void VolumeProducer::Stop()
{
    StopInternal(AllJoynStatus::Ok);
}

void VolumeProducer::StopInternal(int32 status)
{
    UnregisterFromBus();
    Stopped(this, ref new AllJoynProducerStoppedEventArgs(status));
}

int32 VolumeProducer::RemoveMemberFromSession(_In_ String^ uniqueName)
{
    return alljoyn_busattachment_removesessionmember(
        AllJoynHelpers::GetInternalBusAttachment(m_busAttachment),
        m_sessionId,
        AllJoynHelpers::PlatformToMultibyteString(uniqueName).data());
}

PCSTR net::allplay::Control::Volume::c_VolumeIntrospectionXml = "<interface name=\"org.alljoyn.Control.Volume\">"
"  <method name=\"AdjustVolume\">"
"    <arg name=\"delta\" type=\"n\" direction=\"in\" />"
"  </method>"
"  <method name=\"AdjustVolumePercent\">"
"    <arg name=\"change\" type=\"d\" direction=\"in\" />"
"  </method>"
"  <signal name=\"EnabledChanged\">"
"    <arg name=\"enabled\" type=\"b\" direction=\"out\" />"
"  </signal>"
"  <signal name=\"MuteChanged\">"
"    <arg name=\"newMute\" type=\"b\" direction=\"out\" />"
"  </signal>"
"  <signal name=\"VolumeChanged\">"
"    <arg name=\"newVolume\" type=\"n\" direction=\"out\" />"
"  </signal>"
"  <property name=\"Enabled\" type=\"b\" access=\"read\" />"
"  <property name=\"Mute\" type=\"b\" access=\"readwrite\" />"
"  <property name=\"Version\" type=\"q\" access=\"read\" />"
"  <property name=\"Volume\" type=\"n\" access=\"readwrite\" />"
"  <property name=\"VolumeRange\" type=\"(nnn)\" access=\"read\" />"
"</interface>"
;
