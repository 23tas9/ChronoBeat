﻿#pragma once
#include <memory>
#include <bitset>

#include "Note.hpp"
#include "LaneType.hpp"
#include "Beatmap.hpp"
#include "Effect/JudgeView.hpp"

class GameManager {
	Beatmap m_beatmap;
	HashTable<JudgeType, size_t> m_judges{
		{ JudgeType::Perfect, 0 },
		{ JudgeType::Great, 0 },
		{ JudgeType::Near, 0 },
		{ JudgeType::Miss, 0 }
	};

	double scroll = 1.0;

	size_t m_maxCombo = 0;
	size_t m_combo = 0;

	Audio m_noteClickSound = AudioAsset(U"Audio.Game.NoteClick");

	Effect m_judgeViewer;

public:
	GameManager() = default;

	GameManager(const Beatmap& beatmap) : m_beatmap{ beatmap } {

	}

	void update(double t, bool autoMode = false) {
		std::bitset<4> processedLane = 0b0000;

		for (auto&& note : m_beatmap.notes) {
			JudgeType judge = note->update(t);

			HoldNote* holdNote = dynamic_cast<HoldNote*>(note.get());

			if (autoMode) {
				judge = JudgeType::None;
				if (note->timeDiff(t) <= 0) {
					judge = JudgeType::Perfect;
					note->isRemovable = true;

					if (holdNote != nullptr) {
						if (holdNote->isHolding && 0 < note->timeDiff(t - holdNote->length)) {
							judge = JudgeType::None;
							note->isRemovable = false;
						}

						if (not holdNote->isHolding) {
							note->isRemovable = false;
							holdNote->isHolding = true;
						}
					}
				}
			}

			if (judge == JudgeType::None) continue;
			if (processedLane[note->lane] & 1) continue;

			int8 state = 1;

			if (holdNote != nullptr) {
				if (holdNote->isHolding) state = 0;
			}

			if (judge != JudgeType::Miss) {
				m_noteClickSound.playOneShot(Globals::Settings::effectVolume);

				m_combo += 1;
				if (m_maxCombo < m_combo) m_maxCombo = m_combo;
			}
			if(judge == JudgeType::Miss) {
				note->isRemovable = true;

				m_combo = 0;

				// not isHoldingのときは終点も考慮してmissを++する
				if (holdNote != nullptr) {
					if (not holdNote->isHolding) m_judges[judge] += 1;
				}
			}

			m_judgeViewer.add<MyEffect::JudgeView>(note->lane, judge);

			m_judges[judge] += 1;

			processedLane[note->lane] = state;
		}

		m_beatmap.notes.remove_if([=](const auto& note) {
			return note->isRemovable;
		});
	}

	void draw(double t) const {
		drawLane();

		for (const auto& note : m_beatmap.notes) {
			note->draw(t, scroll);
		}

		{
			double x = Globals::laneStartX + (Globals::laneWidth * (Globals::laneNum / 2));

			FontAsset(U"Font.Game.Combo")((m_combo < 10) ? U"" : U"{}"_fmt(m_combo)).drawAt(TextStyle::Outline(0.2, Palette::Black), x, Globals::judgeLineY - 300);
		}

		drawJudgeLine();

		m_judgeViewer.update();
	}

	void drawLane() const {
		for (int32 i : step(Globals::laneNum)) {
			double x = Globals::laneStartX + (Globals::laneWidth * i);

			RectF rect{ x, .0, Globals::laneWidth, Globals::windowSize.y };

			rect.draw(Palette::Black).drawFrame(1.0, Palette::White);

			if (Globals::controllKeys[static_cast<LaneType>(i)].pressed()) {
				RectF{ x, Globals::judgeLineY - Globals::windowSize.y / 2.0, Globals::laneWidth, Globals::windowSize.y / 2.0 }
					.draw(Arg::top = ColorF{ Palette::Yellow, .0 }, Arg::bottom = ColorF{ Palette::Yellow, .25 });
			}
		}
	}

	void drawJudgeLine() const {
		Line judgeLine{
			Globals::laneStartX, Globals::judgeLineY,
			Globals::laneStartX + Globals::laneWidth * Globals::laneNum, Globals::judgeLineY
		};

		judgeLine.draw(2.0, Palette::Orange);
	}

	inline const HashTable<JudgeType, size_t>& getJudges() const {
		return m_judges;
	}

	inline const Array<std::shared_ptr<Note>>& getNote() const noexcept {
		return m_beatmap.notes;
	}

	inline const Beatmap& getBeatmap() const noexcept {
		return m_beatmap;
	}

	inline const size_t getMaxCombo() const noexcept {
		return m_maxCombo;
	}
};
