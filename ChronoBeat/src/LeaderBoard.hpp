#pragma once
#include <Siv3D.hpp>

namespace LeaderBoard {
	/// @brief レコード
	struct Record {
		/// @brief ユーザー名
		String userName;

		/// @brief スコア
		double score;
	};

	/// @brief 有効なレコードかどうかをチェックします。
	/// @param value レコードが格納された JSON
	/// @return 有効なレコードなら true, そうでなければ false
	inline bool IsValidRecord(const JSON& value) {
		return (value.isObject()
			&& value.hasElement(U"username")
			&& value.hasElement(U"score")
			&& value[U"username"].isString()
			&& value[U"score"].isNumber());
	}

	/// @brief JSON データをリーダーボードとして読み込みます。
	/// @param json JSON データ
	/// @param dst 更新するリーダーボード
	/// @remark 読み込みに失敗した場合、dst は更新されません。
	/// @return 読み込みに成功したら true, 失敗したら false
	inline bool ReadLeaderboard(const JSON& json, Array<Record>& dst) {
		if (not json.isArray()) {
			return false;
		}

		Array<Record> leaderboard;

		for (auto&& [key, value] : json) {
			if (not IsValidRecord(value))
				continue;

			Record record;
			record.userName = value[U"username"].get<String>();
			record.score = value[U"score"].get<double>();

			leaderboard << std::move(record);
		}

		dst = std::move(leaderboard);
		return true;
	}

	/// @brief サーバからリーダーボードを取得するタスクを作成します。
	/// @param url サーバの URL
	/// @param count 取得上限数
	/// @return タスク
	inline AsyncHTTPTask CreateGetTask(const URLView url, const StringView sheetname, int32 count = 5)
	{
		// GET リクエストの URL を作成する
		const URL requestURL = U"{}?sheet={}&count={}"_fmt(url, sheetname, count);

		return SimpleHTTP::GetAsync(requestURL, {});
	}

	/// @brief サーバにスコアを送信するタスクを作成します。
	/// @param url サーバの URL
	/// @param userName ユーザー名
	/// @param score スコア
	/// @param additionalData 追加の情報
	/// @return タスク
	inline AsyncHTTPTask CreatePostTask(const URLView url, const StringView sheetname, const StringView userName, double score)
	{
		// POST リクエストの URL を作成する
		URL requestURL = U"{}?sheet={}&username={}&score={}"_fmt(url, sheetname, PercentEncode(userName), PercentEncode(Format(score)));

		const HashTable<String, String> headers = {
			{ U"Content-Type", U"application/x-www-form-urlencoded; charset=UTF-8" }
		};

		return SimpleHTTP::PostAsync(requestURL, headers, nullptr, 0);
	}

	/// @brief リーダーボードから SimpleTable を作成します。
	/// @param leaderboard リーダーボード
	/// @return SimpleTable
	inline SimpleTable ToTable(const Array<Record>& leaderboard)
	{
		SimpleTable table{ { 100, 260, 140 } };

		// ヘッダー行を追加する
		table.push_back_row({ U"Rank", U"Player Name", U"Score" }, { 0, 0, 0 });
		table.setRowBackgroundColor(0, ColorF{ 0.92 });

		// 順位
		int32 rank = 1;

		// リーダーボードの内容を追加する
		for (auto& record : leaderboard)
		{
			table.push_back_row({ Format(rank++), record.userName, Format(record.score) });
		}

		return table;
	}
}
