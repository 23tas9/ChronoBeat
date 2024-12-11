#include "Note.hpp"

/////////////////////////////
// Base Note
/////////////////////////////

Note::Note(int32 _lane, double t, double sp = 1.0) :
	lane{ _lane }, timing{ t }, speed{ sp } {

}

std::shared_ptr<Note> Note::Make(NoteType ntype, int32 lane, double t, double sp = 1.0) {
	if (ntype == NoteType::Tap) return std::make_shared<TapNote>(lane, t, sp);
	if (ntype == NoteType::Stay) return std::make_shared<StayNote>(lane, t, sp);
	if (ntype == NoteType::Hold) throw Error{ U"Hold note needs length argment" };

	return nullptr;
}

std::shared_ptr<Note> Note::Make(NoteType ntype, int32 lane, double t, double len, double sp = 1.0) {
	if (ntype != NoteType::Hold) throw Error{ U"!Hold note doesn't use length argment" };

	return std::make_shared<HoldNote>(lane, t, len, sp);
}

LaneType Note::getLaneType() const {
	return static_cast<LaneType>(lane);
}

JudgeType Note::getJudge(double t) const {
	const double diff = timeDiff(t);

	int32 minTiming = Globals::judgeTimings[JudgeType::Near];

	if (diff * 1000 < -minTiming) return JudgeType::Miss;

	const double adiff = Math::Abs(diff);
	minTiming += 1;

	JudgeType result = JudgeType::None;

	for (auto&& [type, millis] : Globals::judgeTimings) {
		if (minTiming <= millis) continue;

		if (adiff * 1000 <= millis) {
			minTiming = millis;
			result = type;
		}
	}

	return result;
}

InputGroup Note::getControllKey() const {
	return Globals::controllKeys[getLaneType()];
}

NoteType Note::GetType(Note* note) {
	if (dynamic_cast<TapNote*>(note) != nullptr) return NoteType::Tap;
	if (dynamic_cast<HoldNote*>(note) != nullptr) return NoteType::Hold;
	if (dynamic_cast<StayNote*>(note) != nullptr) return NoteType::Stay;

	return NoteType::Tap;
}

double Note::GetTimingFromJson(const JSON& json, double bpm) {
	const int32 lpb = json[U"LPB"].get<int32>();
	const int32 num = json[U"num"].get<int32>();

	double blockPerTime = 60.0 / bpm;

	return (blockPerTime / lpb) * num;
}

double Note::timeDiff(double t) const {
	return timing - t;
}

inline double Note::calcX() const {
	return Globals::laneStartX + (Note::NoteMergin / 2) + (Globals::laneWidth * lane);
}

inline double Note::calcY(double t, double scroll) const {
	return Globals::judgeLineY - (timeDiff(t) * Globals::defaultNoteSpeed) * (Globals::speed * scroll * speed);
}

inline Vec2 Note::calcPos(double t, double scroll) const {
	return Vec2{ calcX(), calcY(t, scroll) };
}

/////////////////////////////
// TapNote
/////////////////////////////

TapNote::TapNote(int32 lane, double t, double sp) : Note(lane, t, sp) {

}

JudgeType TapNote::update(double t) {
	JudgeType result = getJudge(t);

	if (result == JudgeType::Miss) return result;
	if (not getControllKey().down()) return JudgeType::None;

	if(result != JudgeType::None)
		isRemovable = true;

	return result;
}

void TapNote::draw(double t, double scroll) const {
	const Vec2 pos = calcPos(t, scroll);

	RectF rect{ pos.x, pos.y, Globals::laneWidth - Note::NoteMergin, Globals::noteHeight };

	rect.rounded(2).draw();
}

/////////////////////////////
// HoldNote
/////////////////////////////

HoldNote::HoldNote(int32 lane, double t, double len, double sp) : Note(lane, t, sp), length{ len } {

}

JudgeType HoldNote::getJudge(double t, double len = .0) const {
	const double diff = timeDiff(t - len);
	int32 minTiming = Globals::judgeTimings[JudgeType::Near];

	if (diff * 1000 < -minTiming) return JudgeType::Miss;

	const double adiff = Math::Abs(diff);
	minTiming += 1;
	
	JudgeType result = JudgeType::None;

	for (auto&& [type, millis] : Globals::judgeTimings) {
		if (minTiming <= millis) continue;

		if (adiff * 1000 <= millis) {
			minTiming = millis;
			result = type;
		}
	}

	return result;
}

JudgeType HoldNote::update(double t) {
	const InputGroup key = getControllKey();

	JudgeType result = getJudge(t);

	if (not isHolding) {
		if (result == JudgeType::Miss) return result;

		/*
		// 始点が通り過ぎたら無視
		if (timeDiff(t) < -Globals::judgeTimings[JudgeType::Near])
			return JudgeType::None;
		*/

		// 判定したなら holding
		if (key.down() && result != JudgeType::None) {
			isHolding = true;
			return result;
		}

		return JudgeType::None;
	}

	if (isHolding) {
		result = getJudge(t, length);

		// ホールド終点まで長押ししてるならPerfect
		if (result == JudgeType::Perfect) {
			isRemovable = true;
			return result;
		}

		// ホールド中にキーを離す
		if (key.up()) {
			isRemovable = true;

			// なにも判定できない = 早すぎる
			if (result == JudgeType::None) return JudgeType::Miss;

			return result;
		}
	}

	return JudgeType::None;
}
void HoldNote::draw(double t, double scroll) const {
	const Vec2 pos = calcPos(t, scroll);
	const double target = calcY(t - length, scroll);

	RectF rect{ pos.x, target + (Globals::noteHeight / 2), Globals::laneWidth - Note::NoteMergin, pos.y - target + (Globals::noteHeight / 2) };

	rect.rounded(2).draw(isHolding ? Palette::Gray : Palette::White);
}

/////////////////////////////
// StayNote
/////////////////////////////
StayNote::StayNote(int32 lane, double t, double sp) : Note(lane, t, sp) {

}

JudgeType StayNote::update(double t) {
	JudgeType result = getJudge(t);

	if (result == JudgeType::Miss) return result;

	if (not getControllKey().pressed()) return JudgeType::None;
	if (0 < timeDiff(t)) return JudgeType::None;

	if (result != JudgeType::None) {
		result = JudgeType::Perfect;
		isRemovable = true;
	}

	return result;
}

void StayNote::draw(double t, double scroll) const {
	const Vec2 pos = calcPos(t, scroll);

	RectF rect{ pos.x, pos.y, Globals::laneWidth - Note::NoteMergin, Globals::noteHeight };

	rect.rounded(2).draw(Palette::Yellow);
}
