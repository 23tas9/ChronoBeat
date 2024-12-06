#include "SongInfo.hpp"
#include "Globals.hpp"

SongInfo::SongInfo(const JSON& json) {
	FilePath base = FileSystem::PathAppend(Globals::BeatmapBaseDir, json[U"name"].getString());

	title = json[U"title"].getString();
	artist = json[U"artist"].getString();

	bpm = json[U"bpm"].getString();

	jacketPath = FileSystem::PathAppend(base, U"jacket.png");
	songPath = FileSystem::PathAppend(base, U"song.wav");

	const Array<FilePath> beatmapPaths{
		U"easy.json",
		U"normal.json",
		U"hard.json",
		U"chronos.json"
	};
	Array<double> beatmapDifficulties{ };

	for (const JSON& e : json[U"difficulties"].arrayView()) {
		beatmapDifficulties << e.get<double>();
	}

	for (int32 i : step(static_cast<int32>(SongDifficulty::Chronos) + 1)) {
		beatmapInfos[static_cast<SongDifficulty>(i)] = BeatmapInfo{
			FileSystem::PathAppend(base, beatmapPaths.at(i)),
			beatmapDifficulties.at(i)
		};
	}
}

SongInfo SongInfo::registerAsset() const {
	const String& songName = getSongAssetName();
	const String& textureName = getJacketAssetName();

	if (not AudioAsset::Register(songName, Resource(songPath))) throw AssetRegistError{ songName };
	if (not TextureAsset::Register(textureName, Resource(jacketPath))) throw AssetRegistError{ textureName };

	AudioAsset::Load(songName);
	TextureAsset::Load(textureName);

	return std::move(*this);
}

inline String SongInfo::getJacketAssetName() const {
	return U"Jacket.{}"_fmt(title);
}

inline String SongInfo::getSongAssetName() const {
	return U"Song.{}"_fmt(title);
}
