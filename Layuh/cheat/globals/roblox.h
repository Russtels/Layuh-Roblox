#pragma once
#include <cstdint>
#include <string>
#include <sstream>
#include <windows.h>
#include <winhttp.h>
#include <nlohmann/json.hpp>
#pragma comment(lib, "winhttp.lib")
#include "oxorany/oxorany_include.h"

namespace offsets
{
    constexpr uintptr_t RenderToFakeDataModel = 0x38;
    constexpr std::uint32_t CFrame = 0x11c;

    inline uintptr_t Adornee = 0;
    inline uintptr_t Anchored = 0;
    inline uintptr_t AnimationId = 0;
    inline uintptr_t AttributeList = 0;
    inline uintptr_t AttributeToNext = 0;
    inline uintptr_t AttributeToValue = 0;
    inline uintptr_t AutoJumpEnabled = 0;
    inline uintptr_t BeamBrightness = 0;
    inline uintptr_t BeamColor = 0;
    inline uintptr_t BeamLightEmission = 0;
    inline uintptr_t BeamLightInfuence = 0;
    inline uintptr_t Camera = 0;
    inline uintptr_t CameraMaxZoomDistance = 0;
    inline uintptr_t CameraMinZoomDistance = 0;
    inline uintptr_t CameraMode = 0;
    inline uintptr_t CameraPos = 0;
    inline uintptr_t CameraRotation = 0;
    inline uintptr_t CameraSubject = 0;
    inline uintptr_t CameraType = 0;
    inline uintptr_t CanCollide = 0;
    inline uintptr_t CanTouch = 0;
    inline uintptr_t CharacterAppearanceId = 0;
    inline uintptr_t Children = 0;
    inline uintptr_t ChildrenEnd = 0; // childsize
    inline uintptr_t ClassDescriptor = 0;
    inline uintptr_t ClickDetectorMaxActivationDistance = 0;
    inline uintptr_t ClockTime = 0;
    inline uintptr_t CreatorId = 0;
    inline uintptr_t DataModelDeleterPointer = 0;
    inline uintptr_t DataModelPrimitiveCount = 0;
    inline uintptr_t DataModelToRenderView1 = 0;
    inline uintptr_t DataModelToRenderView2 = 0;
    inline uintptr_t DataModelToRenderView3 = 0;
    inline uintptr_t DecalTexture = 0;
    inline uintptr_t DecryptLuaState = 0;
    inline uintptr_t Deleter = 0;
    inline uintptr_t DeleterBack = 0;
    inline uintptr_t Dimensions = 0;
    inline uintptr_t DisplayName = 0;
    inline uintptr_t EvaluateStateMachine = 0;
    inline uintptr_t FOV = 0;
    inline uintptr_t FakeDataModelPointer = 0;
    inline uintptr_t FakeDataModelToDataModel = 0;
    inline uintptr_t FogColor = 0;
    inline uintptr_t FogEnd = 0;
    inline uintptr_t FogStart = 0;
    inline uintptr_t ForceNewAFKDuration = 0;
    inline uintptr_t FramePositionOffsetX = 0;
    inline uintptr_t FramePositionOffsetY = 0;
    inline uintptr_t FramePositionX = 0;
    inline uintptr_t FramePositionY = 0;
    inline uintptr_t FrameRotation = 0;
    inline uintptr_t FrameSizeX = 0;
    inline uintptr_t FrameSizeY = 0;
    inline uintptr_t GameId = 0;
    inline uintptr_t GameLoaded = 0;
    inline uintptr_t GetGlobalState = 0;
    inline uintptr_t Gravity = 0;
    inline uintptr_t Health = 0;
    inline uintptr_t HealthDisplayDistance = 0;
    inline uintptr_t HipHeight = 0;
    inline uintptr_t HumanoidDisplayName = 0;
    inline uintptr_t HumanoidState = 0;
    inline uintptr_t HumanoidStateId = 0;
    inline uintptr_t InputObject = 0;
    inline uintptr_t InsetMaxX = 0;
    inline uintptr_t InsetMaxY = 0;
    inline uintptr_t InsetMinX = 0;
    inline uintptr_t InsetMinY = 0;
    inline uintptr_t JobEnd = 0;
    inline uintptr_t JobId = 0;
    inline uintptr_t JobStart = 0;
    inline uintptr_t Job_Name = 0;
    inline uintptr_t JobsPointer = 0;
    inline uintptr_t JumpPower = 0;
    inline uintptr_t LocalPlayer = 0;
    inline uintptr_t LocalScriptByteCode = 0;
    inline uintptr_t LocalScriptBytecodePointer = 0;
    inline uintptr_t LocalScriptHash = 0;
    inline uintptr_t MaterialType = 0;
    inline uintptr_t MaxHealth = 0;
    inline uintptr_t MaxSlopeAngle = 0;
    inline uintptr_t MeshPartColor3 = 0;
    inline uintptr_t ModelInstance = 0;
    inline uintptr_t ModuleScriptByteCode = 0;
    inline uintptr_t ModuleScriptBytecodePointer = 0;
    inline uintptr_t ModuleScriptHash = 0;
    inline uintptr_t MoonTextureId = 0;
    inline uintptr_t MousePosition = 0;
    inline uintptr_t MouseSensitivity = 0;
    inline uintptr_t MoveDirection = 0;
    inline uintptr_t Name = 0;
    inline uintptr_t NameDisplayDistance = 0;
    inline uintptr_t NameSize = 0;
    inline uintptr_t OnDemandInstance = 0;
    inline uintptr_t OutdoorAmbient = 0;
    inline uintptr_t Parent = 0;
    inline uintptr_t PartSize = 0;
    inline uintptr_t Ping = 0;
    inline uintptr_t PlaceId = 0;
    inline uintptr_t PlayerConfigurerPointer = 0;
    inline uintptr_t Position = 0;
    inline uintptr_t Primitive = 0;
    inline uintptr_t PrimitiveGravity = 0;
    inline uintptr_t PrimitiveValidateValue = 0;
    inline uintptr_t PrimitivesPointer1 = 0;
    inline uintptr_t PrimitivesPointer2 = 0;
    inline uintptr_t ProximityPromptActionText = 0;
    inline uintptr_t ProximityPromptEnabled = 0;
    inline uintptr_t ProximityPromptGamepadKeyCode = 0;
    inline uintptr_t ProximityPromptHoldDuraction = 0;
    inline uintptr_t ProximityPromptMaxActivationDistance = 0;
    inline uintptr_t ProximityPromptMaxObjectText = 0;
    inline uintptr_t RenderJobToDataModel = 0;
    inline uintptr_t RenderJobToFakeDataModel = 0;
    inline uintptr_t RenderJobToRenderView = 0;
    inline uintptr_t RequireBypass = 0;
    inline uintptr_t RigType = 0;
    inline uintptr_t Rotation = 0;
    inline uintptr_t RunContext = 0;
    inline uintptr_t ScriptContext = 0;
    inline uintptr_t Sit = 0;
    inline uintptr_t SkyboxBk = 0;
    inline uintptr_t SkyboxDn = 0;
    inline uintptr_t SkyboxFt = 0;
    inline uintptr_t SkyboxLf = 0;
    inline uintptr_t SkyboxRt = 0;
    inline uintptr_t SkyboxUp = 0;
    inline uintptr_t SoundId = 0;
    inline uintptr_t StarCount = 0;
    inline uintptr_t StringLength = 0;
    inline uintptr_t SunTextureId = 0;
    inline uintptr_t TagList = 0;
    inline uintptr_t TaskSchedulerMaxFPS = 0;
    inline uintptr_t TaskSchedulerPointer = 0;
    inline uintptr_t Team = 0;
    inline uintptr_t TeamColor = 0;
    inline uintptr_t Tool_Grip_Position = 0;
    inline uintptr_t Transparency = 0;
    inline uintptr_t UserId = 0;
    inline uintptr_t Value = 0;
    inline uintptr_t Velocity = 0;
    inline uintptr_t ViewportSize = 0;
    inline uintptr_t VisualEngine = 0;
    inline uintptr_t VisualEnginePointer = 0;
    inline uintptr_t VisualEngineToDataModel1 = 0;
    inline uintptr_t VisualEngineToDataModel2 = 0;
    inline uintptr_t WalkSpeed = 0;
    inline uintptr_t WalkSpeedCheck = 0;
    inline uintptr_t WhitelistEncryption = 0;
    inline uintptr_t WhitelistEncryption2 = 0;
    inline uintptr_t WhitelistSetInsert = 0;
    inline uintptr_t WhitelistedPages = 0;
    inline uintptr_t WhitelistedThreads = 0;
    inline uintptr_t Workspace = 0;
    inline uintptr_t viewmatrix = 0;

