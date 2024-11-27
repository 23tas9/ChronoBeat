#pragma once
#include <Siv3D.hpp>

struct SemVer {
	size_t	major,
		minor,
		patch;

	StringView	m_preRelease,
		m_buildMetaData;

	/// @brief セマンティックバージョニング構造体
	/// @param _major メジャーバージョン
	/// @param _minor マイナーバージョン
	/// @param _patch パッチバージョン
	/// @param pre_release プレリリース(α, β)
	/// @param build_meta_data ビルドメタデータ
	constexpr SemVer(size_t _major, size_t _minor, size_t _patch, StringView pre_release = U"", StringView build_meta_data = U"") :
		major{ _major },
		minor{ _minor },
		patch{ _patch },
		m_preRelease{ pre_release },
		m_buildMetaData{ build_meta_data } {}

	constexpr SemVer(const std::array<size_t, 3>& version, StringView pre_release = U"", StringView build_meta_data = U"") :
		major{ version[0] },
		minor{ version[1] },
		patch{ version[2] },
		m_preRelease{ pre_release },
		m_buildMetaData{ build_meta_data } {}

	size_t at(size_t index) {
		if (index == 0) return major;
		else if (index == 1) return minor;
		else if (index == 2) return patch;
		else throw Error{ U"'index' is out of range!" };
	}

	size_t at(size_t index) const {
		if (index == 0) return major;
		else if (index == 1) return minor;
		else if (index == 2) return patch;
		else throw Error{ U"'index' is out of range!" };
	}

	size_t operator[](size_t index) {
		return at(index);
	}

	size_t operator[](size_t index) const {
		return at(index);
	}

	String toStr() const {
		return U"v{}.{}.{}{}{}"_fmt(
			major, minor, patch,
			m_preRelease.isEmpty() ? U"" : (U"-" + m_preRelease),
			m_buildMetaData.isEmpty() ? U"" : (U"+" + m_buildMetaData)
		);
	}
	String withTitle(StringView title) const {
		return title + U" - " + toStr();
	}

	// s3dのConsole, Printなどと連携
	friend detail::Console_impl& operator<<(detail::Console_impl& console, const SemVer& version) {
		console << version.toStr();
		return console;
	}
};
