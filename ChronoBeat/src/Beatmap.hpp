#pragma once
#include "Note.hpp"

struct Beatmap {
	JSON json = nullptr;

	double bpm = 0.0;
	double offset = 0.0;
	double length = 0.0;

	Array<std::shared_ptr<Note>> notes;

	size_t maxCombo = 0;

	Beatmap() = default;

	Beatmap(const FilePath& path, double _length, bool timingOffset = true) : json{ JSON::Load(Resource(path)) }, length{ _length } {
		bpm = json[U"BPM"].get<double>();

		offset = (json[U"offset"].get<double>() / 1000) +
			timingOffset ? ((60.0 / bpm) * 4) : 0.0;

		for (auto && obj : json[U"notes"].arrayView()) {
			//Console << obj;
			NoteType type = static_cast<NoteType>(obj[U"type"].get<int32>() - 1);
			int32 lane = obj[U"block"].get<int32>();

			double timing = Note::GetTimingFromJson(obj, bpm);

			if (type == NoteType::Hold) {
				maxCombo += 2;
				notes << Note::Make(type, lane, offset + timing, Note::GetTimingFromJson(obj[U"notes"][0], bpm) - timing, 1.0);
			}
			else {
				maxCombo += 1;
				notes << Note::Make(type, lane, offset + timing, 1.0);
			}
		}
	}

	operator bool() const {
		return not json.isEmpty();
	}
};
