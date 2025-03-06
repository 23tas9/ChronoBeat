#pragma once
#include "Common.hpp"
#include "../SongInfo.hpp"
#include "../Globals.hpp"

struct TextboxUI {
	static constexpr double UIWidth = 400.0;
	static constexpr double UIHeight = 80.0;

	Optional<size_t> maxChars = none;

	TextEditState edit;

	Font font = FontAsset(U"Font.UI.Normal");
	RoundRect displayRect;

	bool selected = false;

	TextboxUI() = default;

	TextboxUI(const Vec2& pos, const String& init = U"", Optional<size_t> _maxChars = none) : maxChars{_maxChars},
		displayRect{ pos, UIWidth, UIHeight, UIHeight / 2 },
		edit{ init } {

	}

	void draw() const {
		displayRect.draw();

		font(edit.text).draw(Arg::leftCenter = displayRect.leftCenter().movedBy(16, 0), Palette::Black);

		displayRect.drawFrame(selected ? 3.0 : 1.0, Globals::Theme::backgroundAccent);
	}

	bool update() {
		bool delta = selected;

		if (MouseL.down()) {
			if (displayRect.mouseOver())
				selected = true;
			else
				selected = false;
		}
		else if (KeyEnter.down()) {
			selected = false;
		}

		edit.active = selected;

		if (edit.active) {
			edit.cursorPos = TextInput::UpdateText(edit.text, edit.cursorPos, TextInputMode::AllowBackSpaceDelete);

			if (maxChars.has_value()) {
				if (*maxChars < edit.text.size()) {
					edit.text.resize(*maxChars);
					edit.cursorPos = Min(edit.cursorPos, *maxChars);
				}
			}
		}

		return (delta != selected && selected == false);
	}

	inline String value() const {
		return edit.text;
	}
};

struct NumberUI {
	static constexpr double UIWidth = 400.0;
	static constexpr double UIHeight = 80.0;

	static constexpr double ButtonWidth = 60.0;

	static constexpr double ButtonMergin = 20.0;

	double value = 0.0;

	double minv = 0.0;
	double maxv = 1.0;

	double amount = 0.1;

	RoundRect display;

	Triangle rbtn;
	Triangle lbtn;

	NumberUI() = default;

	NumberUI(const Vec2& pos, double init, double _minv = 0.0, double _maxv = 1.0, double _amount = 0.1) :
		value{ init }, minv{ _minv }, maxv{ _maxv }, amount{ _amount },
		display{ Arg::center = pos.movedBy(UIWidth / 2, UIHeight / 2), UIWidth - (ButtonWidth * 2 + ButtonMergin * 2), UIHeight, UIHeight / 2 },
		rbtn{ pos.movedBy(UIWidth - ButtonWidth / 2, UIHeight / 2), ButtonWidth, 90_deg },
		lbtn{ pos.movedBy(ButtonWidth / 2, UIHeight / 2), ButtonWidth, -90_deg } { }
	
	NumberUI(const Arg::center_<Vec2>& center, double init, double _minv = 0.0, double _maxv = 1.0, double _amount = 0.1) :
		NumberUI(center->movedBy(-UIWidth / 2, -UIHeight / 2), init, _minv, _maxv, _amount){ }

	void draw() const {
		display.draw();

		FontAsset(U"Font.UI.Normal")(U"{:.1f}"_fmt(value)).drawAt(display.center(), Palette::Black);

		rbtn.draw()
			.drawFrame(rbtn.leftPressed() ? 3.0 : 0.0, Globals::Theme::backgroundAccent);

		lbtn.draw()
			.drawFrame(lbtn.leftPressed() ? 3.0 : 0.0, Globals::Theme::backgroundAccent);
	}

	bool update() {
		double delta = value;

		if (rbtn.leftClicked()) {
			value += amount;
		}

		if (lbtn.leftClicked()) {
			value -= amount;
		}

		value = Math::Clamp(value, minv, maxv);

		return delta != value;
	}
};

struct SettingScene : public App::Scene {
	static constexpr Vec2 UIStartPos{ 128, 128 };
	static constexpr double UITextWidth = 256;

	static constexpr double NoteSpawnMergin = 0.5;

	Stopwatch merginTimer{ StartImmediately::No };
	Stopwatch noteTimer{ StartImmediately::Yes };

	TextboxUI usernameUI;

	NumberUI masterVolumeUI;
	NumberUI songVolumeUI;
	NumberUI effectVolumeUI;
	NumberUI bgmVolumeUI;

	NumberUI noteSpeedUI;

	RoundRect backButton{
		Vec2{ UIStartPos.x / 2, Globals::windowSize.y - UIStartPos.y / 2 - NumberUI::UIHeight },
		NumberUI::UIWidth, NumberUI::UIHeight, NumberUI::UIHeight / 2
	};

