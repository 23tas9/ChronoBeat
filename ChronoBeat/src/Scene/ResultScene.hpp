#pragma once
#include "Common.hpp"
#include "../SongInfo.hpp"
#include "../Globals.hpp"

#include "../LeaderBoard.hpp"
#include "../_environment.hpp"

#include "../LoadingCircle.hpp"

struct ResultScene : public App::Scene {
	static constexpr Size JacketTileSize{ 480, 480 };

	static constexpr double LoadingCircleRadius = 40.0;

	SongInfo m_info;
	Texture m_jacketImage;
	SongDifficulty m_difficulty;

	double m_score = 0.0;

	String m_scoreRating = U"SS";

	Optional<AsyncHTTPTask> m_scorePostTask;
	Optional<AsyncHTTPTask> m_scoreGetTask;

	Array<LeaderBoard::Record> m_records;

	SimpleTable m_rankingTable;

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

		m_score = Math::Round(ratio * 10000.0) / 100.0;

		String sheetname = Globals::songInfos[getData().infoIndex].title;

		m_scorePostTask = LeaderBoard::CreatePostTask(Environment::LeaderboardURLRaw, sheetname, Globals::Settings::username, m_score);
	}

	void update() override {
		if (m_scoreGetTask.has_value()) {
			if (m_scoreGetTask->isReady()) {
				if (const auto response = m_scoreGetTask->getResponse(); response.isOK()) {
					if (LeaderBoard::ReadLeaderboard(m_scoreGetTask->getAsJSON(), m_records)) {
						m_rankingTable = LeaderBoard::ToTable(m_records);
					}
					else {
						Print << U"Failed to read the leaderboard.";
					}
				}

				m_scoreGetTask.reset();
			}
		}

		if (m_scorePostTask.has_value()) {
			if (m_scorePostTask->isReady()) {
				if (const auto response = m_scorePostTask->getResponse(); response.isOK()) {
					m_scoreGetTask = LeaderBoard::CreateGetTask(Environment::LeaderboardURLRaw, Globals::songInfos[getData().infoIndex].title, 5);
				}
				else {
					Print << U"Failed to submit the score.";
				}

				m_scorePostTask.reset();
			}
		}

		// 遷移
		if ((KeySpace | KeyEnter | MouseL).down()) {
			changeScene(SceneState::Select, Globals::sceneTransitionTime);
		}
	}

	void draw() const override {
		auto& data = getData();

		FontAsset(U"Font.UI.Title")(SongInfo::GetDifficultyName(m_difficulty)).draw(Arg::topCenter = Scene::Rect().topCenter().movedBy(0, 128));

		// ranking
		{
			Vec2 pos{ 256, 256 };

			if (m_rankingTable) {
				m_rankingTable.draw(pos);
			}
			else {
				FontAsset(U"Font.UI.Normal")(U"読み込み中...").draw(pos);
			}
		}

		// score
		{
			Vec2 pos{ 256, 640 };

			for (auto&& [key, value] : data.judges) {
				const Color& color = Globals::judgeColor[key];
				FontAsset(U"Font.Game.Judge.2")(GetJudgeName(key))
					.draw(
						TextStyle::Outline(0.1, Palette::White),
						pos.moveBy(0, 80), color
					);
				FontAsset(U"Font.Game.Judge.2")(U"{:4d}"_fmt(value)).draw(pos.movedBy(320, 0));
			}
		}

		// jacket
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

		if (m_scorePostTask.has_value()) {
			if (not m_scorePostTask->isReady()) {
				if (not LoadingCircleAddon::IsActive()) {
					Vec2 pos = Globals::windowSize.movedBy(-LoadingCircleRadius - 16, -LoadingCircleRadius - 16);
					LoadingCircleAddon::Begin(Circle{ pos, LoadingCircleRadius }, 2.0, Palette::White);
				}
			}
		}
		else if (LoadingCircleAddon::IsActive()) {
			LoadingCircleAddon::End();
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
