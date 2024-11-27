#pragma once

RectF CrawLingText(const Font& font, const String& text, const RectF& region, ColorF color = Palette::White, int32 speed = 100, double t = Scene::Time()) {
	static constexpr int32 DefaultMarginX = 100;

	// MSDF フォントの描画のための設定
	const ScopedCustomShader2D shader{ Font::GetPixelShader(font.method()) };

	// テキストの全体幅を取得
	const double textRegion = Math::Max(font(text).region().w + DefaultMarginX, region.w);

	// 現在のオフセットを計算
	double offset = Math::Fmod(t * speed, textRegion);

	// 描画開始位置
	Vec2 startPos = region.rightCenter().movedBy(-offset, 0);

	// グリフを取得
	const Array<Glyph>& glyphs = font.getGlyphs(text);

	// テキストの描画
	Vec2 penPos = startPos;
	for (const auto& glyph : glyphs) {
		Vec2 glyphPos = Math::Round(penPos + glyph.getOffset());

		if (region.leftX() <= glyphPos.x && glyphPos.x + glyph.xAdvance <= region.rightX()) {
			glyph.texture.draw(glyphPos.movedBy(0, -glyph.yAdvance / 2), color);
		}

		penPos.x += glyph.xAdvance;

		// ペン位置が領域を超えた場合、ループさせる
		if (region.rightX() < penPos.x) {
			penPos.x -= textRegion;
		}
	}

	return region;
}
