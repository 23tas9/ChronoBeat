#pragma once
#include <Siv3D.hpp>
#include "../Globals.hpp"

namespace MyEffect {
	struct JudgeView : IEffect {
		static constexpr double MaxLifetime = 0.3;
		static constexpr double FadeTime = 0.1;
		Transition fadeTransition{ SecondsF{ FadeTime }, SecondsF{ FadeTime } };

		static constexpr int32 FloatHeight = 60;

		Vec2 pos;

		String viewText;

		Color color;

		explicit JudgeView(int32 lane, const JudgeType& type) {
			pos = { Globals::laneStartX + (Globals::laneWidth * lane + Globals::laneWidth / 2), Globals::judgeLineY - Globals::JudgeViewOffsets[Globals::judgeViewIndex] };
			viewText = Globals::judgeName.at(type);
			color = Globals::judgeColor.at(type);
		}

		bool update(double t) override {
			const double progress = EaseOutExpo(t / MaxLifetime);

			fadeTransition.update(t <= MaxLifetime - FadeTime);

			const int32 alpha = static_cast<int32>(255 * fadeTransition.easeIn());

			FontAsset(U"Font.Game.Judge.1")(viewText)
				.drawAt(
					TextStyle::Outline(0.1, Palette::White),
					pos.movedBy(0, -FloatHeight * progress),
					color.withAlpha(alpha)
				);
			return (t < MaxLifetime);
		}
	};
}
