//
//  main.cpp
//  Pacman1
//
//  Created by Koissi Adjorlolo on 2/22/16.
//  Copyright © 2016 centuryapps. All rights reserved.
//

#include <iostream>
#include <SDL2/SDL.h>

typedef int BlockId;

typedef struct
{
    int x, y;
} Vector2i;

static const Vector2i NULL_VECTOR = { -1, -1 };
static bool v2iEqualv2i(Vector2i v1, Vector2i v2);

typedef unsigned char Direction;
static const Direction Direction_None = 0;
static const Direction Direction_Left = 1;
static const Direction Direction_Right = 2;
static const Direction Direction_Up = 3;
static const Direction Direction_Down = 4;

typedef enum
{
    GhostState_None,
    GhostState_Trapped,
    GhostState_Escaping,
    GhostState_Enemy,
    GhostState_Food
} GhostState;

// This was an accident.  It works.
const Vector2i TRAPPED_POSITION = { 13, 14 };

typedef struct
{
    Vector2i position;
    Direction direction;
    bool mouthOpen = false;
} Pacman;

/*
static const Pacman NULL_PACMAN = {
    NULL_VECTOR,
    Direction_None
};
 */

typedef struct
{
    Vector2i position;
    Direction direction;
    Direction wantedDirection;
    GhostState state;
    unsigned int trappedTimer;
    unsigned int foodTimer;
    Vector2i escapePosition;
    Vector2i fleePosition;
} Ghost;

static const Ghost NULL_GHOST = {
    NULL_VECTOR,
    Direction_None,
    GhostState_None
};

static unsigned int MAX_TRAPPED_TIME = 10 * 1000; // MILLISECONDS
static unsigned int MAX_FOOD_TIME = 10 * 1000; // NANOSECONDS (Just kidding that would be rediculous)

static bool ghostEqualghost(Ghost g1, Ghost g2);

static const char *TITLE = "Pacman";
static const int WINDOW_POSX = SDL_WINDOWPOS_UNDEFINED;
static const int WINDOW_POSY = SDL_WINDOWPOS_UNDEFINED;
static const int WINDOW_WIDTH = 896;
static const int WINDOW_HEIGHT = 992;
static const Uint32 WINDOW_FLAGS = 0;
static const Uint32 RENDERER_FLAGS = SDL_RENDERER_ACCELERATED |
                                     SDL_RENDERER_PRESENTVSYNC;
static const int MS_PER_UPDATE = 1000 / 8;

static const BlockId W = 1;
static const BlockId S = 2;
static const BlockId B = 3;
static const BlockId G = 4;
static const BlockId E = 5;
//static const BlockId F = 6;
static const BlockId P = 7;

static const BlockId ID_WALL = W;
static const BlockId ID_SMALLDOT = S;
static const BlockId ID_BIGDOT = B;
static const BlockId ID_GHOST = G;
//static const BlockId ID_ENEMYGHOST = E;
//static const BlockId ID_FOODGHOST = F;
static const BlockId ID_PACMAN = P;

static Ghost createGhost(Vector2i position, Direction direction);
static Pacman createPacman(Vector2i position, Direction direction);
static void initGame();
static void update();
static void updateGhosts(Ghost ghosts[], int nGhosts);
static void updateGhost(Ghost &ghost);
static void updatePacman(Pacman &pacman);

static void render(SDL_Renderer *renderer);
static void renderBoard(SDL_Renderer *renderer);
static void renderGhosts(SDL_Renderer *renderer, Ghost ghosts[], int nGhosts);
static void renderGhost(SDL_Renderer *renderer, Ghost ghost);
static void renderPacman(SDL_Renderer *renderer, Pacman pacman);

