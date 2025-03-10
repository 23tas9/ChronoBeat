#pragma once
#include "Common.hpp"
#include "../Globals.hpp"
#include "../SongInfo.hpp"
#include "../CrawlingText.hpp"

class SelectScene : public App::Scene {
	// 3:4
	static constexpr Size TileSize{ 457, 610 };
	static constexpr int32 TileMarginX = 48;

	// 1:1
	static constexpr Size JacketTileSize{ 350, 350 };
	static constexpr Size JacketTileOffset{ (TileSize.x - JacketTileSize.x) / 2, (TileSize.x - JacketTileSize.x) / 4 };

	static constexpr int32 SongTitleWidth = JacketTileSize.x;
	static constexpr int32 SongTitleOffsetY = 24;

	//
	static constexpr Size DifficultySize{ 100, 100 };

	static constexpr Vec2 TileBaseCenter = Globals::windowSize / 2;

	//
	static constexpr Size RankingAreaSize{ 400, 300 };

	double m_tileOffsetX = .0;
	double m_tileOffsetVelocityX = .0;
	double m_selectableTileWidth = 0.5;

	static constexpr Vec2 SettingTexPos{ Globals::windowSize.movedBy(-128, -128) };
	const Texture& settingTexture = TextureAsset(U"Tex.UI.Setting");

	const Font& m_songTitleFont = FontAsset(U"Font.UI.SongTitle");
	const Font& m_songSubFont = FontAsset(U"Font.UI.SongSub");
	const Font& m_detailFont = FontAsset(U"Font.UI.Detail");

	static constexpr SecondsF SongPreviewFadeInTime = 5.0s;
	int32 m_beforeIndex = -1;
	int32 m_selectInfoIndex = 0;
	Array<SongInfo>& m_infos = Globals::songInfos;

	ColorF currentDifficultyColor = SongInfo::GetColor(getData().currentDifficulty);
	ColorF m_targetDifficultyColor = currentDifficultyColor;
	ColorF m_difficultyVelocity = ColorF{ .0 };

	Audio m_song;

public:
	SelectScene(const InitData& init) : IScene(init) {

	}

	~SelectScene() {
		m_song.stop();
	}

	void update() override {
		if (KeyEscape.down()) {
			changeScene(SceneState::Title, Globals::sceneTransitionTime);
		}

		if (settingTexture.regionAt(SettingTexPos).leftClicked()) {
			changeScene(SceneState::Setting, Globals::sceneTransitionTime);
		}

		SongDifficulty& currentDifficulty = getData().currentDifficulty;

		SongDifficulty deltaDifficulty = currentDifficulty;

		// 曲選択
		m_selectInfoIndex += static_cast<int32>(KeyRight.down()) - static_cast<int32>(KeyLeft.down());

		if (m_selectInfoIndex < 0) m_selectInfoIndex = static_cast<int32>(m_infos.size()) - 1;
		if (m_infos.size() <= m_selectInfoIndex) m_selectInfoIndex = 0;

		// 難易度変更
		int32 currentDifficultyNum = std::to_underlying(currentDifficulty) + static_cast<int32>(KeyUp.down()) - static_cast<int32>(KeyDown.down());
		currentDifficultyNum = Clamp(currentDifficultyNum, 0, std::to_underlying(SongDifficulty::Hard));

		currentDifficulty = static_cast<SongDifficulty>(currentDifficultyNum);

		if (currentDifficulty != deltaDifficulty) {
			m_targetDifficultyColor = SongInfo::GetColor(currentDifficulty);
		}

		//枠線の色変更
		currentDifficultyColor = Math::SmoothDamp(
			currentDifficultyColor,
			m_targetDifficultyColor,
			m_difficultyVelocity,
			.05
		);

		// クリックされた曲を選択
		for (auto&& [i, info] : Indexed(m_infos)) {
			const Vec2 center = TileBaseCenter.movedBy(m_tileOffsetX + (TileMarginX / 2) + (i * (TileSize.x + TileMarginX)), 0);

			// タイル
			RectF region{ Arg::center = center, TileSize };

			if (region.leftClicked()) {
				if (m_selectInfoIndex == static_cast<int32>(i)) transition();
				m_selectInfoIndex = static_cast<int32>(i);
			}
		}

		double targetTileOffset = -m_selectInfoIndex * (TileSize.x + TileMarginX);
		m_tileOffsetX = Math::SmoothDamp(m_tileOffsetX, targetTileOffset, m_tileOffsetVelocityX, 0.1);

		// 曲セレクト
		// (曲選択時、ビジュアル的に選択されるまでは反応しないように)
		if(Math::Abs(m_tileOffsetX - targetTileOffset) <= m_selectableTileWidth){
			if (KeyEnter.down()) {
				transition();
			}
		}

		// 曲が選択されたとき
		if (m_beforeIndex != m_selectInfoIndex) {
			if (m_song.isPlaying()) m_song.stop();

			AudioAsset(U"Audio.UI.MoveCursor").playOneShot(Globals::Settings::effectVolume);

			m_song = AudioAsset(m_infos[m_selectInfoIndex].getSongAssetName());

			m_song.setLoop(true);
			m_song.play(SongPreviewFadeInTime);
			m_song.setVolume(Globals::Settings::songVolume);
		}

		m_beforeIndex = m_selectInfoIndex;
	}

