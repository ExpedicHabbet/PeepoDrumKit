﻿#pragma once
#include "core_types.h"
#include "imgui/imgui_include.h"

#include <shared_mutex>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "chart_editor_settings.h"

#define PEEPODRUMKIT_UI_STRINGS_X_MACRO_LIST_EN \
X("Game Preview") \
X("Chart Timeline") \
X("Chart Timeline - Debug") \
X("Chart Properties") \
X("Chart Events") \
X("Chart Lyrics") \
X("Tempo Calculator") \
X("Undo History") \
X("Chart Inspector") \
X("Settings") \
X("Usage Guide") \
X("Update Notes") \
X("TJA Export Debug View") \
X("TJA Import Test") \
X("Audio Test") \
X("File") \
X("Edit") \
X("Selection") \
X("Transform") \
X("Window") \
X("Language") \
X("English") \
X("Japanese") \
X("Simplified Chinese") \
X("Traditional Chinese") \
X("%s (%s)") \
X("Help") \
X("Copy") \
X("Delete") \
X("Save") \
X("Undo") \
X("Redo") \
X("Cut") \
X("Paste") \
X("Open Recent") \
X("Exit") \
X("New Chart") \
X("Open...") \
X("Clear Items") \
X("Open Chart Directory...") \
X("Save As...") \
X("Refine Selection") \
X("Select All") \
X("Clear Selection") \
X("Invert Selection") \
X("Start Range Selection") \
X("End Range Selection") \
X("From Range Selection") \
X("Shift selection Left") \
X("Shift selection Right") \
X("Select Item Pattern xo") \
X("Select Item Pattern xoo") \
X("Select Item Pattern xooo") \
X("Select Item Pattern xxoo") \
X("Add New Pattern...") \
X("Select Custom Pattern") \
X("Delete?") \
X("Flip Note Types") \
X("Toggle Note Sizes") \
X("Expand Items") \
X("Compress Items") \
X("2:1 (8th to 4th)") \
X("3:2 (12th to 8th)") \
X("4:3 (16th to 12th)") \
X("1:2 (4th to 8th)") \
X("2:3 (8th to 12th)") \
X("3:4 (12th to 16th)") \
X("Toggle VSync") \
X("Toggle Fullscreen") \
X("Window Size") \
X("Resize to") \
X("Current Size") \
X("DPI Scale") \
X("Zoom In") \
X("Zoom Out") \
X("Reset Zoom") \
X(" Reset ") \
X("Current Scale") \
X("Test Menu") \
X("Show Audio Test") \
X("Show TJA Import Test") \
X("Show TJA Export View") \
X("Show ImGui Demo") \
X("Show ImGui Style Editor") \
X("Reset Style Colors") \
X("Copyright (c) 2022") \
X("Build Time:") \
X("Build Date:") \
X("Build Configuration:") \
X("Current Version:") \
X("Debug") \
X("Release") \
X("Courses") \
X("Add New") \
X("Edit...") \
X("Open Audio Device") \
X("Close Audio Device") \
X("Average: ") \
X("Min: ") \
X("Max: ") \
X("Use %s") \
X("Peepo Drum Kit - Unsaved Changes") \
X("Save changes to the current file?") \
X("Save Changes") \
X("Discard Changes") \
X("Cancel") \
X("Events") \
X("Tempo") \
X("Time Signature") \
X("Notes") \
X("Notes (Expert)") \
X("Notes (Master)") \
X("Scroll Speed") \
X("Vertical Scroll Speed") \
X("Scroll Speed Tempo") \
X("Bar Line Visibility") \
X("Go-Go Time") \
X("Lyrics") \
X("Scroll Type") \
X("JPOS Scroll") \
X("JPOS Scroll Move") \
X("Vertical JPOS Scroll Move") \
X("JPOS Scroll Duration") \
X("Sync") \
X("Chart Duration") \
X("Song Demo Start") \
X("Song Offset") \
X("Selection to Scroll Changes") \
X("Set Cursor") \
X("Add") \
X("Remove") \
X("Clear") \
X("Set from Range Selection") \
X("Chart") \
X("Chart Title") \
X("Chart Subtitle") \
X("Chart Creator") \
X("Song File Name") \
X("Jacket File Name") \
X("Song Volume") \
X("Sound Effect Volume") \
X("Selected Course") \
X("Difficulty Type") \
X("Difficulty Level") \
X("Difficulty Level Decimal") \
X("Course Creator") \
X("Selected Items") \
X("( Nothing Selected )") \
X("Selected ") \
X("Items") \
X("Tempos") \
X("Time Signatures") \
X("Scroll Speeds") \
X("Bar Lines") \
X("Go-Go Ranges") \
X("Bar Line Visible") \
X("Visible") \
X("Hidden") \
X("Scroll Types") \
X("JPOS Scrolls") \
X("NMSCROLL") \
X("HBSCROLL") \
X("BMSCROLL") \
X("Balloon Pop Count") \
X("Interpolate: Scroll Speed") \
X("Interpolate: Scroll Speed Tempo") \
X("Interpolate: Vertical Scroll Speed") \
X("Time Offset") \
X("Note Type") \
X("Note Type Size") \
X("Don") \
X("DON") \
X("Ka") \
X("KA") \
X("Drumroll") \
X("DRUMROLL") \
X("Balloon") \
X("BALLOON") \
X("Small") \
X("Big") \
X("Easy") \
X("Normal") \
X("Hard") \
X("Oni") \
X("Oni-Ura") \
X("Single") \
X("Double") \
X("Description") \
X("Time") \
X("Initial State") \
X("Lyrics Overview") \
X("Edit Line") \
X("(No Lyrics)") \
X("Reset") \
X("Tap") \
X(" First Beat ") \
X("Nearest Whole") \
X("Nearest") \
X("Min and Max") \
X("Timing Taps") \
X("First Beat") \
X("%d Taps") \
X("") \
X("KADON") \
X("Bomb") \
X("Adlib") \
X("Fuseroll") \
X("Load Full CJKV Glyphs (slow)") \
/* empty last line */

