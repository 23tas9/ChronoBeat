#pragma once
#include <Siv3D.hpp>
#include "../SongInfo.hpp"

enum class SceneState {
	Title,
	Select,
	Setting,
	Game,
	Result
};

struct GameData {
	size_t infoIndex = 0;
	SongDifficulty currentDifficulty = SongDifficulty::Hard;

	HashTable<JudgeType, size_t> judges{
		{ JudgeType::Perfect, 0 },
		{ JudgeType::Great, 0 },
		{ JudgeType::Near, 0 },
		{ JudgeType::Miss, 0 }
	};

	std::pair<size_t, size_t> combo = { 0, 0 };
};

using App = SceneManager<SceneState, GameData>;

namespace Common {
	inline const double CircleRadius = Math::Sqrt(
		 Math::Pow(Globals::windowSize.x, 2) + Math::Pow(Globals::windowSize.y, 2)
	);

	void drawFadeIn(double t) {
		double progress = Math::Clamp(1.0 - (t / Globals::sceneTransitionTime.count()), 0.0, 1.0);

		Scene::Rect().draw(ColorF{ Palette::Black, progress });
		Circle{ Scene::Center(), CircleRadius }.drawPie(0_deg, -progress * 360_deg, Palette::Black);
	}

	void drawFadeOut(double t) {

		double progress = Math::Clamp(t / Globals::sceneTransitionTime.count(), 0.0, 1.0);

		Scene::Rect().draw(ColorF{ Palette::Black, progress });
		Circle{ Scene::Center(), CircleRadius }.drawPie(0_deg, progress * 360_deg, Palette::Black);
	}
}
