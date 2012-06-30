#ifndef __MAP_HPP__
#define __MAP_HPP__

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <deque>
#include <map>
#include <vector>

#include "sprite.hpp"
#include "logic/object.hpp"
#include "motion.hpp"

namespace Game
{
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

        Sprite *mSprite;
        bool mIsMapTile;
        
        int mState;
        int mX, mY, mZ;
        int mW, mH, mDX, mDY;

        TileNode(void) : mMotion(NULL), mIsMapTile(false) { }
        ~TileNode(void) { if (mMotion) delete mMotion; }
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
        SimpleSprite            *mMapSprite;
        TileMap                  mTileMap;
        
        int mTileWidth;
        int mTileHeight;
        int mTileLHeight;

    public:
        
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
