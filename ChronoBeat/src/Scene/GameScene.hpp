#pragma once
#include "Common.hpp"
#include "../GameManager.hpp"

#include "../SongInfo.hpp"

class GameScene : public App::Scene {
	GameManager m_game;

	SongInfo m_info;
	Audio m_song;

	Texture m_jacketImage;

	static constexpr Size JacketTileSize{ 350, 350 };
	static constexpr int32 JudgeViewMargin = 64;

	SimpleAnimation m_playCount;

	double m_metronomeMergin;

	Stopwatch m_metronomeTimer{ StartImmediately::No };
	Stopwatch m_songTimer{ StartImmediately::No };

	bool m_isPlayed = false;

	size_t m_metronomeCount = 0;

	double m_gameSpeed = 1.0;

	bool m_isAutomode = false;

public:
	GameScene(const InitData& init) : IScene(init) {
		m_info = Globals::songInfos[getData().infoIndex];

		const BeatmapInfo& beatmapInfo = m_info.beatmapInfos[getData().currentDifficulty];
		const Beatmap& beatmap{ beatmapInfo.jsonPath };//{ U"E:/Program Files/NoteEditor/Beatmap/Notes/03_Dogbite.json" };

		m_metronomeMergin = 60.0 / beatmap.bpm / m_gameSpeed;

		m_game = GameManager{ beatmap };

		m_song = AudioAsset(m_info.getSongAssetName());

		m_song.setLoop(false);
		m_song.setVolume(Globals::Settings::songVolume);

		m_jacketImage = TextureAsset(m_info.getJacketAssetName());

		m_playCount
			.set(U"Ready", { 0.0s, 0.0 }, { 1.0s, 1.0 }, EaseOutCubic)
			.set(U"Ready", { 4.0s, 1.0 }, { 4.0s + SecondsF{ m_metronomeMergin }, 0.0 }, EaseInCubic)
			.start();

		//System::SetTerminationTriggers(UserAction::NoAction);
	}

	~GameScene() {
		System::SetTerminationTriggers(UserAction::Default);
	}

	void update() override {
		if (SimpleGUI::CheckBox(m_isAutomode, U"Auto", { 10, 10 }));
		/*
		if (SimpleGUI::Slider(Globals::speed, 1.0, 100.0, { 10, 80 }));
		*/

		if (not m_playCount.isDone()) return;
		if (not m_metronomeTimer.isStarted()) m_metronomeTimer.start();
		if (not m_songTimer.isStarted()) m_songTimer.start();

		if (isFinished()) {
			auto& data = getData();

			data.judges = m_game.getJudges();

			data.combo = { m_game.getMaxCombo(), m_game.getBeatmap().maxCombo };

			changeScene(SceneState::Result, Globals::sceneTransitionTime);
		}

		const double now = m_songTimer.sF();

		if (4 <= m_metronomeCount) {
			if (not m_isPlayed) {
				m_song.play();
				m_isPlayed = true;
			}
		}
		else if (const double currentTimer = m_metronomeTimer.sF(); m_metronomeMergin <= currentTimer) {
			AudioAsset(U"Audio.Game.Metronome").playOneShot(Globals::Settings::effectVolume);
			m_metronomeCount += 1;

			m_metronomeTimer.set(SecondsF{ currentTimer - m_metronomeMergin });
		}

		m_game.update(now * m_gameSpeed, m_isAutomode);
	}

	void draw() const override {
		const double now = m_songTimer.sF();
		const double ratio = now / m_song.lengthSec();

		// ジャケット
		{
			const RectF jacketRegion{
				Arg::topRight = Vec2{ Globals::windowSize.x - 64, 64 },
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

			{
				Vec2 pos = bl.movedBy(-80, 32);

				for (auto&& [key, value] : m_game.getJudges()) {
					const Color& color = Globals::judgeColor[key];
					FontAsset(U"Font.Game.Judge.1")(GetJudgeName(key))
						.draw(
							TextStyle::Outline(0.1, Palette::White),
							pos.moveBy(0, JudgeViewMargin), color
						);
					FontAsset(U"Font.Game.Judge.1")(U"{:4d}"_fmt(value)).draw(pos.movedBy(160, 0));
				}
			}
		}

		m_game.draw(now * m_gameSpeed);

		// Ready?
		if (not m_playCount.isDone()) {
			const Vec2 scenter = Scene::CenterF();
			const RectF region = FontAsset(U"Font.UI.Title")(U"Ready?").regionAt(scenter);

			const double readyRatio = m_playCount[U"Ready"];

			// 背景暗く
			Scene::Rect().draw(ColorF{ 0, 0, 0, 0.3 * readyRatio });

			FontAsset(U"Font.UI.Title")(U"Ready?").draw(
				Arg::center = scenter,
				ColorF{ 1.0, 1.0, 1.0, readyRatio }
			);
		}
	}

	bool isFinished() const {
		return m_isPlayed && not m_song.isActive();
	}

	void drawFadeIn(double t) const override {
		draw();

		Common::drawFadeIn(t);
	}
};
