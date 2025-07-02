#pragma once
#include "core_types.h"
#include <string_view>
#include <vector>
#include <optional>

#define UTF8_DankHug "\xEE\x80\x80"
#define UTF8_FeelsOkayMan "\xEE\x80\x81"
#define UTF8_PeepoCoffeeBlonket "\xEE\x80\x82"
#define UTF8_PeepoHappy "\xEE\x80\x83"

// NOTE: Get a list of all included icons, primarily for debugging purposes
struct EmbeddedIconsList { struct Data { cstr Name; char UTF8[5]; }; Data* V; size_t Count; };
EmbeddedIconsList GetEmbeddedIconsList();

// NOTE: Exposed here specifically to be used with PushFont() / PopFont()
struct ImFont;
inline ImFont* FontMain = nullptr;
enum FontBaseSizes : i32 { Small = 16, Medium = 18, Large = 22 };
inline std::string FontMainFileNameDefault = "NotoSansCJKjp-Regular.otf";
inline std::string FontMainFileNameTarget = FontMainFileNameDefault;
inline std::string FontMainFileNameCurrent = "";

inline f32 GuiScaleFactorCurrent = 1.0f;
inline f32 GuiScaleFactorTarget = GuiScaleFactorCurrent;
inline f32 GuiScaleFactorToSetNextFrame = GuiScaleFactorCurrent;
inline f32 GuiScale(f32 value) { return Floor(value * GuiScaleFactorCurrent); }
inline f32 GuiScale_AtTarget(f32 value) { return Floor(value * GuiScaleFactorTarget); }
inline vec2 GuiScale(vec2 value) { return Floor(value * GuiScaleFactorCurrent); }
inline vec2 GuiScale_AtTarget(vec2 value) { return Floor(value * GuiScaleFactorTarget); }
inline i32 GuiScaleI32(i32 value) { return static_cast<i32>(Floor(static_cast<f32>(value) * GuiScaleFactorCurrent)); }
inline i32 GuiScaleI32_AtTarget(i32 value) { return static_cast<i32>(Floor(static_cast<f32>(value) * GuiScaleFactorTarget)); }

constexpr f32 GuiScaleFactorMin = FromPercent(50.0f);
constexpr f32 GuiScaleFactorMax = FromPercent(300.0f);
constexpr f32 ClampRoundGuiScaleFactor(f32 scaleFactor) { return Clamp(FromPercent(Round(ToPercent(scaleFactor))), GuiScaleFactorMin, GuiScaleFactorMax); }

inline b8 EnableGuiScaleAnimation = true;
inline b8 IsGuiScaleCurrentlyAnimating = false;
inline f32 GuiScaleFactorLastAnimationStart = 1.0f;
inline f32 GuiScaleAnimationElapsed = 0.0f;
inline f32 GuiScaleAnimationDuration = 0.25f;

namespace ApplicationHost
{
	struct StartupParam
	{
		std::string_view WindowTitle = {};
		std::optional<ivec2> WindowPosition = {};
		std::optional<ivec2> WindowSize = {};
		b8 WaitableSwapChain = true;
		b8 AllowSwapChainTearing = false;
	};

	struct State
	{
		// NOTE: READ ONLY
		// --------------------------------
		void* NativeWindowHandle;
		ivec2 WindowPosition;
		ivec2 WindowSize;
		b8 IsBorderlessFullscreen;
		b8 IsAnyWindowFocusedThisFrame;
		b8 IsAnyWindowFocusedLastFrame;
		b8 HasAllFocusBeenLostThisFrame;
		b8 HasAnyFocusBeenGainedThisFrame;
		// TODO: Implement the notion of "handling" a dropped file event
		std::vector<std::string> FilePathsDroppedThisFrame;
		std::string WindowTitle;
		void* FontFileContent;
		size_t FontFileContentSize;
		// --------------------------------

		// NOTE: READ + WRITE
		// --------------------------------
		i32 SwapInterval = 1;
		std::string SetWindowTitleNextFrame;
		std::optional<ivec2> SetWindowPositionNextFrame;
		std::optional<ivec2> SetWindowSizeNextFrame;
		std::optional<ivec2> MinWindowSizeRestraints = ivec2(640, 360);
		std::optional<b8> SetBorderlessFullscreenNextFrame;
		std::optional<i32> RequestExitNextFrame;
		// --------------------------------
	};

	inline State GlobalState = {};

	// NOTE: Specifically to handle the case of unsaved user data
	enum class CloseResponse : u8 { Exit, SupressExit };

	using StartupFunc = void(*)();
	using BeforeUpdateFunc = void(*)();
	using UpdateFunc = void(*)();
	using ShutdownFunc = void(*)();
	using WindowCloseRequestFunc = CloseResponse(*)();

	struct UserCallbacks
	{
		StartupFunc OnStartup;
		BeforeUpdateFunc OnBeforeUpdate;
		UpdateFunc OnUpdate;
		ShutdownFunc OnShutdown;
		WindowCloseRequestFunc OnWindowCloseRequest;
	};

	i32 EnterProgramLoop(const StartupParam& startupParam, UserCallbacks userCallbacks);
}