static Vector2i findFirstGhostPosition();
static Vector2i findFirstPacmanPosition();
static Vector2i findFirstBlockIdPosition(BlockId blockId);
static BlockId getBlockIdForPosition(Vector2i position);
static void setBlockIdForPosition(BlockId blockId, Vector2i position);
static bool positionOutOfBounds(Vector2i position);
static bool canMoveInDirection(Direction direction, Vector2i position);
static Direction findPossibleDirection(Vector2i position);
static Direction findClosestPossibleDirection(Direction direction, Vector2i position);
static Vector2i positionInDirection(Direction direction, Vector2i position);
static bool blockIsEmpty(Vector2i position);
static BlockId eatFood(Vector2i position);
static bool ghostAtPosition(Vector2i position);
static void eatGhostAtPosition(Vector2i position);
static Ghost getGhostAtPosition(Vector2i position);
static void eatGhost(Ghost &ghost);
static void killPacman(Pacman &pacman);
static Direction getOppositeDirection(Direction direction);
static void ateSmallDot(Vector2i position);
static void ateBigDot(Vector2i position);
static void turnGhostsToFood(Ghost ghosts[], int nGhosts);
static unsigned int countFood();
static void moveGhostToPosition(Ghost &ghost, Vector2i position);
static int getIntSign(int num);
static void attackPosition(Ghost &ghost, Vector2i position);
static Direction findIdealDirection(Vector2i position, Vector2i wantedPosition);
static bool positionIsIntersection(Direction direction, Vector2i position);
static float distance(Vector2i p1, Vector2i p2);
static Direction getRandomDirection();
static int random(int min, int max);
static Vector2i getRandomEmptyPosition();
static void moveGhost(Ghost &ghost);

static BlockId SMALLDOT_SIZE = 8;
static BlockId BIGDOT_SIZE = 16;
static BlockId BLOCK_SIZE = 32;
static const int BOARD_WIDTH = 28;
static const int BOARD_HEIGHT = 32;
static BlockId GAME_BOARD[BOARD_HEIGHT][BOARD_WIDTH] = {
    { W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W },
    { W, S, S, S, S, S, S, S, S, S, S, S, S, W, W, S, S, S, S, S, S, S, S, S, S, S, S, W },
    { W, S, W, W, W, W, S, W, W, W, W, W, S, W, W, S, W, W, W, W, W, S, W, W, W, W, S, W },
    { W, B, W, W, W, W, S, W, W, W, W, W, S, W, W, S, W, W, W, W, W, S, W, W, W, W, B, W },
    { W, S, W, W, W, W, S, W, W, W, W, W, S, W, W, S, W, W, W, W, W, S, W, W, W, W, S, W },
    { W, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, W },
    { W, S, W, W, W, W, S, W, W, S, W, W, W, W, W, W, W, W, S, W, W, S, W, W, W, W, S, W },
    { W, S, W, W, W, W, S, W, W, S, W, W, W, W, W, W, W, W, S, W, W, S, W, W, W, W, S, W },
    { W, S, S, S, S, S, S, W, W, S, S, S, S, W, W, S, S, S, S, W, W, S, S, S, S, S, S, W },
    { W, W, W, W, W, W, S, W, W, W, W, W, 0, W, W, 0, W, W, W, W, W, S, W, W, W, W, W, W },
    { W, W, W, W, W, W, S, W, W, W, W, W, 0, W, W, 0, W, W, W, W, W, S, W, W, W, W, W, W },
    { W, W, W, W, W, W, S, W, W, G, 0, 0, 0, 0, 0, 0, 0, 0, G, W, W, S, W, W, W, W, W, W },
    { W, W, W, W, W, W, S, W, W, 0, W, W, W, W, W, W, W, W, 0, W, W, S, W, W, W, W, W, W },
    { W, W, W, W, W, W, S, W, W, 0, W, 0, 0, 0, 0, 0, 0, W, 0, W, W, S, W, W, W, W, W, W },
    { 0, 0, 0, 0, 0, 0, S, 0, 0, 0, W, 0, 0, 0, 0, 0, 0, W, 0, 0, 0, S, 0, 0, 0, 0, 0, 0 },
    { W, W, W, W, W, W, S, W, W, 0, W, 0, 0, 0, 0, 0, 0, W, 0, W, W, S, W, W, W, W, W, W },
    { W, W, W, W, W, W, S, W, W, 0, W, W, W, W, W, W, W, W, 0, W, W, S, W, W, W, W, W, W },
    { W, W, W, W, W, W, S, W, W, G, 0, 0, 0, 0, 0, 0, 0, 0, G, W, W, S, W, W, W, W, W, W },
    { W, W, W, W, W, W, S, W, W, 0, W, W, W, W, W, W, W, W, 0, W, W, S, W, W, W, W, W, W },
    { W, W, W, W, W, W, S, W, W, 0, W, W, W, W, W, W, W, W, 0, W, W, S, W, W, W, W, W, W },
    { W, S, S, S, S, S, S, S, S, S, S, S, S, W, W, S, S, S, S, S, S, S, S, S, S, S, S, W },
    { W, S, W, W, W, W, S, W, W, W, W, W, S, W, W, S, W, W, W, W, W, S, W, W, W, W, S, W },
    { W, S, W, W, W, W, S, W, W, W, W, W, S, W, W, S, W, W, W, W, W, S, W, W, W, W, S, W },
    { W, B, S, S, W, W, S, S, S, S, S, S, S, 0, P, S, S, S, S, S, S, S, W, W, S, S, B, W },
    { W, W, W, S, W, W, S, W, W, S, W, W, W, W, W, W, W, W, S, W, W, S, W, W, S, W, W, W },
    { W, W, W, S, W, W, S, W, W, S, W, W, W, W, W, W, W, W, S, W, W, S, W, W, S, W, W, W },
    { W, S, S, S, S, S, S, W, W, S, S, S, S, W, W, S, S, S, S, W, W, S, S, S, S, S, S, W },
    { W, S, W, W, W, W, W, W, W, W, W, W, S, W, W, S, W, W, W, W, W, W, W, W, W, W, S, W },
    { W, S, W, W, W, W, W, W, W, W, W, W, S, W, W, S, W, W, W, W, W, W, W, W, W, W, S, W },
    { W, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, S, W },
    { W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W },
};