	void draw() const override {
		const SongDifficulty& currentDifficulty = getData().currentDifficulty;

		String currentDifficultyName = SongInfo::GetDifficultyName(currentDifficulty);


		Vec2 difficultyNamePos = TileBaseCenter
			.movedBy(0, -TileSize.y / 2)
			.movedBy(0, -m_songTitleFont.height() * 1.5);

		// 現在の難易度名の表示
		m_songTitleFont(currentDifficultyName)
			.draw(
				TextStyle::Outline(0.2, currentDifficultyColor),
				Arg::bottomCenter = difficultyNamePos,
				Palette::White
			);

		// 何曲目かの文字
		m_songSubFont(U"{:4d} / {:2d}"_fmt(m_selectInfoIndex + 1, m_infos.size()))
			.drawAt(TileBaseCenter.movedBy(0, TileSize.y / 2).movedBy(0, m_songSubFont.height() * 1.5));

		for (auto&& [i, info] : Indexed(m_infos)) {
			const Vec2 center = TileBaseCenter.movedBy(m_tileOffsetX + (TileMarginX / 2) + (i * (TileSize.x + TileMarginX)), 0);

			// タイル
			RectF region{ Arg::center = center, TileSize };

			// 枠線
			region.drawFrame(
				8,
				currentDifficultyColor,
				currentDifficultyColor.withAlpha(.0)
			);

			// ジャケット
			const Texture& jacket = TextureAsset(info.getJacketAssetName());
			const Size& jacketSize = jacket.size();

			const Vec2 jacketCenter = region.pos + JacketTileOffset + JacketTileSize / 2;
			const RectF jacketRegion = jacket.resized(
				Math::Max(JacketTileSize.x, jacketSize.x * static_cast<double>(JacketTileSize.y) / jacketSize.y),
				Math::Max(JacketTileSize.y, jacketSize.y * static_cast<double>(JacketTileSize.x) / jacketSize.x)
			).drawAtClipped(jacketCenter, RectF{ Arg::center = jacketCenter, JacketTileSize })
			.drawFrame(2, Palette::White);

			// タイトル
			const double songTitleFontHeight = m_songTitleFont.height();
			Vec2 songTitleCenter = jacketRegion.bottomCenter().movedBy(0, songTitleFontHeight / 2 + SongTitleOffsetY);

			RectF songTitleRegion{ Arg::center = songTitleCenter, SongTitleWidth, m_songTitleFont.height() };

			// 長いときは流れるように
			if(songTitleRegion.contains(m_songTitleFont(info.title).regionAt(songTitleCenter)))
				m_songTitleFont(info.title).drawAt(songTitleCenter);
			else
				CrawLingText(m_songTitleFont, info.title, songTitleRegion);

			songTitleRegion.bottom().movedBy(0, 12).draw(2, Palette::White);

			// アーティスト
			// const double songSubTitleFontHeight = m_songSubFont.height();
			Vec2 songArtistCenter = songTitleCenter.movedBy(0, songTitleFontHeight * 1.5);

			m_songSubFont(info.artist).drawAt(songArtistCenter);

			// bpm
			m_detailFont(U"BPM: {}"_fmt(info.bpm))
				.draw(Arg::bottomLeft = region.bl().movedBy(24, -24));

			// 難易度
			RectF difficultyRegion{ Arg::bottomRight = region.br().movedBy(-24, -24), DifficultySize };

			m_songTitleFont(U"{:2d}"_fmt(static_cast<int32>(Math::Floor(info.beatmapInfos.at(currentDifficulty).difficulty))))
				.drawAt(difficultyRegion.center());

			difficultyRegion.drawFrame(2.0, Palette::White);
		}

		// ranking
		{
			RectF area{ Arg::center = TileBaseCenter.movedBy(-RankingAreaSize.x * 1.2, 0), RankingAreaSize};

			const auto& ranking = Globals::records[m_infos[m_selectInfoIndex].title];

			Vec2 pos = area.pos.movedBy(16, 16);

			area.drawFrame(1.0, Palette::White);

			if (ranking.isEmpty()) {
				FontAsset(U"Font.UI.Normal")(U"No Data.").drawAt(area.center());
			}
			else {
				for (const auto& [rank, record] : Indexed(ranking)) {
					FontAsset(U"Font.UI.Normal")(U"{}."_fmt(rank + 1)).draw(pos, Palette::Orange);
					FontAsset(U"Font.UI.Normal")(record.userName).draw(pos.movedBy(48, 0));
					FontAsset(U"Font.UI.Normal")(U"{:.2f}"_fmt(record.score)).draw(pos.movedBy(RankingAreaSize.x / 1.5, 0));

					pos.moveBy(0, 60);
				}
			}
		}

		settingTexture.drawAt(SettingTexPos);
	}

	void drawFadeIn(double t) const override {
		draw();

		Common::drawFadeIn(t);
	}

	void drawFadeOut(double t) const override {
		draw();

		Common::drawFadeOut(t);
	}

	void transition() {
		getData().infoIndex = m_selectInfoIndex;

		AudioAsset(U"Audio.UI.SongSelected").playOneShot(Globals::Settings::effectVolume);

		changeScene(SceneState::Game, Globals::sceneTransitionTime);
	}
};
