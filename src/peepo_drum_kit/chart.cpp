#include "chart.h"
#include "core_build_info.h"
#include <algorithm>

namespace PeepoDrumKit
{
	void DebugCompareCharts(const ChartProject& chartA, const ChartProject& chartB, DebugCompareChartsOnMessageFunc onMessageFunc, void* userData)
	{
		auto logf = [onMessageFunc, userData](cstr fmt, ...)
		{
			char buffer[512];
			va_list args;
			va_start(args, fmt);
			onMessageFunc(std::string_view(buffer, _vsnprintf_s(buffer, ArrayCount(buffer), fmt, args)), userData);
			va_end(args);
		};

		if (chartA.Courses.size() != chartB.Courses.size()) { logf("Course count mismatch (%zu != %zu)", chartA.Courses.size(), chartB.Courses.size()); return; }

		for (size_t i = 0; i < chartA.Courses.size(); i++)
		{
			const ChartCourse& courseA = *chartA.Courses[i];
			const ChartCourse& courseB = *chartB.Courses[i];

			for (GenericList list = {}; list < GenericList::Count; IncrementEnum(list))
			{
				const size_t countA = GetGenericListCount(courseA, list);
				const size_t countB = GetGenericListCount(courseB, list);
				if (countA != countB) { logf("%s count mismatch (%zu != %zu)", GenericListNames[EnumToIndex(list)], countA, countB); continue; }

				for (size_t itemIndex = 0; itemIndex < countA; itemIndex++)
				{
					for (GenericMember member = {}; member < GenericMember::Count; IncrementEnum(member))
					{
						GenericMemberUnion valueA {}, valueB {};
						const b8 hasValueA = TryGetGeneric(courseA, list, itemIndex, member, valueA);
						const b8 hasValueB = TryGetGeneric(courseB, list, itemIndex, member, valueB);
						assert(hasValueA == hasValueB);
						if (!hasValueA || member == GenericMember::B8_IsSelected)
							continue;

						static constexpr auto safeCStrAreSame = [](cstr a, cstr b) -> b8 { if ((a == nullptr) || (b == nullptr) && a != b) return false; return (strcmp(a, b) == 0); };
						cstr memberName = ""; b8 isSame = false;
						switch (member)
						{
						case GenericMember::B8_IsSelected: { memberName = "IsSelected"; isSame = (valueA.B8 == valueB.B8); } break;
						case GenericMember::B8_BarLineVisible: { memberName = "IsVisible"; isSame = (valueA.B8 == valueB.B8); } break;
						case GenericMember::I16_BalloonPopCount: { memberName = "BalloonPopCount"; isSame = (valueA.I16 == valueB.I16); } break;
						case GenericMember::F32_ScrollSpeed: { memberName = "ScrollSpeed"; isSame = ApproxmiatelySame(valueA.CPX, valueB.CPX); } break;
						case GenericMember::Beat_Start: { memberName = "BeatStart"; isSame = (valueA.Beat == valueB.Beat); } break;
						case GenericMember::Beat_Duration: { memberName = "BeatDuration"; isSame = (valueA.Beat == valueB.Beat); } break;
						case GenericMember::Time_Offset: { memberName = "TimeOffset"; isSame = ApproxmiatelySame(valueA.Time.Seconds, valueB.Time.Seconds); } break;
						case GenericMember::NoteType_V: { memberName = "NoteType"; isSame = (valueA.NoteType == valueB.NoteType); } break;
						case GenericMember::Tempo_V: { memberName = "Tempo"; isSame = ApproxmiatelySame(valueA.Tempo.BPM, valueB.Tempo.BPM); } break;
						case GenericMember::TimeSignature_V: { memberName = "TimeSignature"; isSame = (valueA.TimeSignature == valueB.TimeSignature); } break;
						case GenericMember::CStr_Lyric: { memberName = "Lyric"; isSame = safeCStrAreSame(valueA.CStr, valueB.CStr); } break;
						case GenericMember::I8_ScrollType: { memberName = "ScrollType"; isSame = (valueA.I16 == valueB.I16); } break;
						}

						if (!isSame)
							logf("%s[%zu].%s value mismatch", GenericListNames[EnumToIndex(list)], itemIndex, memberName);
					}
				}
			}
		}
	}

	struct TempTimedDelayCommand { Beat Beat; Time Delay; };
	static constexpr Beat GetBeat(const TempTimedDelayCommand& v) { return v.Beat; }

	static constexpr NoteType ConvertTJANoteType(TJA::NoteType tjaNoteType)
	{
		switch (tjaNoteType)
		{
		case TJA::NoteType::None: return NoteType::Count;
		case TJA::NoteType::Don: return NoteType::Don;
		case TJA::NoteType::Ka: return NoteType::Ka;
		case TJA::NoteType::DonBig: return NoteType::DonBig;
		case TJA::NoteType::KaBig: return NoteType::KaBig;
		case TJA::NoteType::Start_Drumroll: return NoteType::Drumroll;
		case TJA::NoteType::Start_DrumrollBig: return NoteType::DrumrollBig;
		case TJA::NoteType::Start_Balloon: return NoteType::Balloon;
		case TJA::NoteType::End_BalloonOrDrumroll: return NoteType::Count;
		case TJA::NoteType::Start_BaloonSpecial: return NoteType::BalloonSpecial;
		case TJA::NoteType::DonBigBoth: return NoteType::Count;
		case TJA::NoteType::KaBigBoth: return NoteType::Count;
		case TJA::NoteType::Hidden: return NoteType::Adlib;
		case TJA::NoteType::Bomb: return NoteType::Bomb;
		case TJA::NoteType::KaDon: return NoteType::KaDon;
		case TJA::NoteType::Fuse: return NoteType::Fuse;
		default: return NoteType::Count;
		}
	}