static const int NGHOSTS = 4;
static Ghost gGhosts[NGHOSTS];
static Pacman gPacman;
static bool gRunning = false;
static SDL_TimerID gFoodTimerId;
static unsigned int gFoodCount = 0;
static int gDeathCount = 0;

int main(int argc, const char * argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout << "Unable to init SDL" << std::endl;
        exit(1);
    }
    
    SDL_Window *window = SDL_CreateWindow(TITLE,
                                          WINDOW_POSX,
                                          WINDOW_POSY,
                                          WINDOW_WIDTH,
                                          WINDOW_HEIGHT,
                                          WINDOW_FLAGS);
    
    if (window == nullptr)
    {
        std::cout << "Unable to create window" << std::endl;
        SDL_Quit();
        exit(1);
    }
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, RENDERER_FLAGS);
    
    if (renderer == nullptr)
    {
        std::cout << "Unable to create renderer" << std::endl;
        SDL_Quit();
        exit(1);
    }
    
    gRunning = true;
    SDL_Event event;
    
    double previous = (double)SDL_GetTicks();
    double lag = 0.0;
    
    initGame();
    
    while (gRunning)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    gRunning = false;
                    break;
                    
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_LEFT:
                                gPacman.direction = Direction_Left;
                            break;
                            
                        case SDLK_RIGHT:
                                gPacman.direction = Direction_Right;
                            break;
                            
                        case SDLK_UP:
                                gPacman.direction = Direction_Up;
                            break;
                            
                        case SDLK_DOWN:
                                gPacman.direction = Direction_Down;
                            break;
                            
                        default:
                            break;
                    }
                    
                default:
                    break;
            }
        }
        
        double current = (double)SDL_GetTicks();
        double elapsed = current - previous;
        lag += elapsed;
        previous = current;
        
        while (lag >= MS_PER_UPDATE)
        {
            update();
            lag -= MS_PER_UPDATE;
        }
        
        render(renderer);
    }
    
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    renderer = nullptr;
    window = nullptr;
    
    SDL_Quit();
    
    return 0;
}

static Ghost createGhost(Vector2i position, Direction direction)
{
    Ghost ghost;
    
    ghost.state = GhostState_Enemy;
    ghost.position = position;
    ghost.direction = direction;
    ghost.trappedTimer = 0;
    ghost.foodTimer = 0;
    ghost.escapePosition = getRandomEmptyPosition();
    ghost.fleePosition = getRandomEmptyPosition();
    
    return ghost;
}

static Pacman createPacman(Vector2i position, Direction direction)
{
    Pacman pacman;
    
    pacman.position = position;
    pacman.direction = direction;
    
    return pacman;
}

static void initGame()
{
    gFoodCount = countFood();
    
    // Ghosts
    for (int ghostIndex = 0;
         ghostIndex < NGHOSTS;
         ghostIndex++)
    {
        Vector2i ghostPosition = findFirstGhostPosition();
        
        if (!v2iEqualv2i(ghostPosition, NULL_VECTOR))
        {
            // The initial direction isn't too important.
            gGhosts[ghostIndex] = createGhost(ghostPosition,
                                              getRandomDirection());
        }
        else
        {
            gGhosts[ghostIndex] = NULL_GHOST;
        }
        
        setBlockIdForPosition(E, ghostPosition);
    }
    
    // Pacman
    Vector2i pacmanPosition = findFirstPacmanPosition();
    
    if (!v2iEqualv2i(pacmanPosition, NULL_VECTOR))
    {
        gPacman = createPacman(pacmanPosition, Direction_None);
    }
    else
    {
        std::cout << "No Pacman found!" << std::endl;
        gRunning = false;
    }
}

