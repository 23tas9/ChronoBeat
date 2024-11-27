#pragma once

struct AssetRegistError : Error {
	AssetRegistError(const String& name) : Error(U"Asset registration failure\nname: {}"_fmt(name)) {}
};