	static constexpr TJA::NoteType ConvertTJANoteType(NoteType noteType)
	{
		switch (noteType)
		{
		case NoteType::Don: return TJA::NoteType::Don;
		case NoteType::DonBig: return TJA::NoteType::DonBig;
		case NoteType::Ka: return TJA::NoteType::Ka;
		case NoteType::KaBig: return TJA::NoteType::KaBig;
		case NoteType::Drumroll: return TJA::NoteType::Start_Drumroll;
		case NoteType::DrumrollBig: return TJA::NoteType::Start_DrumrollBig;
		case NoteType::Balloon: return TJA::NoteType::Start_Balloon;
		case NoteType::BalloonSpecial: return TJA::NoteType::Start_BaloonSpecial;
		case NoteType::KaDon: return TJA::NoteType::KaDon;
		case NoteType::Adlib: return TJA::NoteType::Hidden;
		case NoteType::Fuse: return TJA::NoteType::Fuse;
		case NoteType::Bomb: return TJA::NoteType::Bomb;
		default: return TJA::NoteType::None;
		}
	}

	Beat FindCourseMaxUsedBeat(const ChartCourse& course)
	{
		// NOTE: Technically only need to look at the last item of each sorted list **but just to be sure**, in case there is something wonky going on with out-of-order durations or something
		Beat maxBeat = Beat::Zero();
		for (const auto& v : course.TempoMap.Tempo) maxBeat = Max(maxBeat, v.Beat);
		for (const auto& v : course.TempoMap.Signature) maxBeat = Max(maxBeat, v.Beat);
		for (size_t i = 0; i < EnumCount<BranchType>; i++)
			for (const auto& v : course.GetNotes(static_cast<BranchType>(i))) maxBeat = Max(maxBeat, v.BeatTime + Max(Beat::Zero(), v.BeatDuration));
		for (const auto& v : course.GoGoRanges) maxBeat = Max(maxBeat, v.BeatTime + Max(Beat::Zero(), v.BeatDuration));
		for (const auto& v : course.ScrollChanges) maxBeat = Max(maxBeat, v.BeatTime);
		for (const auto& v : course.BarLineChanges) maxBeat = Max(maxBeat, v.BeatTime);
		for (const auto& v : course.Lyrics) maxBeat = Max(maxBeat, v.BeatTime);
		return maxBeat;
	}