static void update()
{
    updatePacman(gPacman);
    updateGhosts(gGhosts, NGHOSTS);
    
    if (gFoodCount == 0)
    {
        gRunning = false;
        std::cout << "You won!!!" << std::endl;
        std::cout << "You died " << gDeathCount << " times." << std::endl;
    }
}

static void updateGhosts(Ghost ghosts[], int nGhosts)
{
    for (int ghostIndex = 0;
         ghostIndex < nGhosts;
         ghostIndex++)
    {
        updateGhost(ghosts[ghostIndex]);
    }
}

static void updateGhost(Ghost &ghost)
{
    switch (ghost.state)
    {
        case GhostState_Enemy:
            attackPosition(ghost, gPacman.position);
            
            moveGhost(ghost);
            break;
            
        case GhostState_Food:
            ghost.foodTimer += MS_PER_UPDATE;
            attackPosition(ghost, ghost.fleePosition);
            moveGhost(ghost);
            
            if (ghost.foodTimer >= MAX_FOOD_TIME)
            {
                ghost.state = GhostState_Enemy;
                ghost.foodTimer = 0;
            }
            
            break;
            
        case GhostState_Trapped:
            ghost.trappedTimer += MS_PER_UPDATE;
            moveGhostToPosition(ghost, TRAPPED_POSITION);
            
            if (ghost.trappedTimer >= MAX_TRAPPED_TIME)
            {
                ghost.state = GhostState_Escaping;
            }
            break;
            
        case GhostState_Escaping:
            moveGhostToPosition(ghost, ghost.escapePosition);
            
            if (v2iEqualv2i(ghost.escapePosition, ghost.position))
            {
                ghost.trappedTimer = 0;
                ghost.state = GhostState_Enemy;
                ghost.escapePosition = getRandomEmptyPosition();
            }
            break;
            
        default:
            break;
    }
}

static void updatePacman(Pacman &pacman)
{
    pacman.mouthOpen = !pacman.mouthOpen;
    
    // Eating food.
    switch (eatFood(pacman.position))
    {
        case ID_SMALLDOT:
            ateSmallDot(pacman.position);
            break;
            
        case ID_BIGDOT:
            ateBigDot(pacman.position);
            break;
            
        default:
            break;
    }
    
    Ghost ghost = getGhostAtPosition(pacman.position);
    
    if (!ghostEqualghost(ghost, NULL_GHOST))
    {
        if (ghost.state == GhostState_Enemy)
        {
            killPacman(pacman);
        }
        else if (ghost.state == GhostState_Food)
        {
            eatGhostAtPosition(pacman.position);
        }
    }
    
    if (canMoveInDirection(pacman.direction, pacman.position))
    {
        pacman.position = positionInDirection(pacman.direction, pacman.position);
        Direction oppositeDir = getOppositeDirection(pacman.direction);
        
        // Check edge case where Pacman and Ghost can pass through each other.
        if (ghostAtPosition(positionInDirection(oppositeDir,
                                                pacman.position)))
        {
            Ghost ghost = getGhostAtPosition(positionInDirection(oppositeDir,
                                                                 pacman.position));
            if (ghost.state == GhostState_Enemy &&
                ghost.direction == oppositeDir)
            {
                killPacman(pacman);
            }
        }
        
        if (pacman.position.x < 0)
        {
            pacman.position.x = BOARD_WIDTH;
            pacman.position.y = 14;
        }
        if (pacman.position.x >= BOARD_HEIGHT - 3)
        {
            pacman.position.x = 0;
            pacman.position.y = 14;
        }
    }
    
    Ghost ghostAtPosition = getGhostAtPosition(pacman.position);
    
    if (!ghostEqualghost(ghostAtPosition, NULL_GHOST))
    {
        if (ghostAtPosition.state == GhostState_Food)
        {
            eatGhostAtPosition(pacman.position);
        }
        else if (ghostAtPosition.state == GhostState_Enemy)
        {
            killPacman(pacman);
        }
    }
}

