#pragma once

enum class JudgeType : int32 {
	Perfect,
	Great,
	Near,
	Miss,
	None
};

inline String GetJudgeName(const JudgeType& type) {
	if (type == JudgeType::Perfect) return U"Perfect";
	if (type == JudgeType::Great) return U"Great";
	if (type == JudgeType::Near) return U"Near";
	if (type == JudgeType::Miss) return U"Miss";

	return U"None";
}