	b8 CreateChartProjectFromTJA(const TJA::ParsedTJA& inTJA, ChartProject& out)
	{
		out.ChartDuration = Time::Zero();
		out.ChartTitle[Language::Base] = inTJA.Metadata.TITLE;
		out.ChartTitle[Language::JA] = inTJA.Metadata.TITLE_JA;
		out.ChartTitle[Language::EN] = inTJA.Metadata.TITLE_EN;
		out.ChartTitle[Language::CN] = inTJA.Metadata.TITLE_CN;
		out.ChartTitle[Language::TW] = inTJA.Metadata.TITLE_TW;
		out.ChartTitle[Language::KO] = inTJA.Metadata.TITLE_KO;
		out.ChartSubtitle[Language::Base] = inTJA.Metadata.SUBTITLE;
		out.ChartSubtitle[Language::JA] = inTJA.Metadata.SUBTITLE_JA;
		out.ChartSubtitle[Language::EN] = inTJA.Metadata.SUBTITLE_EN;
		out.ChartSubtitle[Language::CN] = inTJA.Metadata.SUBTITLE_CN;
		out.ChartSubtitle[Language::TW] = inTJA.Metadata.SUBTITLE_TW;
		out.ChartSubtitle[Language::KO] = inTJA.Metadata.SUBTITLE_KO;
		out.ChartCreator = inTJA.Metadata.MAKER;
		out.ChartGenre = inTJA.Metadata.GENRE;
		out.ChartLyricsFileName = inTJA.Metadata.LYRICS;
		out.SongOffset = inTJA.Metadata.OFFSET;
		out.SongDemoStartTime = inTJA.Metadata.DEMOSTART;
		out.SongFileName = inTJA.Metadata.WAVE;
		out.SongJacket = inTJA.Metadata.PREIMAGE;
		out.SongVolume = inTJA.Metadata.SONGVOL;
		out.SoundEffectVolume = inTJA.Metadata.SEVOL;
		out.BackgroundImageFileName = inTJA.Metadata.BGIMAGE;
		out.BackgroundMovieFileName = inTJA.Metadata.BGMOVIE;
		out.MovieOffset = inTJA.Metadata.MOVIEOFFSET;
		for (size_t i = 0; i < inTJA.Courses.size(); i++)
		{
			const TJA::ConvertedCourse& inCourse = TJA::ConvertParsedToConvertedCourse(inTJA, inTJA.Courses[i]);
			ChartCourse& outCourse = *out.Courses.emplace_back(std::make_unique<ChartCourse>());

			// HACK: Write proper enum conversion functions
			outCourse.Type = Clamp(static_cast<DifficultyType>(inCourse.CourseMetadata.COURSE), DifficultyType {}, DifficultyType::Count);
			outCourse.Level = Clamp(static_cast<DifficultyLevel>(inCourse.CourseMetadata.LEVEL), DifficultyLevel::Min, DifficultyLevel::Max);
			outCourse.Decimal = Clamp(static_cast<DifficultyLevelDecimal>(inCourse.CourseMetadata.LEVEL_DECIMALTAG), DifficultyLevelDecimal::None, DifficultyLevelDecimal::Max);
			outCourse.CourseCreator = inCourse.CourseMetadata.NOTESDESIGNER;

			outCourse.Life = Clamp(static_cast<TowerLives>(inCourse.CourseMetadata.LIFE), TowerLives::Min, TowerLives::Max);
			outCourse.Side = Clamp(static_cast<Side>(inCourse.CourseMetadata.SIDE), Side{}, Side::Count);

			outCourse.TempoMap.Tempo.Sorted = { TempoChange(Beat::Zero(), inTJA.Metadata.BPM) };
			outCourse.TempoMap.Signature.Sorted = { TimeSignatureChange(Beat::Zero(), TimeSignature(4, 4)) };
			TimeSignature lastSignature = TimeSignature(4, 4);

			i32 currentBalloonIndex = 0;

			BeatSortedList<TempTimedDelayCommand> tempSortedDelayCommands;
			BeatSortedForwardIterator<TempTimedDelayCommand> tempDelayCommandsIt;
			for (const TJA::ConvertedMeasure& inMeasure : inCourse.Measures)
			{
				for (const TJA::ConvertedDelayChange& inDelayChange : inMeasure.DelayChanges)
					tempSortedDelayCommands.InsertOrUpdate(TempTimedDelayCommand { inMeasure.StartTime + inDelayChange.TimeWithinMeasure, inDelayChange.Delay });
			}

			for (const TJA::ConvertedMeasure& inMeasure : inCourse.Measures)
			{
				for (const TJA::ConvertedNote& inNote : inMeasure.Notes)
				{
					if (inNote.Type == TJA::NoteType::End_BalloonOrDrumroll)
					{
						// TODO: Proper handling
						if (!outCourse.Notes_Normal.Sorted.empty())
							outCourse.Notes_Normal.Sorted.back().BeatDuration = (inMeasure.StartTime + inNote.TimeWithinMeasure) - outCourse.Notes_Normal.Sorted.back().BeatTime;
						continue;
					}

					const NoteType outNoteType = ConvertTJANoteType(inNote.Type);
					if (outNoteType == NoteType::Count)
						continue;

					Note& outNote = outCourse.Notes_Normal.Sorted.emplace_back();
					outNote.BeatTime = (inMeasure.StartTime + inNote.TimeWithinMeasure);
					outNote.Type = outNoteType;

					const TempTimedDelayCommand* delayCommandForThisNote = tempDelayCommandsIt.Next(tempSortedDelayCommands.Sorted, outNote.BeatTime);
					outNote.TimeOffset = (delayCommandForThisNote != nullptr) ? delayCommandForThisNote->Delay : Time::Zero();

					if (inNote.Type == TJA::NoteType::Start_Balloon || inNote.Type == TJA::NoteType::Start_BaloonSpecial || inNote.Type == TJA::NoteType::Fuse)
					{
						// TODO: Implement properly with correct branch handling
						if (InBounds(currentBalloonIndex, inCourse.CourseMetadata.BALLOON))
							outNote.BalloonPopCount = inCourse.CourseMetadata.BALLOON[currentBalloonIndex];
						currentBalloonIndex++;
					}
				}

				if (inMeasure.TimeSignature != lastSignature)
				{
					outCourse.TempoMap.Signature.InsertOrUpdate(TimeSignatureChange(inMeasure.StartTime, inMeasure.TimeSignature));
					lastSignature = inMeasure.TimeSignature;
				}

				for (const TJA::ConvertedTempoChange& inTempoChange : inMeasure.TempoChanges)
					outCourse.TempoMap.Tempo.InsertOrUpdate(TempoChange(inMeasure.StartTime + inTempoChange.TimeWithinMeasure, inTempoChange.Tempo));

				for (const TJA::ConvertedScrollChange& inScrollChange : inMeasure.ScrollChanges)
					outCourse.ScrollChanges.Sorted.push_back(ScrollChange { (inMeasure.StartTime + inScrollChange.TimeWithinMeasure), inScrollChange.ScrollSpeed });

				for (const TJA::ConvertedScrollType& inScrollType : inMeasure.ScrollTypes)
					outCourse.ScrollTypes.Sorted.push_back(ScrollType{ (inMeasure.StartTime + inScrollType.TimeWithinMeasure),  static_cast<ScrollMethod>(inScrollType.Method) });

				for (const TJA::ConvertedJPOSScroll& inJPOSScrollChange : inMeasure.JPOSScrollChanges)
					outCourse.JPOSScrollChanges.Sorted.push_back(JPOSScrollChange{ (inMeasure.StartTime + inJPOSScrollChange.TimeWithinMeasure), inJPOSScrollChange.Move, inJPOSScrollChange.Duration });


				for (const TJA::ConvertedBarLineChange& barLineChange : inMeasure.BarLineChanges)
					outCourse.BarLineChanges.Sorted.push_back(BarLineChange { (inMeasure.StartTime + barLineChange.TimeWithinMeasure), barLineChange.Visibile });

				for (const TJA::ConvertedLyricChange& lyricChange : inMeasure.LyricChanges)
					outCourse.Lyrics.Sorted.push_back(LyricChange { (inMeasure.StartTime + lyricChange.TimeWithinMeasure), lyricChange.Lyric });
			}

			for (const TJA::ConvertedGoGoRange& inGoGoRange : inCourse.GoGoRanges)
				outCourse.GoGoRanges.Sorted.push_back(GoGoRange { inGoGoRange.StartTime, (inGoGoRange.EndTime - inGoGoRange.StartTime) });

			//outCourse.TempoMap.SetTempoChange(TempoChange());
			//outCourse.TempoMap = inCourse.GoGoRanges;

			outCourse.ScoreInit = inCourse.CourseMetadata.SCOREINIT;
			outCourse.ScoreDiff = inCourse.CourseMetadata.SCOREDIFF;

			outCourse.TempoMap.RebuildAccelerationStructure();

			if (!inCourse.Measures.empty())
				out.ChartDuration = Max(out.ChartDuration, outCourse.TempoMap.BeatToTime(inCourse.Measures.back().StartTime /*+ inCourse.Measures.back().TimeSignature.GetDurationPerBar()*/));
		}

		return true;
	}