static void render(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    renderBoard(renderer);
    renderGhosts(renderer, gGhosts, NGHOSTS);
    renderPacman(renderer, gPacman);
    
    SDL_RenderPresent(renderer);
}

static void renderBoard(SDL_Renderer *renderer)
{
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            BlockId blockId = GAME_BOARD[y][x];
            SDL_Rect fillRect = {
                x * BLOCK_SIZE,
                y * BLOCK_SIZE,
                BLOCK_SIZE,
                BLOCK_SIZE
            };
            
            switch (blockId)
            {
                case 0:
                    continue;
                    break;
                    
                case ID_WALL:
                    SDL_SetRenderDrawColor(renderer, 0, 0, 100, 255);
                    break;
                    
                case ID_SMALLDOT:
                    SDL_SetRenderDrawColor(renderer, 255, 237, 166, 255);
                    fillRect = {
                        x * BLOCK_SIZE + BLOCK_SIZE / 2 - SMALLDOT_SIZE / 2,
                        y * BLOCK_SIZE + BLOCK_SIZE / 2 - SMALLDOT_SIZE / 2,
                        SMALLDOT_SIZE,
                        SMALLDOT_SIZE
                    };
                    break;
                    
                case ID_BIGDOT:
                    SDL_SetRenderDrawColor(renderer, 255, 237, 166, 255);
                    fillRect = {
                        x * BLOCK_SIZE + BLOCK_SIZE / 2 - BIGDOT_SIZE / 2,
                        y * BLOCK_SIZE + BLOCK_SIZE / 2 - BIGDOT_SIZE / 2,
                        BIGDOT_SIZE,
                        BIGDOT_SIZE
                    };
                    break;
                    
                /*  These don't matter too much since they're drawn elsewhere
                case ID_ENEMYGHOST:
                    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
                    break;
                    
                case ID_FOODGHOST:
                    SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
                    break;
                    
                case ID_PACMAN:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                    break;
                */
                    
                default:
                    continue;
                    break;
            }
            
            SDL_RenderFillRect(renderer, &fillRect);
        }
    }
}

static void renderGhosts(SDL_Renderer *renderer, Ghost ghosts[], int nGhosts)
{
    for (int ghostIndex = 0;
         ghostIndex < nGhosts;
         ghostIndex++)
    {
        renderGhost(renderer, ghosts[ghostIndex]);
    }
}

static void renderGhost(SDL_Renderer *renderer, Ghost ghost)
{
    SDL_Rect rect = {
        ghost.position.x * BLOCK_SIZE,
        ghost.position.y * BLOCK_SIZE,
        BLOCK_SIZE,
        BLOCK_SIZE
    };
    
    switch (ghost.state)
    {
        case GhostState_Enemy:
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            break;
            
        case GhostState_Food:
            if (ghost.foodTimer <= MAX_FOOD_TIME / 2)
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 200, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
            }
            break;
            
        case GhostState_None:
            return;
            
        default:
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            break;
    }
    
    SDL_RenderFillRect(renderer, &rect);
}

static void renderPacman(SDL_Renderer *renderer, Pacman pacman)
{
    SDL_Rect rect = {
        pacman.position.x * BLOCK_SIZE,
        pacman.position.y * BLOCK_SIZE,
        BLOCK_SIZE,
        BLOCK_SIZE
    };
    
    SDL_Rect directionRect;
    
    switch (pacman.direction)
    {
        case Direction_Left:
            directionRect.w = (BLOCK_SIZE / 3) * 2;
            directionRect.h = BLOCK_SIZE / 3;
            directionRect.x = rect.x;
            directionRect.y = rect.y + rect.h / 2 - directionRect.h / 2;
            break;
            
        case Direction_Right:
            directionRect.w = (BLOCK_SIZE / 3) * 2;
            directionRect.h = BLOCK_SIZE / 3;
            directionRect.x = rect.x + rect.w - directionRect.w;
            directionRect.y = rect.y + rect.h / 2 - directionRect.h / 2;
            break;
            
        case Direction_Up:
            directionRect.w = BLOCK_SIZE / 3;
            directionRect.h = (BLOCK_SIZE / 3) * 2;
            directionRect.x = rect.x + rect.w / 2 - directionRect.w / 2;
            directionRect.y = rect.y;
            break;
            
        case Direction_Down:
            directionRect.w = BLOCK_SIZE / 3;
            directionRect.h = (BLOCK_SIZE / 3) * 2;
            directionRect.x = rect.x + rect.w / 2 - directionRect.w / 2;
            directionRect.y = rect.y + rect.h - directionRect.h;
            break;
            
        default:
            break;
    }
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
    
    if (pacman.mouthOpen)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &directionRect);
    }
}

