#include <fstream>
#include <iostream>
#include <iconv.h>
#include <sstream>

#include "conv.hpp"
#include "resource.hpp"
#include "map.hpp"

namespace Game
{
	Font *
	Font::Load(const char *name)
	{
		const char *p = strchr(name, ':');
		if (p == NULL) return NULL;
		
		int size = atoi(p + 1);

		std::string filename(name, p - name);
		
		Font *font = new Font;
		font->mFont = TTF_OpenFont(filename.c_str(), size);

		return font;
	}

	void
	Font::SetStyle(int style)
	{
	}
	
	void
	Font::UnicodeTextSize(const char16_t *text, int *w, int *h)
	{
		TTF_SizeUNICODE(mFont, (const Uint16 *)text, w, h);
	}
	
	SDL_Surface *
	Font::UnicodeTextRender(const char16_t *text, SDL_Color color)
	{
		TTF_RenderUNICODE_Blended(mFont, (const Uint16 *)text, color);
	}

	int 
	Font::UnicodeTextMaxAdvance(const char16_t *text, int w)
	{
		int count = 0;
		int adv;
		
		while (w > 0)
		{
			if (text[count] == u'\0') break;
			TTF_GlyphMetrics(mFont, text[count], NULL, NULL, NULL, NULL, &adv);

			if (w < adv) break;
			w -= adv;
			++ count;
		}

		return count;
	}

	Conversation *
	Conversation::Load(const char *name)
	{
		std::ifstream fin(name);
		std::string fontname, boxtilename;
		int innerWidth, innerHeight;
		int outerWidth, outerHeight;
		
		fin >> innerWidth;
		fin >> innerHeight;
		fin >> boxtilename;
		fin >> fontname;

		MapTiles *tiles = Resource::Get<MapTiles>(boxtilename.c_str());

		outerWidth = (((innerWidth - 1) / tiles->mWidth) + 3);
		outerHeight = (((innerHeight - 1) / tiles->mHeight) + 3);

		SDL_Surface *bg = SDL_CreateRGBSurface(SDL_SWSURFACE,
											   outerWidth * tiles->mWidth, outerHeight * tiles->mHeight,
											   32, 0, 0, 0, 0);
		SDL_SetColorKey(bg, SDL_SRCCOLORKEY,
						SDL_MapRGB(bg->format, tiles->mTransR, tiles->mTransG, tiles->mTransB));

		SDL_Rect r;
		int i, j;
		for (i = 0; i < outerWidth; ++ i)
			for (j = 0; j < outerHeight; ++ j)
			{
				r.x = i * tiles->mWidth;
				r.y = j * tiles->mHeight;
				int u, v;

				if (i == 0) u = 0;
				else if (i == outerWidth - 1) u = 2;
				else u = 1;

				if (j == 0) v = 0;
				else if (j == outerHeight - 1) v = 2;
				else v = 1;

				tiles->Show(v * 3 + u, 0, bg, &r);
			}
		
		std::string line;
		std::u16string text;

		static iconv_t cv = iconv_open("UNICODE", "UTF-8");
		std::vector<char16_t> cvt_buf;

		while (!std::getline(fin, line).eof())
		{
			if (line.length() == 0)
			{
				if (text.length() > 0) break;
				else continue;
			}
			
			cvt_buf.resize(line.length());

			const char *in = line.c_str();
			size_t in_length = line.length();
			char16_t *out  = &cvt_buf[0];
			size_t out_length = in_length * sizeof(char16_t);
			
			iconv(cv, (char **)&in, &in_length, (char **)&out, &out_length);
			text += std::u16string(&cvt_buf[0], out - &cvt_buf[0]);
		}

		Font *font = Resource::Get<Font>(fontname.c_str());

		int cur_idx = 0;
		int page_count = 0;
		int height_count = 0;

		Conversation *conv = new Conversation;
		conv->mInnerWidth = innerWidth;
		conv->mInnerHeight = innerHeight;
		conv->mOuterWidth = outerWidth * tiles->mWidth;
		conv->mOuterHeight = outerHeight * tiles->mHeight;
		conv->mBgSur = bg;
		
		SDL_Surface *sur = NULL;
		r.x = 0;
		while (cur_idx < text.length())
		{
			int l;
			l = font->UnicodeTextMaxAdvance(text.c_str() + cur_idx, innerWidth);
			std::u16string tmp(text.c_str() + cur_idx, l);
			
			int w, h;
			font->UnicodeTextSize(tmp.c_str(), &w, &h);
			if (height_count + h > innerHeight)
			{
				conv->mPages.push_back(sur);
				sur = NULL;
				
				height_count = 0;
				++ page_count;
			}
			else
			{
				if (sur == NULL)
				{
					sur = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, innerWidth, innerHeight, 32,
											   0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);					
				}
				r.y = height_count;
				SDL_Surface *sur_line = font->UnicodeTextRender(tmp.c_str(), {255, 255, 255});
				SDL_SetAlpha(sur_line, 0, 0);
				SDL_BlitSurface(sur_line, NULL, sur, &r);
				SDL_FreeSurface(sur_line);
				
				height_count += h;
			}

			cur_idx += l;
		}

		if (sur != NULL)
			conv->mPages.push_back(sur);

		conv->mPage = -1;
		return conv;
	}

	bool
	Conversation::SetPage(int page) {
		if (page > mPages.size()) return false;
		mPage = page;
		return true;
	}
	
	void
	Conversation::Show(GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect)
	{
		if (mPage == -1) return;

		rect->x = (640 - mOuterWidth) / 2;
		rect->y = (480 - mOuterHeight) - 10;

		SDL_BlitSurface(mBgSur, NULL, screen, rect);

		rect->x = (640 - mInnerWidth) / 2;
		rect->y = (480 - (mInnerHeight + mOuterHeight) / 2) - 10;
		
		SDL_BlitSurface(mPages[mPage], NULL, screen, rect);
	}
}
