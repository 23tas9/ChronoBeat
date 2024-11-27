#pragma once

enum class SongDifficulty : int32 {
	Easy = 0,
	Normal,
	Hard,
	Chronos
};

struct BeatmapInfo {
	FilePath jsonPath;
	double difficulty;

	BeatmapInfo(const FilePath& _jsonPath = U"", double _difficulty = .0) :
		jsonPath{ _jsonPath },
		difficulty{ _difficulty } {

	}
};

struct SongInfo {
	static constexpr ColorF EasyColor = Palette::Lime;
	static constexpr ColorF NormalColor = Palette::Yellow;
	static constexpr ColorF HardColor = Palette::Hotpink;
	static constexpr ColorF ChronosColor = Palette::Gray;

	String title = U"";
	String artist = U"";
	String bpm = U"";

	FilePath jacketPath = U"";
	FilePath songPath = U"";

	HashTable<SongDifficulty, BeatmapInfo> beatmapInfos;

	SongInfo(){}

	SongInfo(const FilePath& dir, const INI& info) {
		title = info[U"info.title"];
		artist = info[U"info.artist"];

		bpm = info[U"info.bpm"];

		jacketPath = info[U"info.jacket_path"];
		songPath = info[U"info.song_path"];

		const Array<FilePath> beatmapPaths = info[U"info.beatmap"].split(U',').map([](const FilePath& path) { return path.trimmed(); });
		const Array<double> beatmapDifficulties = info[U"info.difficulty"].split(U',').map([](const String& value) { return Parse<double>(value); });

		for (int32 i : step(static_cast<int32>(SongDifficulty::Chronos) + 1)) {
			beatmapInfos[static_cast<SongDifficulty>(i)] = BeatmapInfo{
				dir + beatmapPaths.at(i),
				beatmapDifficulties.at(i)
			};
		}
	}

	inline String getJacketAssetName() const {
		return U"Jacket.{}"_fmt(title);
	}

	inline String getSongAssetName() const {
		return U"Song.{}"_fmt(title);
	}

	static ColorF GetColor(const SongDifficulty& difficulty) {
		if (difficulty == SongDifficulty::Easy) return EasyColor;
		if (difficulty == SongDifficulty::Normal) return NormalColor;
		if (difficulty == SongDifficulty::Hard) return HardColor;
		return ChronosColor;
	}

	static String GetDifficultyName(const SongDifficulty& difficulty) {
		if (difficulty == SongDifficulty::Easy) return U"Easy";
		if (difficulty == SongDifficulty::Normal) return U"Normal";
		if (difficulty == SongDifficulty::Hard) return U"Hard";
		return U"Chronos";
	}

	friend void Formatter(FormatData& format, const SongInfo& info) {
		format.string += U"{}<{}>: {}"_fmt(info.title, info.artist, info.bpm);
	}
};