#define UI_Str(in) i18n::HashToString(i18n::CompileTimeValidate<i18n::Hash(in)>())
#define UI_StrRuntime(in) i18n::HashToString(i18n::Hash(in))
#define UI_WindowName(in) i18n::ToStableName(in, i18n::CompileTimeValidate<i18n::Hash(in)>()).Data

namespace PeepoDrumKit
{
	inline std::string SelectedGuiLanguage = std::string("en");
}

namespace PeepoDrumKit::i18n
{
	struct LocaleEntry {
		std::string id;
		std::string name;
	};
	extern std::vector<LocaleEntry> LocaleEntries;

	void InitBuiltinLocale();
	void RefreshLocales();
	void ReloadLocaleFile(cstr languageId = "en");
	void ExportBuiltinLocaleFiles();
	cstr HashToString(u32 inHash);

	constexpr ImU32 Crc32LookupTable[256] =
	{
		0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
		0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
		0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
		0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
		0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
		0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
		0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
		0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
		0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
		0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
		0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
		0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
		0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
		0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
		0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
		0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D,
	};

	constexpr u32 Crc32(const char* data, size_t dataSize, u32 seed)
	{
		seed = ~seed; u32 crc = seed;
		while (dataSize-- != 0)
			crc = (crc >> 8) ^ Crc32LookupTable[(crc & 0xFF) ^ static_cast<u8>(*data++)];
		return ~crc;
	}

	constexpr u32 Hash(std::string_view data, u32 seed = 0xDEADBEEF) { return Crc32(data.data(), data.size(), seed); }

	constexpr u32 AllValidHashes[] =
	{
#define X(en) Hash(en),
			PEEPODRUMKIT_UI_STRINGS_X_MACRO_LIST_EN
#undef X
	};

	constexpr b8 IsValidHash(u32 inHash) { for (u32 it : AllValidHashes) { if (it == inHash) return true; } return false; }

	template <u32 InHash>
	constexpr u32 CompileTimeValidate() { static_assert(IsValidHash(InHash), "Unknown string"); return InHash; }

	cstr HashToString(u32 inHash);

	struct StableNameBuffer { char Data[128]; };
	inline StableNameBuffer ToStableName(cstr inString, u32 inHash)
	{
		cstr translatedString = HashToString(inHash);
		StableNameBuffer buffer;

		char* out = &buffer.Data[0];
		*out = '\0';
		while (*translatedString != '\0')
			*out++ = *translatedString++;
		*out++ = '#';
		*out++ = '#';
		*out++ = '#';
		while (*inString != '\0')
			*out++ = *inString++;
		*out = '\0';

		// strcpy_s(out.Data, translatedString);
		// strcat_s(out.Data, "###");
		// strcat_s(out.Data, inString);

		return buffer;
	}
}