	Audio bgm = AudioAsset(U"Audio.UI.BGM");

public:
	SettingScene(const InitData& init) : IScene(init) {
		Vec2 pos = UIStartPos.movedBy(UITextWidth, 0);

		usernameUI = { pos, Globals::Settings::username, 12 };

		masterVolumeUI = { pos.moveBy(0, NumberUI::UIHeight + NumberUI::ButtonMergin), Globals::Settings::masterVolume, 0.1, 1.0, 0.1 };
		songVolumeUI = { pos.moveBy(0, NumberUI::UIHeight + NumberUI::ButtonMergin), Globals::Settings::songVolume, 0.1, 1.0, 0.1 };
		effectVolumeUI = { pos.moveBy(0, NumberUI::UIHeight + NumberUI::ButtonMergin), Globals::Settings::effectVolume, 0.1, 1.0, 0.1 };
		bgmVolumeUI = { pos.moveBy(0, NumberUI::UIHeight + NumberUI::ButtonMergin), Globals::Settings::bgmVolume, 0.1, 1.0, 0.1 };

		noteSpeedUI = { pos.moveBy(0, NumberUI::UIHeight + NumberUI::ButtonMergin), Globals::speed, 0.5, 10.0, 0.5 };

		bgm.setVolume(Globals::Settings::bgmVolume);
	}

	~SettingScene(){
		bgm.stop(Globals::sceneTransitionTime);

		reload();

		Globals::speed = noteSpeedUI.value;
	}

	void update() override {
		if (not bgm.isActive()) bgm.play();
		bgm.setVolume(Globals::Settings::bgmVolume);

		if (backButton.leftClicked()) {
			changeScene(SceneState::Select, Globals::sceneTransitionTime);
		}

		if (Globals::windowSize.y < (noteTimer.sF() * Globals::defaultNoteSpeed * Globals::speed)) {
			if (not merginTimer.isRunning()) merginTimer.start();
		}

		if (NoteSpawnMergin <= merginTimer.sF()) {
			merginTimer.reset();
			noteTimer.restart();
		}

		bool flag = false;

		flag =
			masterVolumeUI.update() ||
			songVolumeUI.update() ||
			effectVolumeUI.update() ||
			bgmVolumeUI.update() ||
			noteSpeedUI.update() ||
			usernameUI.update();

		if (flag) {
			reload();

			Globals::speed = noteSpeedUI.value;

			AudioAsset(U"Audio.UI.MoveCursor").playOneShot(Globals::Settings::effectVolume);
		}
	}

	void draw() const override {
		Vec2 pos = UIStartPos;

		FontAsset(U"Font.UI.Normal")(U"Username").draw(Arg::leftCenter = pos.movedBy(0, TextboxUI::UIHeight / 2));
		usernameUI.draw();

		FontAsset(U"Font.UI.Normal")(U"Master Volume").draw(Arg::leftCenter = pos.moveBy(0, NumberUI::UIHeight / 2 + NumberUI::UIHeight + NumberUI::ButtonMergin));
		masterVolumeUI.draw();

		FontAsset(U"Font.UI.Normal")(U"Song Volume").draw(Arg::leftCenter = pos.moveBy(0, NumberUI::UIHeight + NumberUI::ButtonMergin));
		songVolumeUI.draw();

		FontAsset(U"Font.UI.Normal")(U"Effect Volume").draw(Arg::leftCenter = pos.moveBy(0, NumberUI::UIHeight + NumberUI::ButtonMergin));
		effectVolumeUI.draw();

		FontAsset(U"Font.UI.Normal")(U"BGM Volume").draw(Arg::leftCenter = pos.moveBy(0, NumberUI::UIHeight + NumberUI::ButtonMergin));
		bgmVolumeUI.draw();

		FontAsset(U"Font.UI.Normal")(U"Note Speed").draw(Arg::leftCenter = pos.moveBy(0, NumberUI::UIHeight + NumberUI::ButtonMergin));
		noteSpeedUI.draw();

		backButton.draw();
		FontAsset(U"Font.UI.Normal")(U"Back").drawAt(backButton.center(), Palette::Black);

		{
			Rect lane{ Globals::windowSize.x - Globals::laneWidth - 256, 0, Globals::laneWidth, Globals::windowSize.y };

			lane.draw(Palette::Black).drawFrame(1.0, Palette::White);

			Line{ lane.x, Globals::judgeLineY, lane.x + lane.w, Globals::judgeLineY }.draw(Palette::Orange);

			double y = noteTimer.sF() * Globals::defaultNoteSpeed * Globals::speed;

			RectF note{ Arg::center = Vec2{ lane.center().x, y }, lane.w, Globals::noteHeight };
			note.rounded(2).draw();
		}
	}

	void reload() const {
		Config.setValue(U"Volume.master", masterVolumeUI.value);
		Config.setValue(U"Volume.song", songVolumeUI.value);
		Config.setValue(U"Volume.effect", effectVolumeUI.value);
		Config.setValue(U"Volume.bgm", bgmVolumeUI.value);

		Config.setValue(U"Game.speed", noteSpeedUI.value);

		Config.setValue(U"Profile.username", usernameUI.value());

		Config.save();

		Globals::Settings::reload();
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
