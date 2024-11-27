#pragma once

struct {
	const FilePath path = U"config.ini";

	INI file{ path };

	template<typename T>
	T getValue(const String& name, T defaultValue) const {
		Optional<T> value = ParseOpt<T>(file[name]);

		if (not value.has_value())
			return defaultValue;

		return value.value();
	}

	template<typename T>
	void setValue(const String& name, T value) {
		file[name] = value;
	}

	bool save() {
		return file.save(path);
	}
} Config;