	b8 ConvertChartProjectToTJA(const ChartProject& in, TJA::ParsedTJA& out, b8 includePeepoDrumKitComment)
	{
		static constexpr cstr FallbackTJAChartTitle = "Untitled Chart";
		out.Metadata.TITLE = !in.ChartTitle[Language::Base].empty() ? in.ChartTitle[Language::Base] : FallbackTJAChartTitle;
		out.Metadata.TITLE_JA = in.ChartTitle[Language::JA];
		out.Metadata.TITLE_EN = in.ChartTitle[Language::EN];
		out.Metadata.TITLE_CN = in.ChartTitle[Language::CN];
		out.Metadata.TITLE_TW = in.ChartTitle[Language::TW];
		out.Metadata.TITLE_KO = in.ChartTitle[Language::KO];
		out.Metadata.SUBTITLE = in.ChartSubtitle[Language::Base];
		out.Metadata.SUBTITLE_JA = in.ChartSubtitle[Language::JA];
		out.Metadata.SUBTITLE_EN = in.ChartSubtitle[Language::EN];
		out.Metadata.SUBTITLE_CN = in.ChartSubtitle[Language::CN];
		out.Metadata.SUBTITLE_TW = in.ChartSubtitle[Language::TW];
		out.Metadata.SUBTITLE_KO = in.ChartSubtitle[Language::KO];
		out.Metadata.MAKER = in.ChartCreator;
		out.Metadata.GENRE = in.ChartGenre;
		out.Metadata.LYRICS = in.ChartLyricsFileName;
		out.Metadata.OFFSET = in.SongOffset;
		out.Metadata.DEMOSTART = in.SongDemoStartTime;
		out.Metadata.WAVE = in.SongFileName;
		out.Metadata.PREIMAGE = in.SongJacket;
		out.Metadata.SONGVOL = in.SongVolume;
		out.Metadata.SEVOL = in.SoundEffectVolume;
		out.Metadata.BGIMAGE = in.BackgroundImageFileName;
		out.Metadata.BGMOVIE = in.BackgroundMovieFileName;
		out.Metadata.MOVIEOFFSET = in.MovieOffset;

		if (includePeepoDrumKitComment)
		{
			out.HasPeepoDrumKitComment = true;
			out.PeepoDrumKitCommentDate = BuildInfo::CompilationDateParsed;
		}

		if (!in.Courses.empty())
		{
			if (!in.Courses[0]->TempoMap.Tempo.empty())
			{
				const TempoChange* initialTempo = in.Courses[0]->TempoMap.Tempo.TryFindLastAtBeat(Beat::Zero());
				out.Metadata.BPM = (initialTempo != nullptr) ? initialTempo->Tempo : FallbackTempo;
			}
		}

		out.Courses.reserve(in.Courses.size());
		for (const std::unique_ptr<ChartCourse>& inCourseIt : in.Courses)
		{
			const ChartCourse& inCourse = *inCourseIt;
			TJA::ParsedCourse& outCourse = out.Courses.emplace_back();

			// HACK: Write proper enum conversion functions
			outCourse.Metadata.COURSE = static_cast<TJA::DifficultyType>(inCourse.Type);
			outCourse.Metadata.LEVEL = static_cast<i32>(inCourse.Level);
			outCourse.Metadata.LEVEL_DECIMALTAG = static_cast<i32>(inCourse.Decimal);
			outCourse.Metadata.NOTESDESIGNER = inCourse.CourseCreator;
			for (const Note& inNote : inCourse.Notes_Normal) if (IsBalloonNote(inNote.Type)) { outCourse.Metadata.BALLOON.push_back(inNote.BalloonPopCount); }
			outCourse.Metadata.SCOREINIT = inCourse.ScoreInit;
			outCourse.Metadata.SCOREDIFF = inCourse.ScoreDiff;

			outCourse.Metadata.LIFE = static_cast<i32>(inCourse.Life);
			outCourse.Metadata.SIDE = static_cast<TJA::SongSelectSide>(inCourse.Side);

			// TODO: Is this implemented correctly..? Need to have enough measures to cover every note/command and pad with empty measures up to the chart duration
			// BUG: NOPE! "07 �Q�[���~���[�W�b�N/003D. MagiCatz/MagiCatz.tja" for example still gets rounded up and then increased by a measure each time it gets saved
			// ... and even so does "Heat Haze Shadow 2.tja" without any weird time signatures..??
			const Beat inChartMaxUsedBeat = FindCourseMaxUsedBeat(inCourse);
			const Beat inChartBeatDuration = inCourse.TempoMap.TimeToBeat(in.GetDurationOrDefault());
			std::vector<TJA::ConvertedMeasure> outConvertedMeasures;

			inCourse.TempoMap.ForEachBeatBar([&](const SortedTempoMap::ForEachBeatBarData& it)
			{
				if (inChartBeatDuration > inChartMaxUsedBeat && (it.Beat >= inChartBeatDuration))
					return ControlFlow::Break;
				if (it.IsBar)
				{
					TJA::ConvertedMeasure& outConvertedMeasure = outConvertedMeasures.emplace_back();
					outConvertedMeasure.StartTime = it.Beat;
					outConvertedMeasure.TimeSignature = it.Signature;
				}
				return (it.Beat >= Max(inChartBeatDuration, inChartMaxUsedBeat)) ? ControlFlow::Break : ControlFlow::Continue;
			});

			if (outConvertedMeasures.empty())
				outConvertedMeasures.push_back(TJA::ConvertedMeasure { Beat::Zero(), TimeSignature(4, 4) });

			static constexpr auto tryFindMeasureForBeat = [](std::vector<TJA::ConvertedMeasure>& measures, Beat beatToFind) -> TJA::ConvertedMeasure*
			{
				static constexpr auto isMoreBeat = [](const TJA::ConvertedMeasure& lhs, const TJA::ConvertedMeasure& rhs)
				{
					return lhs.StartTime > rhs.StartTime;
				};
				// Binary search in descending (ascending but reversed) list
				// if found: `it` is the last element such that `beatToFind >= it->StartTime`
				auto it = std::lower_bound(measures.rbegin(), measures.rend(), TJA::ConvertedMeasure { beatToFind }, isMoreBeat);
				return (it == measures.rend()) ? nullptr : &*it;
			};

			for (const TempoChange& inTempoChange : inCourse.TempoMap.Tempo)
			{
				if (!(&inTempoChange == &inCourse.TempoMap.Tempo[0] && inTempoChange.Tempo.BPM == out.Metadata.BPM.BPM))
				{
					TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, inTempoChange.Beat);
					if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
						outConvertedMeasure->TempoChanges.push_back(TJA::ConvertedTempoChange { (inTempoChange.Beat - outConvertedMeasure->StartTime), inTempoChange.Tempo });
				}
			}

			Time lastNoteTimeOffset = Time::Zero();
			for (const Note& inNote : inCourse.Notes_Normal)
			{
				TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, inNote.BeatTime);
				if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
					outConvertedMeasure->Notes.push_back(TJA::ConvertedNote { (inNote.BeatTime - outConvertedMeasure->StartTime), ConvertTJANoteType(inNote.Type) });

				if (inNote.BeatDuration > Beat::Zero())
				{
					TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, inNote.BeatTime + inNote.BeatDuration);
					if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
						outConvertedMeasure->Notes.push_back(TJA::ConvertedNote { ((inNote.BeatTime + inNote.BeatDuration) - outConvertedMeasure->StartTime), TJA::NoteType::End_BalloonOrDrumroll });
				}

