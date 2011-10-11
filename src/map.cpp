#include <fstream>
#include "map.hpp"

namespace Game
{
	int MapTiles::sTPL;
	int MapTiles::sWidth;
	int MapTiles::sHeight;
	int MapTiles::sLHeight;
	SDL_Surface *MapTiles::sTileTexture;

	void
	MapTiles::LoadResources(const char *confname)
	{
		std::string filename;
		std::ifstream conf(confname);

		conf >> sWidth;
		conf >> sHeight;
		conf >> sLHeight;
		conf >> sTPL;

		conf >> filename;
		
		SDL_Surface *suf = SDL_LoadBMP(filename.c_str());
		sTileTexture = SDL_DisplayFormat(suf);
		SDL_FreeSurface(suf);
	}

	void
	MapTiles::UnloadResources(void)
	{
		SDL_FreeSurface(sTileTexture);
	}

	void
	MapTiles::Show(int id, GameEngine::tick_t tick, SDL_Surface *surface, SDL_Rect *rect)
	{
		SDL_Rect src_rect;

		src_rect.x = id % sTPL * sWidth;
		src_rect.y = id / sTPL * (sHeight + sLHeight);
		src_rect.w = sWidth;
		src_rect.h = sHeight + sLHeight;

		SDL_BlitSurface(sTileTexture, &src_rect, surface, rect);
	}

	class MapSprite : public Sprite
	{
		int mId;
	public:
		void
		SetId(int id) { mId = id; }
	
		void
		Show(GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect) {
			MapTiles::Show(mId, tick, screen, rect);
		}

		void
		ShowWithBR(GameEngine::tick_t tick, SDL_Surface *screen, SDL_Rect *rect) {
			rect->x -= MapTiles::sWidth  - 1;
			rect->y -= MapTiles::sHeight - 1;
			Show(tick, screen, rect);
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
						delete *_it;
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
	Map::AddSprite(Sprite *sprite, int x, int y, int z, int w, int h, int dx, int dy)
	{
		TileNode *node = new TileNode;
		node->mSprite = sprite;

		node->mX = x;
		node->mY = y;
		node->mZ = z;
		
		node->mW = w;
		node->mH = h;

		node->mDX = dx;
		node->mDY = dy;

		node->mXIdx = DivDown(x + dx - 1, MapTiles::sWidth);
		node->mYIdx = DivDown(y + dy - z - 1, MapTiles::sHeight);
		node->mZIdx = DivDown(z, MapTiles::sLHeight);
		std::deque<TileNode *> *q = mTileMap.Touch(node->mXIdx, node->mYIdx, node->mZIdx);
		node->mIt = q->insert(q->end(), node);

		return node;
	}

	void
	Map::UpdateSprite(TileNode *node)
	{
		std::deque<TileNode *> *q = mTileMap.Get(node->mXIdx, node->mYIdx, node->mZIdx);
		q->erase(node->mIt);

		if (q->empty())
			mTileMap.Remove(node->mXIdx, node->mYIdx, node->mZIdx);

		node->mXIdx = DivDown(node->mX + node->mDX - 1, MapTiles::sWidth);
		node->mYIdx = DivDown(node->mY + node->mDY - node->mZ - 1, MapTiles::sHeight);
		node->mZIdx = DivDown(node->mZ, MapTiles::sLHeight);
		
		q = mTileMap.Touch(node->mXIdx, node->mYIdx, node->mZIdx);
		node->mIt = q->insert(q->end(), node);
	}

	void
	Map::RemoveSprite(TileNode *node)
	{
		std::deque<TileNode *> *q = mTileMap.Touch(node->mXIdx, node->mYIdx, node->mZIdx);
		q->erase(node->mIt);

		if (q->empty())
			mTileMap.Remove(node->mXIdx, node->mYIdx, node->mZIdx);

		delete node;
	}

	void
	Map::Load(const char *filename)
	{
		std::ifstream fin(filename);
		int w, h, count;
		fin >> w >> h >> count;

		mMapSprite = new MapSprite[count];

		int i, j, c = 0, d;
		for (j = 0; j != h; ++ j)
		{
			for (i = 0; i != w; ++ i)
			{
				fin >> d;
				while (d > 0)
				{
					int id;
					fin >> id;

					mMapSprite[c].SetId(id);
					AddSprite(&mMapSprite[c],
							  i * MapTiles::sWidth,
							  j * MapTiles::sHeight,
							  (d - 1) * MapTiles::sLHeight,
							  MapTiles::sWidth, MapTiles::sHeight + MapTiles::sLHeight,
							  MapTiles::sWidth - 1, MapTiles::sHeight - 1);
					
					c ++;
					d --;
				}
			}
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
						node->mSprite->Show(tick, screen, &_rect);
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
		if (mMapSprite)
			delete mMapSprite;
	}
}
