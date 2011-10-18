#ifndef __MAP_HPP__
#define __MAP_HPP__

#include <SDL/SDL.h>
#include <deque>
#include <map>
#include <vector>

#include "sprite.hpp"
#include "logic/object.hpp"
#include "motion.hpp"

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

		std::deque<TileNode *>::iterator mIt;

		friend class TileMap;
		friend class Map;

	public:

		class Motion
		{
			friend class TileMap;
			friend class Map;
		private:
			std::deque<TileNode *>::iterator mMotionIt;

		public:
			bool mInitialized;			
			Game::Motion mXMotion;
			Game::Motion mYMotion;
			Game::Motion mZMotion;			
		} *mMotion;

		TileNode(void) : mIsMapTile(false), mMotion(NULL) { }
		~TileNode(void) { if (mMotion) delete mMotion; }

		Sprite *mSprite;
		bool mIsMapTile;
		
		int mState;
		int mX, mY, mZ;
		int mW, mH, mDX, mDY;
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

		std::deque<TileNode *>   mMotionList;
		std::vector<MapSprite *> mMapSprite;
		MapTiles                *mMapTiles;

	public:

		TileMap    mTileMap;
		
		TileNode *AddConstantSprite(Sprite *sprite, int x, int y, int z, int w, int h, int dx, int dy);
		TileNode *AddMotiveSprite(Sprite *sprite, int w, int h, int dx, int dy);
		void      UpdateSprite(TileNode *sprite);
		void      RemoveSprite(TileNode *sprite);
		
		static Map *Load(const char *filename);
		void UpdateMotion(GameEngine::tick_t tick);
		void Show(GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect, int x, int y, int w, int h);	
		~Map(void);
	};
	
}

#endif