static Vector2i findFirstGhostPosition()
{
    return findFirstBlockIdPosition(ID_GHOST);
}

static Vector2i findFirstPacmanPosition()
{
    return findFirstBlockIdPosition(ID_PACMAN);
}

static Vector2i findFirstBlockIdPosition(BlockId blockId)
{
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            Vector2i position = { x, y };
            
            if (getBlockIdForPosition(position) == blockId)
            {
                return position;
            }
        }
    }
    
    return { -1, -1 };
}

static BlockId getBlockIdForPosition(Vector2i position)
{
    if (!positionOutOfBounds(position))
    {
        return GAME_BOARD[position.y][position.x];
    }
    
    return -1;
}

static void setBlockIdForPosition(BlockId blockId, Vector2i position)
{
    if (!positionOutOfBounds(position))
    {
        GAME_BOARD[position.y][position.x] = blockId;
    }
}

static bool positionOutOfBounds(Vector2i position)
{
    return !(position.x < BOARD_WIDTH && position.x >= 0 &&
             position.y < BOARD_HEIGHT && position.y >= 0);
}

static bool v2iEqualv2i(Vector2i v1, Vector2i v2)
{
    return (v1.x == v2.x &&
            v1.y == v2.y);
}

static bool ghostEqualghost(Ghost g1, Ghost g2)
{
    return (v2iEqualv2i(g1.position, g2.position) &&
            g1.state == g2.state &&
            g1.direction == g2.direction);
}

static bool canMoveInDirection(Direction direction, Vector2i position)
{
    Vector2i positionToCheck = positionInDirection(direction, position);
    return blockIsEmpty(positionToCheck);
}

static Direction findPossibleDirection(Vector2i position)
{
    if (canMoveInDirection(Direction_Left, position)) return Direction_Left;
    if (canMoveInDirection(Direction_Down, position)) return Direction_Down;
    if (canMoveInDirection(Direction_Right, position)) return Direction_Right;
    if (canMoveInDirection(Direction_Up, position)) return Direction_Up;
    
    return Direction_None;
}

static Direction findClosestPossibleDirection(Direction direction, Vector2i position)
{
    switch (direction)
    {
        case Direction_Left:
            if (canMoveInDirection(Direction_Up, position)) return Direction_Up;
            if (canMoveInDirection(Direction_Down, position)) return Direction_Down;
            if (canMoveInDirection(Direction_Right, position)) return Direction_Right;
            break;
            
        case Direction_Right:
            if (canMoveInDirection(Direction_Down, position)) return Direction_Down;
            if (canMoveInDirection(Direction_Up, position)) return Direction_Up;
            if (canMoveInDirection(Direction_Left, position)) return Direction_Left;
            break;
            
        case Direction_Up:
            if (canMoveInDirection(Direction_Right, position)) return Direction_Right;
            if (canMoveInDirection(Direction_Left, position)) return Direction_Left;
            if (canMoveInDirection(Direction_Down, position)) return Direction_Down;
            break;
            
        case Direction_Down:
            if (canMoveInDirection(Direction_Left, position)) return Direction_Left;
            if (canMoveInDirection(Direction_Right, position)) return Direction_Right;
            if (canMoveInDirection(Direction_Up, position)) return Direction_Up;
            break;
            
        default:
            break;
    }
    
    return Direction_None;
}

static Vector2i positionInDirection(Direction direction, Vector2i position)
{
    Vector2i nextPosition;
    
    switch (direction)
    {
        case Direction_Left:
            nextPosition = {
                position.x - 1,
                position.y
            };
            break;
            
        case Direction_Right:
            nextPosition = {
                position.x + 1,
                position.y
            };
            break;
            
        case Direction_Up:
            nextPosition = {
                position.x,
                position.y - 1
            };
            break;
            
        case Direction_Down:
            nextPosition = {
                position.x,
                position.y + 1
            };
            break;
            
        default:
            break;
    }
    
    return nextPosition;
}

static bool blockIsEmpty(Vector2i position)
{
    BlockId idAtPosition = getBlockIdForPosition(position);
    return (idAtPosition != ID_WALL);
}

