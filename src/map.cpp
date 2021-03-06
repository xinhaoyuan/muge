#include <fstream>
#include <string>

#include "map.hpp"
#include "resource.hpp"

namespace Game
{
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

		node->mXIdx = DivDown(x + dx - 1, mTileWidth);
		node->mYIdx = DivDown(y + dy - z - 1, mTileHeight);
		node->mZIdx = DivDown(z, mTileLHeight);
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
		int xIdx = DivDown(node->mX + node->mDX - 1, mTileWidth);
		int yIdx = DivDown(node->mY + node->mDY - node->mZ - 1, mTileHeight);
		int zIdx = DivDown(node->mZ, mTileLHeight);

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
		fin >> result->mTileLHeight;
		std::string tilesname;
		fin >> tilesname;

		result->mMapSprite = Resource::Get<SimpleSprite>(tilesname.c_str());
		result->mTileWidth = result->mMapSprite->mWidth;
		result->mTileHeight = result->mMapSprite->mHeight - result->mTileLHeight;

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

					if (id < 0) continue;

					TileNode *node =
						result->AddConstantSprite(result->mMapSprite,
												  i * result->mTileWidth,
												  j * result->mTileHeight,
												  (cur_h - 1) * result->mTileLHeight,
												  result->mTileWidth,
												  result->mTileHeight + result->mTileLHeight,
												  result->mTileWidth - 1,
												  result->mTileHeight + result->mTileLHeight - 1);
					node->mIsMapTile = true;
					node->mState = id;					
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
	}
}
