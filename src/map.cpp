#include <fstream>
#include <string>

#include "map.hpp"
#include "resource.hpp"

namespace Game
{
	MapTiles *
	MapTiles::Load(const char *name)
	{
		MapTiles *result = new MapTiles;
		
		std::string filename;
		std::ifstream conf(name);

		conf >> result->mWidth;
		conf >> result->mHeight;
		conf >> result->mLHeight;
		conf >> result->mTPL;

		conf >> filename;

		SDL_RWops *file = SDL_RWFromFile(filename.c_str(), "rb");
		int png = IMG_isPNG(file);
		SDL_Surface *suf = IMG_Load_RW(file, 1);
		result->mTileTexture = suf;

		if (png)
		{
			SDL_SetColorKey(result->mTileTexture, SDL_SRCCOLORKEY,
							result->mTileTexture->format->colorkey); 
		}
		else
		{
			conf >> result->mTransR;
			conf >> result->mTransG;
			conf >> result->mTransB;

			SDL_SetColorKey(result->mTileTexture, SDL_SRCCOLORKEY,
							SDL_MapRGB(result->mTileTexture->format, result->mTransR, result->mTransG, result->mTransB));
		}

		return result;
	}

	void
	MapTiles::Show(int id, GameEngine::tick_t tick, SDL_Surface *surface, SDL_Rect *rect)
	{
		SDL_Rect src_rect;

		src_rect.x = id % mTPL * mWidth;
		src_rect.y = id / mTPL * (mHeight + mLHeight);
		src_rect.w = mWidth;
		src_rect.h = mHeight + mLHeight;

		SDL_BlitSurface(mTileTexture, &src_rect, surface, rect);
	}

	class MapSprite : public Sprite
	{
		
		int mId;
		MapTiles *mTiles;
	public:
		MapSprite(int id, MapTiles *tiles) : mId(id), mTiles(tiles) { }
		
		void
		Set(int id, MapTiles *tiles) { mId = id; mTiles = tiles; }
	
		virtual void
		Show(int state, GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect) {
			mTiles->Show(mId, tick, screen, rect);
		}
	};


	std::deque<TileNode *> *
	TileMap::Get(int x, int y, int z)
	{
		if (mMap.find(z) == mMap.end())
			return NULL;
		if (mMap[z].find(y) == mMap[x].end())
			return NULL;
		if (mMap[z][y].find(x) == mMap[x][y].end())
			return NULL;
		return &mMap[z][y][x];
	}
	
	std::deque<TileNode *> *
	TileMap::Touch(int x, int y, int z)
	{
		return &mMap[z][y][x];
	}
	
	void
	TileMap::Remove(int x, int y, int z)
	{
		mMap[z][y].erase(x);
		if (mMap[z][y].empty())
		{
			mMap[z].erase(y);

			if (mMap[z].empty())
			{
				mMap.erase(z);
			}
		}
	}
	
	TileMap::~TileMap(void)
	{
		std::map<int, std::map<int, std::map<int, std::deque<TileNode *> > > >::iterator
			itH = mMap.begin();

		while (itH != mMap.end())
		{
			std::map<int, std::map<int, std::deque<TileNode *> > >::iterator
				itY = itH->second.begin();

			while (itY != itH->second.end())
			{
				std::map<int, std::deque<TileNode *> >::iterator
					itX = itY->second.begin();
				
				while (itX != itY->second.end())
				{
					std::deque<TileNode *>::iterator _it = itX->second.begin();

					while (_it != itX->second.end())
					{
						TileNode *node = *_it;
						delete node;
						++ _it;
					}

					++ itX;
				}

				++ itY;
			}

			++ itH;
		}
	}

	static inline int
	DivDown(int i, int unit)
	{
		if (i < 0)
			return i = i / unit -1;
		else return i = i / unit;
	}

	TileNode *
	Map::AddConstantSprite(Sprite *sprite, int x, int y, int z, int w, int h, int dx, int dy)
	{
		TileNode *node = new TileNode;
		node->mSprite = sprite;

		node->mX = x;
		node->mY = y;
		node->mZ = z;

		node->mState = -1;
		node->mMotion = NULL;
		
		node->mW = w;
		node->mH = h;

		node->mDX = dx;
		node->mDY = dy;

		node->mXIdx = DivDown(x + dx - 1, mMapTiles->mWidth);
		node->mYIdx = DivDown(y + dy - z - 1, mMapTiles->mHeight);
		node->mZIdx = DivDown(z, mMapTiles->mLHeight);
		std::deque<TileNode *> *q = mTileMap.Touch(node->mXIdx, node->mYIdx, node->mZIdx);
		node->mIt = q->insert(q->end(), node);

		return node;
	}

	TileNode *
	Map::AddMotiveSprite(Sprite *sprite, int w, int h, int dx, int dy)
	{
		TileNode *node = new TileNode;
		node->mSprite = sprite;

		node->mState = -1;
		node->mMotion = new TileNode::Motion;
		node->mMotion->mInitialized = false;
		node->mMotion->mMotionIt = mMotionList.insert(mMotionList.end(), node);
		
		node->mW = w;
		node->mH = h;

		node->mDX = dx;
		node->mDY = dy;
		
		return node;
	}