static BlockId eatFood(Vector2i position)
{
    BlockId blockIdAtPosition = getBlockIdForPosition(position);
    
    if (!blockIsEmpty(position) &&
        (blockIdAtPosition == ID_SMALLDOT ||
         blockIdAtPosition == ID_BIGDOT))
    {
        gFoodCount--;
        setBlockIdForPosition(0, position);
    }
    
    return blockIdAtPosition;
}

static bool ghostAtPosition(Vector2i position)
{
    for (int ghostIndex = 0;
         ghostIndex < NGHOSTS;
         ghostIndex++)
    {
        Ghost ghost = gGhosts[ghostIndex];
        
        if (v2iEqualv2i(ghost.position, position))
        {
            return true;
        }
    }
    
    return false;
}

static Ghost getGhostAtPosition(Vector2i position)
{
    for (int ghostIndex = 0;
         ghostIndex < NGHOSTS;
         ghostIndex++)
    {
        Ghost ghost = gGhosts[ghostIndex];
        
        if (v2iEqualv2i(ghost.position, position))
        {
            return ghost;
        }
    }
    
    return NULL_GHOST;
}

static void eatGhostAtPosition(Vector2i position)
{
    for (int ghostIndex = 0;
         ghostIndex < NGHOSTS;
         ghostIndex++)
    {
        Ghost &ghost = gGhosts[ghostIndex];
        
        if (v2iEqualv2i(ghost.position, position))
        {
            eatGhost(ghost);
        }
    }
}

static void eatGhost(Ghost &ghost)
{
    ghost.state = GhostState_Trapped;
}

static void killPacman(Pacman &pacman)
{
    // For now...
    gDeathCount++;
    pacman.position = findFirstPacmanPosition();
    
    // Reset ghost spawns
    for (int ghostIndex = 0;
         ghostIndex < NGHOSTS;
         ghostIndex++)
    {
        Vector2i newGhostPosition = findFirstBlockIdPosition(E);
        setBlockIdForPosition(G, newGhostPosition);
    }
    
    initGame();
}

static Direction getOppositeDirection(Direction direction)
{
    if (direction == Direction_Right) return Direction_Left;
    if (direction == Direction_Left) return Direction_Right;
    if (direction == Direction_Down) return Direction_Up;
    if (direction == Direction_Up) return Direction_Down;
    
    return Direction_None;
}

static void ateSmallDot(Vector2i position)
{
    setBlockIdForPosition(0, position);
    gFoodCount--;
}

static void ateBigDot(Vector2i position)
{
    turnGhostsToFood(gGhosts, NGHOSTS);
    setBlockIdForPosition(0, position);
    gFoodCount--;
}

Uint32 foodTimerCallback(Uint32 interval, void *param)
{
    for (int ghostIndex = 0;
         ghostIndex < NGHOSTS;
         ghostIndex++)
    {
        Ghost &ghost = gGhosts[ghostIndex];
        
        if (ghost.state == GhostState_Food)
        {
            ghost.state = GhostState_Enemy;
        }
    }
    
    SDL_RemoveTimer(gFoodTimerId);
    
    return interval;
}

static void turnGhostsToFood(Ghost ghosts[], int nGhosts)
{
    for (int ghostIndex = 0;
         ghostIndex < nGhosts;
         ghostIndex++)
    {
        Ghost &ghost = ghosts[ghostIndex];
        
        if (ghost.state == GhostState_Enemy)
        {
            ghost.state = GhostState_Food;
        }
    }
}

static unsigned int countFood()
{
    unsigned int count = 0;
    
    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            if (GAME_BOARD[y][x] == ID_SMALLDOT ||
                GAME_BOARD[y][x] == ID_BIGDOT)
            {
                count++;
            }
        }
    }
    
    return count;
}

static void moveGhostToPosition(Ghost &ghost, Vector2i position)
{
    int xDir = getIntSign(position.x - ghost.position.x);
    int yDir = getIntSign(position.y - ghost.position.y);
    
    if (ghost.position.x == position.x)
    {
        xDir = 0;
    }
    
    if (ghost.position.y == position.y)
    {
        yDir = 0;
    }
    
    ghost.position.x += xDir;
    ghost.position.y += yDir;
}

static int getIntSign(int num)
{
    return (num < 0) ? -1 : 1;
}

