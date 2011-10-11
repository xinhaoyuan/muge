#ifndef __MAP_HPP__
#define __MAP_HPP__

#include <SDL/SDL.h>
#include <deque>
#include <map>

#include "sprite.hpp"
#include "logic/object.hpp"

namespace Game
{

	class MapTiles
	{
		int mTPL;
		SDL_Surface *mTileTexture;
		
	public:
		int mWidth;
		int mHeight;
		int mLHeight;
		int mTransR;
		int mTransG;
		int mTransB;
		
		static MapTiles *Load(const char *confname);
		void Show(int id, GameEngine::tick_t tick, SDL_Surface *surface, SDL_Rect *rect);
	};

	class TileNode
	{
	private:
		int mXIdx;
		int mYIdx;
		int mZIdx;

		friend class TileMap;
		friend class Map;

	public:

		Sprite *mSprite;

		int mX;
		int mY;
		int mZ;

		int mW, mH, mDX, mDY;
		
		std::deque<TileNode *>::iterator mIt;
	};
	
	class TileMap
	{
	public:
		std::map< int, std::map< int, std::map< int, std::deque<TileNode *> > > >
		mMap;
		
		std::deque<TileNode *> *Get(int x, int y, int z);		
		std::deque<TileNode *> *Touch(int x, int y, int z);
		void Remove(int x, int y, int z);
		~TileMap(void);
	};

	class MapSprite;
	
	class Map
	{
	private:

		TileMap    mTileMap;
		MapSprite *mMapSprite;

		MapTiles  *mMapTiles;

	public:

		TileNode *AddSprite(Sprite *sprite, int x, int y, int z, int w, int h, int dx, int dy);
		void      UpdateSprite(TileNode *sprite);
		void      RemoveSprite(TileNode *sprite);
		
		static Map *Load(const char *filename);
		void Show(GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect, int x, int y, int w, int h);	
		~Map(void);
	};
	
}

#endif
