#include "raylib.h"
#include "raymath.h"

#define GRAVITY 1100.0f
#define ACCELERATION 900.0f
#define FRICTION 1400.0f

#define JUMP_SPEED 550.0f
#define FALL_SPEED 1000.0f
#define SLIDE_SPEED 50.0f

#define MAX_SPEED 300.0f
#define MAX_FALL_SPEED 900.0f

#define MAP_LENGTH 18
#define MAP_HEIGHT 13

#define PLAYER_SIZE 48.0f

typedef struct Player
{
    Vector2 position;
    float vSpeed;
    float hSpeed;
    int frameCounter;
    int currentFrame;
    int onGround;
    int jump;
} Player;


void UpdatePlayer(Player *player, Rectangle *source, float delta, char map[MAP_HEIGHT][MAP_LENGTH]);
void DrawMap(char map[MAP_HEIGHT][MAP_LENGTH], Texture2D tilemap);
char GetTile(char map[MAP_HEIGHT][MAP_LENGTH], float x, float y, float cellSize);

int main(void)
{

    const int screenWidth = 864;
    const int screenHeight = 624;

    InitWindow(screenWidth, screenHeight, "Smack Bruhdas - Demo");

    Texture2D background = LoadTexture("resources/autumn_background.png");
    Texture2D tileMap = LoadTexture("resources/tiles.png");

    char map[MAP_HEIGHT][MAP_LENGTH] = {
        "..................",
        ".........#*.......",
        "..................",
        ".........#......[]",
        "*..#####..oo......",
        "*..*###....**.....",
        "............*o....",
        ".............oo...",
        "..................",
        "................[]",
        ".......#.#........",
        ".......*o**.......",
        "##################"
    };

    Player player = {0};
    player.position = (Vector2){ 400.0f, 400.0f };

    Texture2D robot = LoadTexture("resources/robot_walking.png");
    Rectangle robotSourceRec = { 0.0f, 0.0f, 16.0f, 16.0f };
    Vector2 robotSpriteOrigin = { 0.0f, 0.0f };
    float robotRotation = 0.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {

        float delta_time = GetFrameTime();

        UpdatePlayer(&player, &robotSourceRec, delta_time, map);

        BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawTextureEx(background, (Vector2){ 0.0f, 0.0f }, 0.0f, 3.0f, WHITE);
            DrawMap(map, tileMap);
            DrawTexturePro(robot, robotSourceRec, (Rectangle){ player.position.x, player.position.y, 48.0f, 48.0f}, robotSpriteOrigin, robotRotation, WHITE);
        EndDrawing();
    }

    UnloadTexture(tileMap);
    UnloadTexture(robot);
    UnloadTexture(background);
    
    CloseWindow();

    return 0;
}

