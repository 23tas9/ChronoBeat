#pragma once
#include "SemVer.hpp"
#include "LaneType.hpp"
#include "JudgeType.hpp"
#include "SongInfo.hpp"
#include "Config.hpp"

#include "LeaderBoard.hpp"

namespace Globals {
	// Window
	inline const String gameTitle = U"Chrono Beat";
	inline const SemVer gameVersion{ 0, 0, 2, U"beta" };

	inline constexpr Size windowSize{ 1920, 1080 };

	inline Array<SongInfo> songInfos;

	// Volume
	namespace Settings {
		inline double masterVolume = Config.getValue<double>(U"Volume.master", 0.5);
		inline double songVolume = Config.getValue<double>(U"Volume.song", 1.0);
		inline double effectVolume = Config.getValue<double>(U"Volume.effect", 1.0);
		inline double bgmVolume = Config.getValue<double>(U"Volume.bgm", 0.7);

		inline String username = Config.getValue<String>(U"Profile.username", U"Guest{:0>4d}"_fmt(Random<int32>(9999)));

		inline void reload() {
			masterVolume = Config.getValue<double>(U"Volume.master", 0.5);
			songVolume = Config.getValue<double>(U"Volume.song", 1.0);
			effectVolume = Config.getValue<double>(U"Volume.effect", 1.0);
			bgmVolume = Config.getValue<double>(U"Volume.bgm", 0.7);

			username = Config.getValue<String>(U"Profile.username", U"Guest{:0>4d}"_fmt(Random<int32>(9999)));

			GlobalAudio::SetVolume(masterVolume);
		}
	}

	// lane
	inline const int32 laneNum = 4;
	inline const int32 laneWidth = 128;

	inline const double laneStartX =
		(windowSize.x / 2 - laneWidth / 2 - (laneWidth * (laneNum / 2)))
		- 300;

	inline const int32 judgeLineY = windowSize.y - 120;

	// note
	inline const int32 defaultNoteSpeed = 400;
	inline const int32 noteHeight = 24;
	inline double speed = Config.getValue<double>(U"Game.speed", 1.0);

	inline const Duration sceneTransitionTime = 0.75s;

	inline const size_t maxUsernameCharCount = 12;

	namespace Fonts {
		inline const String cinecaption = Resource(U"resource/font/cinecaption226.ttf");
		inline const String mamelon = Resource(U"resource/font/Mamelon-3.5-Hi-Regular.otf");
	}

	namespace Sounds {
		inline const String BGM = Resource(U"resource/bgm/bgm.mp3");

		inline const String pendulumClock = Resource(U"resource/se/pendulum-clock.mp3");
		inline const String moveCursor = Resource(U"resource/se/move-cursor.mp3");
		inline const String songSelected = Resource(U"resource/se/song-selected.mp3");
		inline const String noteClick = Resource(U"resource/se/clock-click.mp3");
		inline const String metronome = Resource(U"resource/se/clock-click.mp3");
	}

	namespace Theme{
		// color from https://pigment.shapefactory.co/?d=3&a=DBD33C&b=393128
		inline const Color backgroundBase{ 57, 49, 40 };
		inline const Color backgroundAccent{ 151, 143, 8 };

		inline const Color JudgePerfect{ 157, 121, 245 };
		inline const Color JudgeGreat{ 218, 156, 219 };
		inline const Color JudgeNear{ 93, 188, 232 };
		inline const Color JudgeMiss{ 85, 68, 108 };
	}

	inline const FilePath BeatmapBaseDir = U"beatmap";

	inline std::array<double, 3> JudgeViewOffsets = {
		100,
		200,
		300
	};
	inline size_t judgeViewIndex = Clamp<size_t>(Config.getValue(U"judge_view", 1), 0, JudgeViewOffsets.size());

	// key
	inline HashTable<LaneType, InputGroup> controllKeys = {
		{ LaneType::D, KeyD },
		{ LaneType::F, KeyF },
		{ LaneType::J, KeyJ },
		{ LaneType::K, KeyK }
	};

	inline HashTable<JudgeType, int32> judgeTimings = {
		{ JudgeType::Perfect, 40 },
		{ JudgeType::Great, 60 },
		{ JudgeType::Near, 80 }
	};

	inline HashTable<JudgeType, double> judgeScoreRatio = {
		{ JudgeType::Perfect, 1.0 },
		{ JudgeType::Great, 0.8 },
		{ JudgeType::Near, 0.5 },
		{ JudgeType::Miss, 0.0 }
	};

	inline HashTable<JudgeType, Color> judgeColor = {
		{ JudgeType::Perfect, Theme::JudgePerfect },
		{ JudgeType::Great, Theme::JudgeGreat },
		{ JudgeType::Near, Theme::JudgeNear },
		{ JudgeType::Miss, Theme::JudgeMiss }
	};

	inline HashTable<JudgeType, String> judgeName = {
		{ JudgeType::Perfect, U"Perfect" },
		{ JudgeType::Great, U"Great" },
		{ JudgeType::Near, U"Near" },
		{ JudgeType::Miss, U"Miss" }
	};

	inline HashTable<String, Array<LeaderBoard::Record>> records;
};