    inline std::string RobloxVersionString;

    inline void autoupdate()
    {
        std::wstring host = oxorany(L"offsets.ntgetwritewatch.workers.dev");
        std::wstring path = oxorany(L"/offsets.json");
        std::string response_string;

        HINTERNET hSession = WinHttpOpen(oxorany(L"RobloxOffsets/1.0"), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
        if (!hSession) return;
        HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); return; }
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, oxorany(L"GET"), path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
        if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return; }

        BOOL bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)
            && WinHttpReceiveResponse(hRequest, NULL);

        if (bResults) {
            DWORD dwSize = 0;
            do {
                DWORD dwDownloaded = 0;
                if (!WinHttpQueryDataAvailable(hRequest, &dwSize) || !dwSize) break;
                std::string buffer(dwSize, 0);
                if (!WinHttpReadData(hRequest, &buffer[0], dwSize, &dwDownloaded)) break;
                response_string.append(buffer, 0, dwDownloaded);
            } while (dwSize > 0);
        }

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);

        if (response_string.empty()) return;

        auto j = nlohmann::json::parse(response_string, nullptr, false);
        if (j.is_discarded()) return; // holy shit takes forever

        if (j.contains(oxorany("RobloxVersion")))
            RobloxVersionString = j[oxorany("RobloxVersion")].get<std::string>();

