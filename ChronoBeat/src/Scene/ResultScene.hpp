#pragma once
#include "Common.hpp"
#include "../SongInfo.hpp"
#include "../Globals.hpp"

struct ResultScene : public App::Scene {
	static constexpr Size JacketTileSize{ 480, 480 };

	SongInfo m_info;
	Texture m_jacketImage;
	SongDifficulty m_difficulty;

	double m_score = 0.0;

	String m_scoreRating = U"SS";

public:
	ResultScene(const InitData& init) : IScene(init) {
		auto& data = getData();

		m_info = Globals::songInfos[data.infoIndex];

		m_difficulty = data.currentDifficulty;

		m_jacketImage = TextureAsset(m_info.getJacketAssetName());

		double ratio = 0.0;

		for (auto&& [key, value] : data.judges) {
			ratio += value / static_cast<double>(data.combo.second) * Globals::judgeScoreRatio[key];
		}

		if (ratio < 1.0) m_scoreRating = U"S";
		if (ratio < 0.9) m_scoreRating = U"AA";
		if (ratio < 0.8) m_scoreRating = U"A";
		if (ratio < 0.75) m_scoreRating = U"B";
		if (ratio < 0.5) m_scoreRating = U"C";

		m_score = ratio * 100.0;
	}

	void update() override {
		if ((KeySpace | KeyEnter | MouseL).down()) {
			changeScene(SceneState::Select, Globals::sceneTransitionTime);
		}
	}

	void draw() const override {
		auto& data = getData();

		FontAsset(U"Font.UI.Title")(SongInfo::GetDifficultyName(m_difficulty)).draw(Arg::topCenter = Scene::Rect().topCenter().movedBy(0, 128));

		{
			Vec2 pos{ 256, 256 };

			for (auto&& [key, value] : data.judges) {
				const Color& color = Globals::judgeColor[key];
				FontAsset(U"Font.Game.Judge.2")(GetJudgeName(key))
					.draw(
						TextStyle::Outline(0.1, Palette::White),
						pos.moveBy(0, 128), color
					);
				FontAsset(U"Font.Game.Judge.2")(U"{:4d}"_fmt(value)).draw(pos.movedBy(320, 0));
			}
		}

		{
			const RectF jacketRegion{
				Arg::topRight = Vec2{ Globals::windowSize.x - 256, 256 },
				JacketTileSize
			};

			const Size& jacketSize = m_jacketImage.size();

			// 比率を維持してJacketTileSizeの大きさでジャケット画像を描画
			Vec2 bl = m_jacketImage.resized(
				Math::Max(JacketTileSize.x, jacketSize.x * static_cast<double>(JacketTileSize.y) / jacketSize.y),
				Math::Max(JacketTileSize.y, jacketSize.y * static_cast<double>(JacketTileSize.x) / jacketSize.x)
			).drawAtClipped(
				jacketRegion.center(),
				jacketRegion
			).drawFrame(2, Palette::White).bl();
		}

		{
			Vec2 base{ Globals::windowSize.x / 2 + 160, Globals::windowSize.y - 128 };

			FontAsset(U"Font.UI.Result.1")(m_scoreRating).draw(Arg::bottomRight = base);
			FontAsset(U"Font.UI.Result.2")(U"{:3.2f}%"_fmt(m_score)).draw(Arg::bottomLeft = base.movedBy(160, 0));
		}
	}

	void drawFadeIn(double t) const override {
		draw();

		Common::drawFadeIn(t);
	}

	void drawFadeOut(double t) const override {
		draw();

		Common::drawFadeOut(t);
	}
};