				const Time thisNoteTimeOffset = ApproxmiatelySame(inNote.TimeOffset.Seconds, 0.0) ? Time::Zero() : inNote.TimeOffset;
				if (thisNoteTimeOffset != lastNoteTimeOffset)
				{
					outConvertedMeasure->DelayChanges.push_back(TJA::ConvertedDelayChange { (inNote.BeatTime - outConvertedMeasure->StartTime), thisNoteTimeOffset });
					lastNoteTimeOffset = thisNoteTimeOffset;
				}
			}

			for (const ScrollChange& inScroll : inCourse.ScrollChanges)
			{
				TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, inScroll.BeatTime);
				if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
					outConvertedMeasure->ScrollChanges.push_back(TJA::ConvertedScrollChange { (inScroll.BeatTime - outConvertedMeasure->StartTime), inScroll.ScrollSpeed });
			}

			for (const ScrollType& inScrollType : inCourse.ScrollTypes)
			{
				TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, inScrollType.BeatTime);
				if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
					outConvertedMeasure->ScrollTypes.push_back(TJA::ConvertedScrollType { (inScrollType.BeatTime - outConvertedMeasure->StartTime), static_cast<i8>(inScrollType.Method) });
			}

			for (const JPOSScrollChange& JPOSScroll : inCourse.JPOSScrollChanges)
			{
				TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, JPOSScroll.BeatTime);
				if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
					outConvertedMeasure->JPOSScrollChanges.push_back(TJA::ConvertedJPOSScroll { (JPOSScroll.BeatTime - outConvertedMeasure->StartTime), JPOSScroll.Move, JPOSScroll.Duration });
			}

			for (const BarLineChange& barLineChange : inCourse.BarLineChanges)
			{
				TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, barLineChange.BeatTime);
				if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
					outConvertedMeasure->BarLineChanges.push_back(TJA::ConvertedBarLineChange { (barLineChange.BeatTime - outConvertedMeasure->StartTime), barLineChange.IsVisible });
			}

			for (const LyricChange& inLyric : inCourse.Lyrics)
			{
				TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, inLyric.BeatTime);
				if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
					outConvertedMeasure->LyricChanges.push_back(TJA::ConvertedLyricChange { (inLyric.BeatTime - outConvertedMeasure->StartTime), inLyric.Lyric });
			}

			// For go-go time events, convert each range to a pair of start & end changes
			for (const GoGoRange& gogo : inCourse.GoGoRanges)
			{
				// start
				TJA::ConvertedMeasure* outConvertedMeasureStart = tryFindMeasureForBeat(outConvertedMeasures, gogo.BeatTime);
				if (assert(outConvertedMeasureStart != nullptr); outConvertedMeasureStart != nullptr)
					outConvertedMeasureStart->GoGoChanges.push_back(TJA::ConvertedGoGoChange{ (gogo.BeatTime - outConvertedMeasureStart->StartTime), true });
				// end
				const Beat endTime = gogo.BeatTime + Max(Beat::Zero(), gogo.BeatDuration);
				TJA::ConvertedMeasure* outConvertedMeasureEnd = tryFindMeasureForBeat(outConvertedMeasures, endTime);
				if (assert(outConvertedMeasureEnd != nullptr); outConvertedMeasureEnd != nullptr)
					outConvertedMeasureEnd->GoGoChanges.push_back(TJA::ConvertedGoGoChange{ (endTime - outConvertedMeasureEnd->StartTime), false });
			}

			TJA::ConvertConvertedMeasuresToParsedCommands(outConvertedMeasures, outCourse.ChartCommands);
		}

		return true;
	}
}