static void attackPosition(Ghost &ghost, Vector2i position)
{
    if (positionIsIntersection(ghost.direction, ghost.position))
    {
        Direction idealDirection = findIdealDirection(ghost.position, position);
        Direction realDirection = idealDirection;
        Direction oppositeDirection = getOppositeDirection(ghost.direction);
        
        if (ghost.wantedDirection != idealDirection)
        {
            ghost.wantedDirection = idealDirection;
        }
        
        if (!canMoveInDirection(idealDirection, ghost.position))
        {
            realDirection = findClosestPossibleDirection(idealDirection, ghost.position);
            
            if (realDirection == Direction_None)
            {
                realDirection = ghost.direction;
            }
        }
        
        if (realDirection == oppositeDirection)
        {
           realDirection = findPossibleDirection(ghost.position);
        }
        
        ghost.direction = realDirection;
    }
}

static Direction findIdealDirection(Vector2i position, Vector2i wantedPosition)
{
    Vector2i leftPosition = positionInDirection(Direction_Left, position);
    Vector2i rightPosition = positionInDirection(Direction_Right, position);
    Vector2i upPosition = positionInDirection(Direction_Up, position);
    Vector2i downPosition = positionInDirection(Direction_Down, position);
    
    float leftDistance = distance(leftPosition, wantedPosition);
    float rightDistance = distance(rightPosition, wantedPosition);
    float upDistance = distance(upPosition, wantedPosition);
    float downDistance = distance(downPosition, wantedPosition);
    
    if ((leftDistance <= rightDistance &&
        leftDistance <= upDistance &&
        leftDistance <= downDistance)) return Direction_Left;
    if (rightDistance <= leftDistance &&
        rightDistance <= upDistance &&
        rightDistance <= downDistance) return Direction_Right;
    if (upDistance <= leftDistance &&
        upDistance <= rightDistance &&
        upDistance <= downDistance) return Direction_Up;
    if (downDistance <= leftDistance &&
        downDistance <= rightDistance &&
        downDistance <= upDistance) return Direction_Down;
    
    return Direction_None;
}

static bool positionIsIntersection(Direction direction, Vector2i position)
{
    switch (direction)
    {
        case Direction_Left:
            return (canMoveInDirection(Direction_Up, position) ||
                    canMoveInDirection(Direction_Down, position));
            break;
        case Direction_Right:
            return (canMoveInDirection(Direction_Up, position) ||
                    canMoveInDirection(Direction_Down, position));
            break;
        case Direction_Up:
            return (canMoveInDirection(Direction_Left, position) ||
                    canMoveInDirection(Direction_Right, position));
            break;
        case Direction_Down:
            return (canMoveInDirection(Direction_Left, position) ||
                    canMoveInDirection(Direction_Right, position));
            break;
        default:
            break;
    }
    
    return false;
}

static float distance(Vector2i p1, Vector2i p2)
{
    return (sqrtf(powf(p1.x - p2.x, 2.0f) +
                  powf(p1.y - p2.y, 2.0f)));
}

static Direction getRandomDirection()
{
    return random(1, 4);
}

static int random(int min, int max)
{
    return rand() % (max - min) + min;
}

static Vector2i getRandomEmptyPosition()
{
    Vector2i position = {
        random(0, BOARD_WIDTH - 1),
        random(0, BOARD_HEIGHT - 1)
    };
    
    while (!blockIsEmpty(position))
    {
        position = {
            random(0, BOARD_WIDTH - 1),
            random(0, BOARD_HEIGHT - 1)
        };
    }
    
    return position;
}

static void moveGhost(Ghost &ghost)
{
    if (v2iEqualv2i(ghost.position, ghost.fleePosition))
    {
        ghost.fleePosition = getRandomEmptyPosition();
    }
    
    if (canMoveInDirection(ghost.direction, ghost.position))
    {
        ghost.position = positionInDirection(ghost.direction, ghost.position);
    }
    else
    {
        ghost.direction = findClosestPossibleDirection(ghost.direction,
                                                       ghost.position);
        
        if (ghost.direction == Direction_None)
        {
            std::cout << "Ghost unable to find direction to move in." << std::endl;
            std::cout << "This should never have happened.  Quitting" << std::endl;
            gRunning = false;
        }
        else
        {
            ghost.position = positionInDirection(ghost.direction, ghost.position);
        }
    }
    
    if (ghost.position.x < 0)
    {
        ghost.position.x = BOARD_WIDTH;
        ghost.position.y = 14;
    }
    if (ghost.position.x >= BOARD_HEIGHT - 3)
    {
        ghost.position.x = 0;
        ghost.position.y = 14;
    }
}
