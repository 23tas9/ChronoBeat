#pragma once
#include <Siv3D.hpp>

/// @brief ロード中の円を描画するアドオン
class LoadingCircleAddon : public IAddon {
public:
	/// @brief ロード中の円の描画を開始します。
	/// @param circle 円
	/// @param thickness 軌跡の太さ 
	/// @param color 軌跡の色
	static void Begin(const Circle& circle, double thickness, const ColorF& color) {
		if (auto p = Addon::GetAddon<LoadingCircleAddon>(U"LoadingCircleAddon")) {
			p->begin(circle, thickness, color);
		}
	}

	/// @brief ロード中の円の描画を終了します。
	static void End() {
		if (auto p = Addon::GetAddon<LoadingCircleAddon>(U"LoadingCircleAddon")) {
			p->end();
		}
	}

	/// @brief ロード中の円の描画が有効かを返します。
	[[nodiscard]]
	static bool IsActive() {
		if (auto p = Addon::GetAddon<LoadingCircleAddon>(U"LoadingCircleAddon")) {
			return p->m_active;
		}
		else {
			return false;
		}
	}

private:

	bool init() override {
		m_trail = Trail{ LifeTime, [](double) { return 1.0; }, EaseOutExpo };

		return true;
	}

	bool update() override {
		if (not m_active) {
			return true;
		}

		m_accumulatedTime += Scene::DeltaTime();

		while (UpdateInterval <= m_accumulatedTime) {
			m_theta = Math::NormalizeAngle(m_theta + AngleStep);

			const Vec2 pos = OffsetCircular{ m_circle.center, m_circle.r, m_theta };

			m_trail.update(UpdateInterval);

			m_trail.add(pos, m_color, m_thickness);

			m_accumulatedTime -= UpdateInterval;
		}

		return true;
	}

	void draw() const override {
		if (not m_active) {
			return;
		}

		m_trail.draw();
	}

	static constexpr double LifeTime = 1.5;

	static constexpr double UpdateInterval = (1.0 / 120.0);

	static constexpr double AngleStep = 1.6_deg;

	Circle m_circle{ 0, 0, 0 };

	double m_thickness = 0.0;

	ColorF m_color = Palette::White;

	Trail m_trail;

	double m_accumulatedTime = 0.0;

	double m_theta = 180_deg;

	bool m_active = false;

	void begin(const Circle& circle, double thickness, const ColorF& color) {
		m_circle = circle;
		m_thickness = thickness;
		m_color = color;
		m_active = true;

		// 開始時点で十分な長さの軌跡を生成しておく
		prewarm();
	}

	void end() {
		m_active = false;
	}

	void prewarm() {
		// 前回の軌跡を消去する。v0.6.14 では m_trail.clear() が使える
		m_trail.update(LifeTime);

		m_accumulatedTime = LifeTime;

		m_theta = 180_deg;

		update();
	}
};