namespace PeepoDrumKit
{
	struct BeatStartAndDurationPtrs { Beat* Start; Beat* Duration; };
	inline BeatStartAndDurationPtrs GetGenericListStructRawBeatPtr(GenericListStruct& in, GenericList list)
	{
		switch (list)
		{
		case GenericList::TempoChanges: return { &in.POD.Tempo.Beat, nullptr };
		case GenericList::SignatureChanges: return { &in.POD.Signature.Beat, nullptr };
		case GenericList::Notes_Normal: return { &in.POD.Note.BeatTime, &in.POD.Note.BeatDuration };
		case GenericList::Notes_Expert: return { &in.POD.Note.BeatTime, &in.POD.Note.BeatDuration };
		case GenericList::Notes_Master: return { &in.POD.Note.BeatTime, &in.POD.Note.BeatDuration };
		case GenericList::ScrollChanges: return { &in.POD.Scroll.BeatTime, nullptr };
		case GenericList::BarLineChanges: return { &in.POD.BarLine.BeatTime, nullptr };
		case GenericList::GoGoRanges: return { &in.POD.GoGo.BeatTime, &in.POD.GoGo.BeatDuration };
		case GenericList::Lyrics: return { &in.NonTrivial.Lyric.BeatTime, nullptr };
		case GenericList::ScrollType: return { &in.POD.ScrollType.BeatTime, nullptr };
		case GenericList::JPOSScroll: return { &in.POD.JPOSScroll.BeatTime, nullptr };
		default: assert(false); return { nullptr, nullptr };
		}
	}

	inline f32* GetGenericListStructRawTimeDurationPtr(GenericListStruct& in, GenericList list)
	{
		switch (list)
		{
		case GenericList::TempoChanges:
		case GenericList::SignatureChanges:
		case GenericList::Notes_Normal:
		case GenericList::Notes_Expert:
		case GenericList::Notes_Master:
		case GenericList::ScrollChanges:
		case GenericList::BarLineChanges:
		case GenericList::GoGoRanges:
		case GenericList::Lyrics:
		case GenericList::ScrollType:
			return nullptr;
		case GenericList::JPOSScroll: return &in.POD.JPOSScroll.Duration;
		default: assert(false); return nullptr;
		}
	}

	Beat GenericListStruct::GetBeat(GenericList list) const
	{
		return *GetGenericListStructRawBeatPtr(*const_cast<GenericListStruct*>(this), list).Start;
	}

	Beat GenericListStruct::GetBeatDuration(GenericList list) const
	{
		const auto ptrs = GetGenericListStructRawBeatPtr(*const_cast<GenericListStruct*>(this), list);
		return (ptrs.Duration != nullptr) ? *ptrs.Duration : Beat::Zero();
	}

	std::tuple<bool, Time> GenericListStruct::GetTimeDuration(GenericList list) const
	{
		const auto ptrs = GetGenericListStructRawTimeDurationPtr(*const_cast<GenericListStruct*>(this), list);
		return (ptrs != nullptr) ? std::make_tuple(true, Time::FromSec(*ptrs)) : std::make_tuple(false, Time::Zero());
	}

	void GenericListStruct::SetBeat(GenericList list, Beat newValue)
	{
		*GetGenericListStructRawBeatPtr(*this, list).Start = newValue;
	}

	void GenericListStruct::SetBeatDuration(GenericList list, Beat newValue)
	{
		auto ptrs = GetGenericListStructRawBeatPtr(*this, list);
		if (ptrs.Duration != nullptr)
			*ptrs.Duration = newValue;
	}