	void
	Map::UpdateSprite(TileNode *node)
	{   	
		int xIdx = DivDown(node->mX + node->mDX - 1, mMapTiles->mWidth);
		int yIdx = DivDown(node->mY + node->mDY - node->mZ - 1, mMapTiles->mHeight);
		int zIdx = DivDown(node->mZ, mMapTiles->mLHeight);

		std::deque<TileNode *> *q;
		
		if (!node->mMotion ||
			node->mMotion->mInitialized)
		{
			if (xIdx == node->mXIdx &&
				yIdx == node->mYIdx &&
				zIdx == node->mZIdx) return;
			
			q = mTileMap.Get(node->mXIdx, node->mYIdx, node->mZIdx);
			q->erase(node->mIt);
			
			if (q->empty())
				mTileMap.Remove(node->mXIdx, node->mYIdx, node->mZIdx);
		}

		node->mXIdx = xIdx;
		node->mYIdx = yIdx;
		node->mZIdx = zIdx;
		
		q = mTileMap.Touch(node->mXIdx, node->mYIdx, node->mZIdx);
		node->mIt = q->insert(q->end(), node);

		if (node->mMotion)
			node->mMotion->mInitialized = true;
	}

	void
	Map::RemoveSprite(TileNode *node)
	{
		std::deque<TileNode *> *q = mTileMap.Touch(node->mXIdx, node->mYIdx, node->mZIdx);
		q->erase(node->mIt);

		if (q->empty())
			mTileMap.Remove(node->mXIdx, node->mYIdx, node->mZIdx);

		if (node->mMotion)
		{
			mMotionList.erase(node->mMotion->mMotionIt);
		}

		delete node;
	}

	Map *
	Map::Load(const char *name)
	{
		Map *result = new Map();
		
		std::ifstream fin(name);
		int w, h;
		fin >> w >> h;
		std::string tilesname;
		fin >> tilesname;


		result->mMapTiles  = Resource::Get<MapTiles>(tilesname.c_str());

		int i, j, c = 0, d, cur_h;
		for (j = 0; j != h; ++ j)
		{
			for (i = 0; i != w; ++ i)
			{
				fin >> d;
				for (cur_h = 0; cur_h < d; ++ cur_h)
				{
					int id;
					fin >> id;

					if (id == -1) continue;

					result->mMapSprite.push_back(new MapSprite(id, result->mMapTiles));
					result->AddConstantSprite(result->mMapSprite[result->mMapSprite.size() - 1],
									  i * result->mMapTiles->mWidth,
									  j * result->mMapTiles->mHeight,
									  (cur_h - 1) * result->mMapTiles->mLHeight,
									  result->mMapTiles->mWidth,
									  result->mMapTiles->mHeight + result->mMapTiles->mLHeight,
									  result->mMapTiles->mWidth - 1,
									  result->mMapTiles->mHeight + result->mMapTiles->mLHeight - 1)->mIsMapTile = true;
					
				}
			}
		}

		return result;
	}

	void
	Map::UpdateMotion(GameEngine::tick_t tick)
	{
		std::deque<TileNode *>::iterator it = mMotionList.begin();
		while (it != mMotionList.end())
		{
			TileNode *node = *it;

			node->mX = node->mMotion->mXMotion.Get(tick);
			node->mY = node->mMotion->mYMotion.Get(tick);
			node->mZ = node->mMotion->mZMotion.Get(tick);
			
			UpdateSprite(node);

			++ it;
		}
	}
	
	void
	Map::Show(GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect, int x, int y, int w, int h)
	{
		SDL_Rect _rect;
		std::map<int, std::map<int, std::map<int, std::deque<TileNode *> > > >::iterator
			itH = mTileMap.mMap.begin();

		rect->x -= x;
		rect->y -= y;

		while (itH != mTileMap.mMap.end())
		{
			std::map<int, std::map<int, std::deque<TileNode *> > >::iterator
				itY = itH->second.begin();

			while (itY != itH->second.end())
			{
				std::map<int, std::deque<TileNode *> >::iterator
					itX = itY->second.begin();
				
				while (itX != itY->second.end())
				{
					std::deque<TileNode *>::iterator _it = itX->second.begin();

					while (_it != itX->second.end())
					{
						TileNode *node = *_it;

						++ _it;
						
						if (node->mX >= x + w            ||
							node->mY - node->mZ >= y + h ||
							node->mX + node->mW < x      ||
							node->mY + node->mH - node->mZ < y) continue;

						_rect.x = rect->x + node->mX;
						_rect.y = rect->y + node->mY - node->mZ;
						node->mSprite->Show(node->mState, tick, screen, &_rect);
					}

					++ itX;
				}

				++ itY;
			}

			++ itH;
		}
	}

	Map::~Map(void)
	{
		int i;
		for (i = 0; i != mMapSprite.size(); ++ i)
			delete mMapSprite[i];
	}
}
