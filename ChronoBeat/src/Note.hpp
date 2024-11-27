#pragma once
#include "NoteType.hpp"
#include "LaneType.hpp"
#include "JudgeType.hpp"
#include "Globals.hpp"

struct Note {
	static constexpr int32 NoteMergin = 8;

	// 0 ~ 3
	int32 lane = 0;
	double timing = .0;
	double speed = 1.0;
	bool isRemovable = false;

	size_t priority = 0;

	Note(int32, double, double);

	virtual ~Note() = default;

	/// @brief ノーツの作成(Hold以外
	/// @param type ノーツのタイプ
	/// @param lane どこのレーンか
	/// @param t タイミング
	/// @param speed 速度
	/// @return 作られたノーツ
	static std::shared_ptr<Note> Make(NoteType type, int32 lane, double t, double speed);

	/// @brief ノーツの作成(Holdのみ
	/// @param type ノーツのタイプ
	/// @param lane どこのレーンか
	/// @param t タイミング
	/// @param speed 速度
	/// @param length ノーツの長さ
	/// @return 作られたノーツ
	static std::shared_ptr<Note> Make(NoteType type, int32 lane, double t, double length, double speed);

	/// @brief lane番号からLaneTypeに変換
	/// @return LaneType
	LaneType getLaneType() const;

	JudgeType getJudge(double) const;

	InputGroup getControllKey() const;

	static NoteType GetType(Note*);

	static double GetTimingFromJson(const JSON&, double);

	/// @brief tとノーツのタイミングの時間差を返す
	/// @param t 現在時間
	/// @return tとノーツのタイミングの時間差
	double timeDiff(double t) const;

	inline double calcX() const;
	inline double calcY(double, double) const;
	inline Vec2 calcPos(double, double) const;

	[[nodiscard]] virtual JudgeType update(double) = 0;

	virtual void draw(double, double) const = 0;
};

struct TapNote : Note {
	TapNote(int32, double, double);
	
	[[nodiscard]] JudgeType update(double) override;

	void draw(double, double) const override;
};

struct HoldNote : Note {
	double length = .0;
	bool isHolding = false;

	/// @brief コンストラクタ
	/// @param lane どこのレーンか
	/// @param t 始点の時間ms
	/// @param length 終点の時間ms
	/// @param speed 速度
	HoldNote(int32 lane, double t, double length, double speed);

	JudgeType getJudge(double, double) const;

	[[nodiscard]] JudgeType update(double) override;

	void draw(double t, double) const override;
};

struct StayNote : Note {
	StayNote(int32 lane, double t, double sp);

	[[nodiscard]] JudgeType update(double t) override;

	void draw(double t, double speed) const override;
};