	void GenericListStruct::SetTimeDuration(GenericList list, Time newValue)
	{
		auto ptrs = GetGenericListStructRawTimeDurationPtr(*this, list);
		if (ptrs != nullptr)
			*ptrs = newValue.Seconds;
	}

	size_t GetGenericMember_RawByteSize(GenericMember member)
	{
		switch (member)
		{
		case GenericMember::B8_IsSelected: return sizeof(b8);
		case GenericMember::B8_BarLineVisible: return sizeof(b8);
		case GenericMember::I16_BalloonPopCount: return sizeof(i16);
		case GenericMember::F32_ScrollSpeed: return sizeof(Complex);
		case GenericMember::Beat_Start: return sizeof(Beat);
		case GenericMember::Beat_Duration: return sizeof(Beat);
		case GenericMember::Time_Offset: return sizeof(Time);
		case GenericMember::NoteType_V: return sizeof(NoteType);
		case GenericMember::Tempo_V: return sizeof(Tempo);
		case GenericMember::TimeSignature_V: return sizeof(TimeSignature);
		case GenericMember::CStr_Lyric: return sizeof(cstr);
		case GenericMember::I8_ScrollType: return sizeof(i8);
		case GenericMember::F32_JPOSScroll: return sizeof(Complex);
		case GenericMember::F32_JPOSScrollDuration: return sizeof(f32);
		default: assert(false); return 0;
		}
	}

	size_t GetGenericListCount(const ChartCourse& course, GenericList list)
	{
		return ApplySingleGenericList<size_t>(list,
			[](auto&& typedList) { return typedList.size(); }, 0,
			course);
	}

	GenericMemberFlags GetAvailableMemberFlags(GenericList list)
	{
		switch (list)
		{
		case GenericList::TempoChanges:
			return GenericMemberFlags_IsSelected | GenericMemberFlags_Start | GenericMemberFlags_Tempo;
		case GenericList::SignatureChanges:
			return GenericMemberFlags_IsSelected | GenericMemberFlags_Start | GenericMemberFlags_TimeSignature;
		case GenericList::Notes_Normal:
		case GenericList::Notes_Expert:
		case GenericList::Notes_Master:
			return GenericMemberFlags_IsSelected | GenericMemberFlags_BalloonPopCount | GenericMemberFlags_Start | GenericMemberFlags_Duration | GenericMemberFlags_Offset | GenericMemberFlags_NoteType;
		case GenericList::ScrollChanges:
			return GenericMemberFlags_IsSelected | GenericMemberFlags_ScrollSpeed | GenericMemberFlags_Start;
		case GenericList::BarLineChanges:
			return GenericMemberFlags_IsSelected | GenericMemberFlags_BarLineVisible | GenericMemberFlags_Start;
		case GenericList::GoGoRanges:
			return GenericMemberFlags_IsSelected | GenericMemberFlags_Start | GenericMemberFlags_Duration;
		case GenericList::Lyrics:
			return GenericMemberFlags_IsSelected | GenericMemberFlags_Start | GenericMemberFlags_Lyric;
		case GenericList::ScrollType: 
			return GenericMemberFlags_IsSelected | GenericMemberFlags_ScrollType | GenericMemberFlags_Start;
		case GenericList::JPOSScroll: 
			return GenericMemberFlags_IsSelected | GenericMemberFlags_JPOSScroll | GenericMemberFlags_JPOSScrollDuration | GenericMemberFlags_Start;
		default:
			assert(false); return GenericMemberFlags_None;
		}
	}

