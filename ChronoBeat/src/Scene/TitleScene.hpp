#pragma once
#include "Common.hpp"

struct TitleScene : public App::Scene {
	const Audio& m_titleBGM = AudioAsset(U"Audio.UI.Title");

	///////////////
	// Animations
	///////////////
	// font
	static constexpr int32 TitleAnimationPerCharMS = 200;
	Stopwatch m_titleAnimTimer{ StartImmediately::No };

	// background anim
	static constexpr int32 BackgroundFadeInMS = 2000;
	static constexpr double BackgroundBaseOpacity = 32.0;
	Stopwatch m_backgroundTimer{ StartImmediately::No };

	// letterbox
	static constexpr double LetterBoxHeight = (Globals::windowSize.y - Globals::windowSize.x / 2.35) / 2;	// 16:9 -> 2.35:1
	static constexpr int32 LetterBoxFadeOutMS = 1000;

	bool m_animationFinished = false;

public:
	TitleScene(const InitData& init) : IScene(init) {

	}

	void update() override {
		if (not m_titleBGM.isPlaying()) {
			m_titleBGM.play(Globals::sceneTransitionTime);
			m_titleBGM.setVolume(Globals::Settings::bgmVolume);
		}

		const bool actioned = Keyboard::GetAllInputs().any([](const Input& key) {
			return key.down();
		}) || Mouse::GetAllInputs().any([](const Input& key) {
			return key.down();
		});

		// アニメーションが終わっていてアクションがあれば遷移
		if (m_animationFinished && actioned) {
			m_titleBGM.stop();
			changeScene(SceneState::Select, Globals::sceneTransitionTime);
		}

		// アクションした場合アニメーションのスキップ
		if (actioned && not m_animationFinished) {
			m_titleAnimTimer.set(Milliseconds{ TitleAnimationPerCharMS * Globals::gameTitle.size()});
			m_backgroundTimer.set(Milliseconds{ BackgroundFadeInMS });

			m_backgroundTimer.start();

			m_animationFinished = true;
		}

		// フェードイン完了時
		if (not m_titleAnimTimer.isStarted()) m_titleAnimTimer.start();

		const double now = m_titleAnimTimer.ms();

		// タイトル描画が終わった時背景描画開始
		if (TitleAnimationPerCharMS * Globals::gameTitle.size() <= now && not m_backgroundTimer.isStarted())
			m_backgroundTimer.start();

		// タイトル描画、背景のアニメーションが完了したなら
		if(TitleAnimationPerCharMS * Globals::gameTitle.size() + BackgroundFadeInMS <= now){
			m_animationFinished = true;
		}
	}

	void draw() const override {
		drawBackgroundClock();
		drawLetterBox();
		drawTitle();
	}

	void drawBackgroundClock() const {
		const Vec2& center = Scene::CenterF();
		const double r = Globals::windowSize.x / 2.25;

		const double now = m_backgroundTimer.msF();
		const double nowS = m_backgroundTimer.sF();
		const double scale = (now <= BackgroundFadeInMS) ?
			EaseOutCubic(Math::Min(1.0, now / BackgroundFadeInMS)) :
			1.0;

		Color bColor = ColorF{ Palette::White, .0 };
		Color eColor = ColorF{ Palette::White, 0.7 * scale };

		double angleM = nowS / 6 * 360_deg;
		double angleH = nowS / 6 / 12 * 360_deg;

		// 時計
		Circle{ center, r * scale }
			.draw(bColor, eColor.withAlpha(64))
			.drawFrame(2, eColor);
		Circle{ center, r * scale + 16 }
			.drawFrame(2, eColor);

		// 時計の分、時間表示
		for (auto i : step(60)) {
			const double theta = i * 6_deg;
			const bool isHour = i % 5 == 0;
			const Vec2 pos = OffsetCircular(center, r * scale - 1, theta);

			Line{ pos.lerp(center, isHour ? 0.2 : 0.1), pos }.draw(4, bColor, eColor);
		}

		// 時計の針
		Line{ center, Arg::angle = angleM, r / 1.2 * scale }.draw(2, bColor, eColor);

		Line{ center, Arg::angle = angleH, r / 2 * scale }.draw(2, bColor, eColor);
	}

	void drawLetterBox() const {
		const double now = m_backgroundTimer.msF();

		double letterboxRatio = (now <= LetterBoxFadeOutMS) ?
			EaseInCubic(1.0 - Math::Min(1.0, now / LetterBoxFadeOutMS)) :
			0.0;

		// レターボックス
		RectF{ 0, 0, Globals::windowSize.x, LetterBoxHeight * letterboxRatio }.draw(Palette::Black);
		RectF{ Arg::bottomLeft = Vec2{ 0, Globals::windowSize.y }, Globals::windowSize.x, LetterBoxHeight * letterboxRatio }.draw(Palette::Black);
	}

	void drawTitle() const {
		const Vec2 scenter = Scene::CenterF();
		const double now = m_titleAnimTimer.ms();

		const Array<Glyph>& titleGlyphs = FontAsset(U"Font.UI.Title").getGlyphs(Globals::gameTitle);

		Vec2 penPos = FontAsset(U"Font.UI.Title")(Globals::gameTitle).regionAt(scenter).pos;

		for (const auto& [index, glyph] : Indexed(titleGlyphs)) {
			if (now <= TitleAnimationPerCharMS * index) break;

			Color color = Palette::White;

			glyph.texture.draw(Math::Round(penPos + glyph.getOffset()), color);

			penPos.x += glyph.xAdvance;
		}

		if (m_backgroundTimer.isStarted()) {
			const double scale = (now <= BackgroundFadeInMS) ?
				EaseInCubic(Math::Min(1.0, now / BackgroundFadeInMS)) :
				1.0;

			const ColorF color{
				1.0,
				scale
			};

			FontAsset(U"Font.UI.Detail")(U"Press any key to start...")
				.drawAt(scenter.movedBy(0, 120), color.withAlpha(Math::Min(scale, 0.5 + 0.5 * Periodic::Sine0_1(1.5s))));

			FontAsset(U"Font.UI.Detail")(Globals::gameVersion.toStr())
				.draw(Arg::bottomRight = Globals::windowSize.movedBy(-32, -32), color);
		}
	}

	void drawFadeOut(double t) const override {
		draw();

		Common::drawFadeOut(t);
	}
};
