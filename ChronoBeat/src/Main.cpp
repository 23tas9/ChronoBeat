#include <Siv3D.hpp> // v0.6.14
#include "Globals.hpp"
#include "Scene/Common.hpp"

#include "SongInfo.hpp"

#include "Scene/TitleScene.hpp"
#include "Scene/SelectScene.hpp"
#include "Scene/SettingScene.hpp"
#include "Scene/GameScene.hpp"
#include "Scene/ResultScene.hpp"

#include "LoadingCircle.hpp"
#include "LeaderBoard.hpp"
#include "_environment.hpp"

void Main() {
	Window::SetTitle(Globals::gameVersion.withTitle(Globals::gameTitle));

	///////////////////
	// Asset register
	///////////////////

	// font
	if (not FontAsset::Register(U"Font.UI.Title", 128, Globals::Fonts::cinecaption, FontStyle::Default))
		throw AssetRegistError(U"Font.UI.Title");
	if (not FontAsset::Register(U"Font.UI.SubTitle", 64, Globals::Fonts::mamelon, FontStyle::Default))
		throw AssetRegistError(U"Font.UI.SubTitle");
	if (not FontAsset::Register(U"Font.UI.Detail", 24, Globals::Fonts::mamelon, FontStyle::Default))
		throw AssetRegistError(U"Font.UI.Detail");
	if (not FontAsset::Register(U"Font.UI.Normal", 32, Globals::Fonts::mamelon, FontStyle::Default))
		throw AssetRegistError(U"Font.UI.Detail");

	if (not FontAsset::Register(U"Font.UI.SongTitle", FontMethod::SDF, 48, Globals::Fonts::cinecaption, FontStyle::Default))
		throw AssetRegistError(U"Font.UI.SongTitle");
	if (not FontAsset::Register(U"Font.UI.SongSub", 28, Globals::Fonts::cinecaption, FontStyle::Default))
		throw AssetRegistError(U"Font.UI.SongSub");
	if (not FontAsset::Register(U"Font.UI.Result.1", 160, Globals::Fonts::cinecaption, FontStyle::Default))
		throw AssetRegistError(U"Font.UI.Result.1");
	if (not FontAsset::Register(U"Font.UI.Result.2", 128, Globals::Fonts::cinecaption, FontStyle::Default))
		throw AssetRegistError(U"Font.UI.Result.2");

	if (not FontAsset::Register(U"Font.Game.Judge.1", FontMethod::SDF, 32, Globals::Fonts::mamelon, FontStyle::Default))
		throw AssetRegistError(U"Font.Game.Judge.1");
	if (not FontAsset::Register(U"Font.Game.Combo", FontMethod::SDF, 48, Globals::Fonts::mamelon, FontStyle::Default))
		throw AssetRegistError(U"Font.Game.Combo");
	if (not FontAsset::Register(U"Font.Game.Judge.2", FontMethod::SDF, 64, Globals::Fonts::mamelon, FontStyle::Default))
		throw AssetRegistError(U"Font.Game.Judge.2");

	// audio
	if (not AudioAsset::Register(U"Audio.UI.BGM", Globals::Sounds::BGM, Loop::Yes))
		throw AssetRegistError(U"Audio.UI.BGM");
	if (not AudioAsset::Register(U"Audio.UI.Title", Globals::Sounds::pendulumClock, Loop::Yes))
		throw AssetRegistError(U"Audio.UI.Title");
	if (not AudioAsset::Register(U"Audio.UI.MoveCursor", Globals::Sounds::moveCursor))
		throw AssetRegistError(U"Audio.UI.MoveCursor");
	if (not AudioAsset::Register(U"Audio.UI.SongSelected", Globals::Sounds::songSelected))
		throw AssetRegistError(U"Audio.UI.SongSelected");
	if (not AudioAsset::Register(U"Audio.Game.NoteClick", Globals::Sounds::noteClick))
		throw AssetRegistError(U"Audio.Game.NoteClick");
	if (not AudioAsset::Register(U"Audio.Game.Metronome", Globals::Sounds::metronome))
		throw AssetRegistError(U"Audio.Game.Metronome");

	// texture
	if (not TextureAsset::Register(U"Tex.UI.Setting", 0xf013_icon, 80))
		throw AssetRegistError(U"Tex.UI.Setting");

	// アドオンの登録
	Addon::Register<LoadingCircleAddon>(U"LoadingCircleAddon");

	//////////////
	// game init
	//////////////
	
	// シーン登録
	App manager;
	manager
		.add<TitleScene>(SceneState::Title)
		.add<SelectScene>(SceneState::Select)
		.add<SettingScene>(SceneState::Setting)
		.add<GameScene>(SceneState::Game)
		.add<ResultScene>(SceneState::Result);

	manager.init(SceneState::Title, Globals::sceneTransitionTime);

	HashTable<String, AsyncHTTPTask> rankingRequests;

	/////////////////////
	// songs and jacket
	/////////////////////
	const JSON infoJson = JSON::Load(Resource(U"songinfo.json"));
	for (const JSON& data : infoJson.arrayView()) {
		SongInfo info{ data };

		Globals::songInfos << info.registerAsset();

		rankingRequests[info.title] = LeaderBoard::CreateGetTask(Environment::LeaderboardURLRaw, info.title, 5);
		Globals::records[info.title] = {};
	}

#if SIV3D_BUILD(DEBUG)
	Console << U"Registered";
	Console << Globals::songInfos;
#endif

	Window::Resize(Globals::windowSize);
	Scene::SetResizeMode(ResizeMode::Keep);

	Window::SetFullscreen(true);

	Scene::SetBackground(Globals::Theme::backgroundBase);

	while (System::Update()) {
		if (not manager.update()) break;

		// ランキング読み込み
		if (0 < rankingRequests.size()) {
			for (auto it = rankingRequests.begin(); it != rankingRequests.end();) {
				auto& title = it->first;
				auto& request = it->second;

				if (not request.isReady()) {
					++it;
					continue;
				}

				if (const auto response = request.getResponse(); response.isOK()) {
					if (not LeaderBoard::ReadLeaderboard(request.getAsJSON(), Globals::records[title])) {
						Print << U"Failed to read the leaderboard.";
					}
#if SIV3D_BUILD(DEBUG)
					else {
						Console << U"Request completed: " << title;

						for (auto&& record : Globals::records[title]) {
							Console << U"{}: {}"_fmt(record.userName, record.score);
						}
					}
#endif

					it = rankingRequests.erase(it);
				}
				else {
					++it;
				}
			}
		}

		// 映画のように線が出るように
		int32 randomLineX = Random(Globals::windowSize.x);
		Scene::Rect().left().movedBy(randomLineX, 0).draw(Palette::Black.withAlpha(153));

		Circle{ Scene::Center(), Globals::windowSize.x }
			.draw(ColorF{ .0, .0 }, Palette::Black.withAlpha(128));
	}
}