	void* TryGetGeneric_RawVoidPtr(const ChartCourse& course, GenericList list, size_t index, GenericMember member)
	{
		switch (list)
		{
		case GenericList::TempoChanges:
			if (auto& vector = *const_cast<SortedTempoChangesList*>(&course.TempoMap.Tempo); index < vector.size())
			{
				switch (member)
				{
				case GenericMember::B8_IsSelected: return &vector[index].IsSelected;
				case GenericMember::Beat_Start: return &vector[index].Beat;
				case GenericMember::Tempo_V: return &vector[index].Tempo;
				}
			} break;
		case GenericList::SignatureChanges:
			if (auto& vector = *const_cast<SortedSignatureChangesList*>(&course.TempoMap.Signature); index < vector.size())
			{
				switch (member)
				{
				case GenericMember::B8_IsSelected: return &vector[index].IsSelected;
				case GenericMember::Beat_Start: return &vector[index].Beat;
				case GenericMember::TimeSignature_V: return &vector[index].Signature;
				}
			} break;
		case GenericList::Notes_Normal:
		case GenericList::Notes_Expert:
		case GenericList::Notes_Master:
		{
			auto& vector = *const_cast<SortedNotesList*>(&(
				list == GenericList::Notes_Normal ? course.Notes_Normal :
				list == GenericList::Notes_Expert ? course.Notes_Expert : course.Notes_Master));

			if (index < vector.size())
			{
				switch (member)
				{
				case GenericMember::B8_IsSelected: return &vector[index].IsSelected;
				case GenericMember::I16_BalloonPopCount: return &vector[index].BalloonPopCount;
				case GenericMember::Beat_Start: return &vector[index].BeatTime;
				case GenericMember::Beat_Duration: return &vector[index].BeatDuration;
				case GenericMember::Time_Offset: return &vector[index].TimeOffset;
				case GenericMember::NoteType_V: return &vector[index].Type;
				}
			}
		} break;
		case GenericList::ScrollChanges:
			if (auto& vector = *const_cast<SortedScrollChangesList*>(&course.ScrollChanges); index < vector.size())
			{
				switch (member)
				{
				case GenericMember::B8_IsSelected: return &vector[index].IsSelected;
				case GenericMember::F32_ScrollSpeed: return &vector[index].ScrollSpeed;
				case GenericMember::Beat_Start: return &vector[index].BeatTime;
				}
			} break;
		case GenericList::BarLineChanges:
			if (auto& vector = *const_cast<SortedBarLineChangesList*>(&course.BarLineChanges); index < vector.size())
			{
				switch (member)
				{
				case GenericMember::B8_IsSelected: return &vector[index].IsSelected;
				case GenericMember::B8_BarLineVisible: return &vector[index].IsVisible;
				case GenericMember::Beat_Start: return &vector[index].BeatTime;
				}
			} break;
		case GenericList::GoGoRanges:
			if (auto& vector = *const_cast<SortedGoGoRangesList*>(&course.GoGoRanges); index < vector.size())
			{
				switch (member)
				{
				case GenericMember::B8_IsSelected: return &vector[index].IsSelected;
				case GenericMember::Beat_Start: return &vector[index].BeatTime;
				case GenericMember::Beat_Duration: return &vector[index].BeatDuration;
				}
			} break;
		case GenericList::Lyrics:
			if (auto& vector = *const_cast<SortedLyricsList*>(&course.Lyrics); index < vector.size())
			{
				switch (member)
				{
				case GenericMember::B8_IsSelected: return &vector[index].IsSelected;
				case GenericMember::Beat_Start: return &vector[index].BeatTime;
				case GenericMember::CStr_Lyric: return vector[index].Lyric.data();
				}
			} break;
		case GenericList::ScrollType:
			if (auto& vector = *const_cast<SortedScrollTypesList*>(&course.ScrollTypes); index < vector.size())
			{
				switch (member)
				{
				case GenericMember::B8_IsSelected: return &vector[index].IsSelected;
				case GenericMember::I8_ScrollType: return &vector[index].Method;
				case GenericMember::Beat_Start: return &vector[index].BeatTime;
				}
			} break;
		case GenericList::JPOSScroll:
			if (auto& vector = *const_cast<SortedJPOSScrollChangesList*>(&course.JPOSScrollChanges); index < vector.size())
			{
				switch (member)
				{
				case GenericMember::B8_IsSelected: return &vector[index].IsSelected;
				case GenericMember::F32_JPOSScroll: return &vector[index].Move;
				case GenericMember::F32_JPOSScrollDuration: return &vector[index].Duration;
				case GenericMember::Beat_Start: return &vector[index].BeatTime;
				}
			} break;
		default:
			assert(false); break;
		}
		return nullptr;
	}

	b8 TryGetGeneric(const ChartCourse& course, GenericList list, size_t index, GenericMember member, GenericMemberUnion& outValue)
	{
		const void* voidMember = TryGetGeneric_RawVoidPtr(course, list, index, member);
		if (voidMember == nullptr)
			return false;

		if (member == GenericMember::CStr_Lyric)
			outValue.CStr = static_cast<cstr>(voidMember);
		else
			memcpy(&outValue, voidMember, GetGenericMember_RawByteSize(member));
		return true;
	}

	b8 TrySetGeneric(ChartCourse& course, GenericList list, size_t index, GenericMember member, const GenericMemberUnion& inValue)
	{
		void* voidMember = TryGetGeneric_RawVoidPtr(course, list, index, member);
		if (voidMember == nullptr)
			return false;

		if (member == GenericMember::CStr_Lyric)
			course.Lyrics[index].Lyric.assign(inValue.CStr);
		else
			memcpy(voidMember, &inValue, GetGenericMember_RawByteSize(member));
		return true;
	}

	b8 TryGetGenericStruct(const ChartCourse& course, GenericList list, size_t index, GenericListStruct& outValue)
	{
		return ApplySingleGenericList(list,
			[&](auto&& typedList, auto&& typedOutValue) { if (InBounds(index, typedList)) { typedOutValue = typedList[index]; return true; } return false; }, false,
			course, outValue);
	}

	b8 TrySetGenericStruct(ChartCourse& course, GenericList list, size_t index, const GenericListStruct& inValue)
	{
		return ApplySingleGenericList(list,
			[&](auto&& typedList, auto&& typedInValue) { if (InBounds(index, typedList)) { typedList[index] = typedInValue; return true; } return false; }, false,
			course, inValue);
	}

	b8 TryAddGenericStruct(ChartCourse& course, GenericList list, GenericListStruct inValue)
	{
		return ApplySingleGenericList(list,
			[&](auto&& typedList, auto&& typedInValue) { typedList.InsertOrUpdate(typedInValue); return true; }, false,
			course, std::move(inValue)); // `std::move` makes no differences on POD
	}

	b8 TryRemoveGenericStruct(ChartCourse& course, GenericList list, const GenericListStruct& inValueToRemove)
	{
		return TryRemoveGenericStruct(course, list, inValueToRemove.GetBeat(list));
	}

	b8 TryRemoveGenericStruct(ChartCourse& course, GenericList list, Beat beatToRemove)
	{
		return ApplySingleGenericList(list,
			[&](auto&& typedList) { typedList.RemoveAtBeat(beatToRemove); return true; }, false,
			course);
	}
}