#define UPDATE_OFFSET(name) \
            if (j.contains(#name)) { \
                std::stringstream ss; \
                ss << std::hex << j[#name].get<std::string>(); \
                ss >> offsets::name; \
            }

        UPDATE_OFFSET(Adornee)
            UPDATE_OFFSET(Anchored)
            UPDATE_OFFSET(AnimationId)
            UPDATE_OFFSET(AttributeList)
            UPDATE_OFFSET(AttributeToNext)
            UPDATE_OFFSET(AttributeToValue)
            UPDATE_OFFSET(AutoJumpEnabled)
            UPDATE_OFFSET(BeamBrightness)
            UPDATE_OFFSET(BeamColor)
            UPDATE_OFFSET(BeamLightEmission)
            UPDATE_OFFSET(BeamLightInfuence)
            UPDATE_OFFSET(Camera)
            UPDATE_OFFSET(CameraMaxZoomDistance)
            UPDATE_OFFSET(CameraMinZoomDistance)
            UPDATE_OFFSET(CameraMode)
            UPDATE_OFFSET(CameraPos)
            UPDATE_OFFSET(CameraRotation)
            UPDATE_OFFSET(CameraSubject)
            UPDATE_OFFSET(CameraType)
            UPDATE_OFFSET(CanCollide)
            UPDATE_OFFSET(CanTouch)
            UPDATE_OFFSET(CharacterAppearanceId)
            UPDATE_OFFSET(Children)
            UPDATE_OFFSET(ChildrenEnd)
            UPDATE_OFFSET(ClassDescriptor)
            UPDATE_OFFSET(ClickDetectorMaxActivationDistance)
            UPDATE_OFFSET(ClockTime)
            UPDATE_OFFSET(CreatorId)
            UPDATE_OFFSET(DataModelDeleterPointer)
            UPDATE_OFFSET(DataModelPrimitiveCount)
            UPDATE_OFFSET(DataModelToRenderView1)
            UPDATE_OFFSET(DataModelToRenderView2)
            UPDATE_OFFSET(DataModelToRenderView3)
            UPDATE_OFFSET(DecalTexture)
            UPDATE_OFFSET(DecryptLuaState)
            UPDATE_OFFSET(Deleter)
            UPDATE_OFFSET(DeleterBack)
            UPDATE_OFFSET(Dimensions)
            UPDATE_OFFSET(DisplayName)
            UPDATE_OFFSET(EvaluateStateMachine)
            UPDATE_OFFSET(FOV)
            UPDATE_OFFSET(FakeDataModelPointer)
            UPDATE_OFFSET(FakeDataModelToDataModel)
            UPDATE_OFFSET(FogColor)
            UPDATE_OFFSET(FogEnd)
            UPDATE_OFFSET(FogStart)
            UPDATE_OFFSET(ForceNewAFKDuration)
            UPDATE_OFFSET(FramePositionOffsetX)
            UPDATE_OFFSET(FramePositionOffsetY)
            UPDATE_OFFSET(FramePositionX)
            UPDATE_OFFSET(FramePositionY)
            UPDATE_OFFSET(FrameRotation)
            UPDATE_OFFSET(FrameSizeX)
            UPDATE_OFFSET(FrameSizeY)
            UPDATE_OFFSET(GameId)
            UPDATE_OFFSET(GameLoaded)
            UPDATE_OFFSET(GetGlobalState)
            UPDATE_OFFSET(Gravity)
            UPDATE_OFFSET(Health)
            UPDATE_OFFSET(HealthDisplayDistance)
            UPDATE_OFFSET(HipHeight)
            UPDATE_OFFSET(HumanoidDisplayName)
            UPDATE_OFFSET(HumanoidState)
            UPDATE_OFFSET(HumanoidStateId)
            UPDATE_OFFSET(InputObject)
            UPDATE_OFFSET(InsetMaxX)
            UPDATE_OFFSET(InsetMaxY)
            UPDATE_OFFSET(InsetMinX)
            UPDATE_OFFSET(InsetMinY)
            UPDATE_OFFSET(JobEnd)
            UPDATE_OFFSET(JobId)
            UPDATE_OFFSET(JobStart)
            UPDATE_OFFSET(Job_Name)
            UPDATE_OFFSET(JobsPointer)
            UPDATE_OFFSET(JumpPower)
            UPDATE_OFFSET(LocalPlayer)
            UPDATE_OFFSET(LocalScriptByteCode)
            UPDATE_OFFSET(LocalScriptBytecodePointer)
            UPDATE_OFFSET(LocalScriptHash)
            UPDATE_OFFSET(MaterialType)
            UPDATE_OFFSET(MaxHealth)
            UPDATE_OFFSET(MaxSlopeAngle)
            UPDATE_OFFSET(MeshPartColor3)
            UPDATE_OFFSET(ModelInstance)
            UPDATE_OFFSET(ModuleScriptByteCode)
            UPDATE_OFFSET(ModuleScriptBytecodePointer)
            UPDATE_OFFSET(ModuleScriptHash)
            UPDATE_OFFSET(MoonTextureId)
            UPDATE_OFFSET(MousePosition)
            UPDATE_OFFSET(MouseSensitivity)
            UPDATE_OFFSET(MoveDirection)
            UPDATE_OFFSET(Name)
            UPDATE_OFFSET(NameDisplayDistance)
            UPDATE_OFFSET(NameSize)
            UPDATE_OFFSET(OnDemandInstance)
            UPDATE_OFFSET(OutdoorAmbient)
            UPDATE_OFFSET(Parent)
            UPDATE_OFFSET(PartSize)
            UPDATE_OFFSET(Ping)
            UPDATE_OFFSET(PlaceId)
            UPDATE_OFFSET(PlayerConfigurerPointer)
            UPDATE_OFFSET(Position)
            UPDATE_OFFSET(Primitive)
            UPDATE_OFFSET(PrimitiveGravity)
            UPDATE_OFFSET(PrimitiveValidateValue)
            UPDATE_OFFSET(PrimitivesPointer1)
            UPDATE_OFFSET(PrimitivesPointer2)
            UPDATE_OFFSET(ProximityPromptActionText)
            UPDATE_OFFSET(ProximityPromptEnabled)
            UPDATE_OFFSET(ProximityPromptGamepadKeyCode)
            UPDATE_OFFSET(ProximityPromptHoldDuraction)
            UPDATE_OFFSET(ProximityPromptMaxActivationDistance)
            UPDATE_OFFSET(ProximityPromptMaxObjectText)
            UPDATE_OFFSET(RenderJobToDataModel)
            UPDATE_OFFSET(RenderJobToFakeDataModel)
            UPDATE_OFFSET(RenderJobToRenderView)
            UPDATE_OFFSET(RequireBypass)
            UPDATE_OFFSET(RigType)
            UPDATE_OFFSET(Rotation)
            UPDATE_OFFSET(RunContext)
            UPDATE_OFFSET(ScriptContext)
            UPDATE_OFFSET(Sit)
            UPDATE_OFFSET(SkyboxBk)
            UPDATE_OFFSET(SkyboxDn)
            UPDATE_OFFSET(SkyboxFt)
            UPDATE_OFFSET(SkyboxLf)
            UPDATE_OFFSET(SkyboxRt)
            UPDATE_OFFSET(SkyboxUp)
            UPDATE_OFFSET(SoundId)
            UPDATE_OFFSET(StarCount)
            UPDATE_OFFSET(StringLength)
            UPDATE_OFFSET(SunTextureId)
            UPDATE_OFFSET(TagList)
            UPDATE_OFFSET(TaskSchedulerMaxFPS)
            UPDATE_OFFSET(TaskSchedulerPointer)
            UPDATE_OFFSET(Team)
            UPDATE_OFFSET(TeamColor)
            UPDATE_OFFSET(Tool_Grip_Position)
            UPDATE_OFFSET(Transparency)
            UPDATE_OFFSET(UserId)
            UPDATE_OFFSET(Value)
            UPDATE_OFFSET(Velocity)
            UPDATE_OFFSET(ViewportSize)
            UPDATE_OFFSET(VisualEngine)
            UPDATE_OFFSET(VisualEnginePointer)
            UPDATE_OFFSET(VisualEngineToDataModel1)
            UPDATE_OFFSET(VisualEngineToDataModel2)
            UPDATE_OFFSET(WalkSpeed)
            UPDATE_OFFSET(WalkSpeedCheck)
            UPDATE_OFFSET(WhitelistEncryption)
            UPDATE_OFFSET(WhitelistEncryption2)
            UPDATE_OFFSET(WhitelistSetInsert)
            UPDATE_OFFSET(WhitelistedPages)
            UPDATE_OFFSET(WhitelistedThreads)
            UPDATE_OFFSET(Workspace)
            UPDATE_OFFSET(viewmatrix)

#undef UPDATE_OFFSET
    }
}