void UpdatePlayer(Player *player, Rectangle *source, float delta, char map[MAP_HEIGHT][MAP_LENGTH])
{
    float xMotion = 0;
    float frictionAmount = FRICTION*delta;
    float accelerationAmount = ACCELERATION*delta;
    
    if (IsKeyDown(KEY_RIGHT) && !IsKeyDown(KEY_LEFT)) xMotion = 1.0f;
    if (IsKeyDown(KEY_LEFT) && !IsKeyDown(KEY_RIGHT)) xMotion = -1.0f;
    if (IsKeyDown(KEY_DOWN)) player->vSpeed += FALL_SPEED*delta;
    if (IsKeyPressed(KEY_SPACE) && player->jump == 1 && player->onGround == 1)
    {
        player->vSpeed = -JUMP_SPEED;
        player->jump = 0;
    }

    if (xMotion != 0)
    {
        source->width = -xMotion*16.0f;

        if (player->onGround == 1)
        {
            //--Frame information-----
            player->frameCounter++;
            if (player->frameCounter > 6)
            {
                player->frameCounter = 0;
                player->currentFrame++;
                if (player->currentFrame > 1) player->currentFrame = 0;
                source->x = (float)player->currentFrame*source->width;
            }

            player->hSpeed += xMotion*accelerationAmount;
            if (player->hSpeed > 0 && xMotion < 0) player->hSpeed -= frictionAmount;
            else if (player->hSpeed < 0 && xMotion > 0) player->hSpeed += frictionAmount;
        }
        else
        {
            player->hSpeed += xMotion*accelerationAmount*0.8f;
            source->x = 16.0f;
        }
    }
    else
    {
        if (player->onGround == 0) source->x = 16.0f;
        else source->x = 0.0f;

        if (player->hSpeed > 0 && player->hSpeed - frictionAmount > 0) player->hSpeed -= frictionAmount;
        else if (player->hSpeed < 0 && player->hSpeed + frictionAmount < 0) player->hSpeed += frictionAmount;
        else player->hSpeed = 0;
    }

    if ((fabsf(player->hSpeed) >= MAX_SPEED)) player->hSpeed = xMotion*MAX_SPEED;
    if (player->vSpeed >= MAX_FALL_SPEED) player->vSpeed = MAX_FALL_SPEED;

    //--Colisions---
    float newPlayerPositionX = player->position.x + player->hSpeed*delta;
    float newPlayerPositionY = player->position.y + player->vSpeed*delta;

    int gridAlignedPosX = (int)(newPlayerPositionX/48.0f)*48;
    int gridAlignedPosY = (int)(newPlayerPositionY/48.0f)*48;

    //--X axis---
    if (player->hSpeed < 0)
    {
        if (newPlayerPositionX + 6.0f <= 0)
        {
            newPlayerPositionX = gridAlignedPosX - 6.0f;
            if (IsKeyDown(KEY_LEFT) && player->onGround == 0) player->hSpeed = -SLIDE_SPEED;
            else player->hSpeed = 0;
        }
        if (GetTile(map, newPlayerPositionX + 6.0f, player->position.y + 6.0f, 48.0f) != '.' || GetTile(map, newPlayerPositionX+6.0f, player->position.y + 40.0f, 48.0f) != '.')
        {
            newPlayerPositionX = gridAlignedPosX + 42;
            if (IsKeyDown(KEY_LEFT) && player->onGround == 0) player->hSpeed = -SLIDE_SPEED;
            else player->hSpeed = 0;
        }
    }
    else if (player->hSpeed > 0)
    {
        if (GetTile(map, newPlayerPositionX + 42.0f, player->position.y + 6.0f, 48.0f) != '.' || GetTile(map, newPlayerPositionX + 42.0f, player->position.y + 40.0f, 48.0f) != '.')
        {
            newPlayerPositionX = gridAlignedPosX + 6.0f;
            if (IsKeyDown(KEY_RIGHT) && player->onGround == 0) player->hSpeed = SLIDE_SPEED;
            else player->hSpeed = 0;
        }
    }

    //--Y axis--
    player->onGround = 0;
    if (player->vSpeed < 0)
    {
        if (newPlayerPositionY + 6.0f <= 0)
        {
            newPlayerPositionY = gridAlignedPosY - 6.0f;
            player->vSpeed = 0;
        }
        if (GetTile(map, newPlayerPositionX + 6.0f, newPlayerPositionY + 6.0f, 48.0f) != '.' || GetTile(map, newPlayerPositionX + 40.0f, newPlayerPositionY + 6.0f, 48.0f) != '.')
        {
            newPlayerPositionY = gridAlignedPosY + 42;
            player->vSpeed = 0;
        }
    }
    else if (player->vSpeed > 0)
    {
        if (GetTile(map, newPlayerPositionX + 6.0f, newPlayerPositionY+48.0f, 48.0f) != '.' || GetTile(map, newPlayerPositionX + 40.0f, newPlayerPositionY + 48.0f, 48.0f) != '.')
        {
            newPlayerPositionY = gridAlignedPosY;
            player->onGround = 1;
            player->jump = 1;
            player->vSpeed = 0;
        }
    }

    
    player->vSpeed += GRAVITY*delta;
    
    player->position.x = newPlayerPositionX;
    player->position.y = newPlayerPositionY;
}

void DrawMap(char map[MAP_HEIGHT][MAP_LENGTH], Texture2D tilemap)
{
    Rectangle mapSourceRec = {0};
    mapSourceRec.width = 16.0f;
    mapSourceRec.height = 16.0f;

    Rectangle mapDestinRec = {0};
    mapDestinRec.width = 48.0f;
    mapDestinRec.height = 48.0f;


    for (int r = 0; r < MAP_HEIGHT; r++)
    {
        for (int t = 0; t < MAP_LENGTH; t++)
        {
            mapDestinRec.x = t*48;
            mapDestinRec.y = r*48;

            switch (map[r][t])
            {
                case '.':
                    break;

                case '#':
                    mapSourceRec.x = 16.0f;
                    mapSourceRec.y = 16.0f;
                    DrawTexturePro(tilemap, mapSourceRec, mapDestinRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
                    break;

                case 'o':
                    mapSourceRec.x = 48.0f;
                    mapSourceRec.y = 16.0f;
                    DrawTexturePro(tilemap, mapSourceRec, mapDestinRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
                    break;

                case '%':
                    mapSourceRec.x = 144.0f;
                    mapSourceRec.y = 16.0f;
                    DrawTexturePro(tilemap, mapSourceRec, mapDestinRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
                    break;
                
                case '*':
                    mapSourceRec.x = 64.0f;
                    mapSourceRec.y = 16.0f;
                    DrawTexturePro(tilemap, mapSourceRec, mapDestinRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
                    break;
                
                case '[':
                    mapSourceRec.x = 96.0f;
                    mapSourceRec.y = 64.0f;
                    DrawTexturePro(tilemap, mapSourceRec, mapDestinRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
                    break;
                
                case ']':
                    mapSourceRec.x = 112.0f;
                    mapSourceRec.y = 64.0f;
                    DrawTexturePro(tilemap, mapSourceRec, mapDestinRec, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
                    break;

                default:
                    break;
            }
        }
    }
}

char GetTile(char map[MAP_HEIGHT][MAP_LENGTH], float x, float y, float cellSize)
{
    int t = x/cellSize;
    int r = y/cellSize;
    if (t >= 0 && t < MAP_LENGTH && r >= 0 && r < MAP_HEIGHT) return map[r][t];
    else return '#';
}