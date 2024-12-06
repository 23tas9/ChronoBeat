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
		difficulty{ _difficulty } { }
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

	SongInfo(const JSON&);

	SongInfo registerAsset() const;

	inline String getJacketAssetName() const;

	inline String getSongAssetName() const;

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
