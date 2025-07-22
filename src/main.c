#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GRAVITY 980.0f
#define JUMP_FORCE -450.0f
#define PLAYER_SPEED 200.0f
#define MAX_PLATFORMS 22
#define PLATFORM_MIN_GAP 50
#define PLATFORM_MAX_GAP 100
#define MAX_HORIZONTAL_GAP 160
#define PLAYER_HITBOX_WIDTH 32
#define PLAYER_HITBOX_HEIGHT 32
#define MAX_FALL_SPEED 800.0f

#define PLATFORM_HEIGHT 32.0f

typedef enum
{
    MENU,
    PLAYING,
    GAME_OVER
} GameState;

typedef enum
{
    IDLE,
    WALKING,
    JUMPING
} PlayerState;

typedef enum
{
    PLATFORM_TYPE_1,
    PLATFORM_TYPE_2,
    PLATFORM_TYPE_3
} PlatformType;

typedef struct
{
    Texture2D texture;
    int frames;
    int currentFrame;
    float frameTime;
    float elapsedTime;
    float frameWidth;
    float frameHeight;
} Animation;

typedef struct
{
    Vector2 position;
    Vector2 velocity;
    Rectangle hitbox;
    Rectangle previousHitbox;
    PlayerState state;
    PlayerState prevState;
    bool facingRight;
    bool onGround;
    int currentPlatform;
    int platformsHit;

    Animation idleAnim;
    Animation walkAnim;
    Animation jumpAnim;
} Player;

typedef struct
{
    Rectangle rect;
    PlatformType type;
    bool active;
} Platform;

GameState gameState = MENU;
Player player;
Platform platforms[MAX_PLATFORMS];
Camera2D camera = {0};
int score = 0;
int highScore = 0;
float gameSpeed = 1.0f;
float startYPosition = 0;

Texture2D menuBackgroundTexture;
Texture2D startButtonTexture;
Texture2D gameBackgroundTexture;
Texture2D platform1Texture;
Texture2D platform2Texture;
Texture2D platform3Texture;
Texture2D platformTextures[3];
Texture2D gameOverTexture;


bool LoadGameAssets();
void UnloadGameAssets();
void InitPlayer();
void InitPlatforms();
void GeneratePlatform(float refX, float refY);
void UpdatePlayer();
void UpdateGameCamera();
void UpdatePlatforms();
void CheckGameOver();
void DrawParallaxBackground(Texture2D texture, float parallaxFactor);
void DrawPlayer();
void DrawPlatforms();
void DrawMenu();
void DrawGameOver();
void DrawHUD();
void UpdateAnimation(Animation *anim, float deltaTime, bool reset);


bool LoadGameAssets()
{
    menuBackgroundTexture = LoadTexture("assets/titleScreen/background.png");
    startButtonTexture = LoadTexture("assets/titleScreen/start_button.png");
    gameBackgroundTexture = LoadTexture("assets/gameplay/background.png");
    platform1Texture = LoadTexture("assets/gameplay/plataforma1.png");
    platform2Texture = LoadTexture("assets/gameplay/plataforma2.png");
    platform3Texture = LoadTexture("assets/gameplay/plataforma3.png");
    player.idleAnim.texture = LoadTexture("assets/player/player_Idle.png");
    player.walkAnim.texture = LoadTexture("assets/player/player_walk.png");
    player.jumpAnim.texture = LoadTexture("assets/player/player_Jump.png");
    gameOverTexture = LoadTexture("assets/gameplay/dead.png");

    platformTextures[PLATFORM_TYPE_1] = platform1Texture;
    platformTextures[PLATFORM_TYPE_2] = platform2Texture;
    platformTextures[PLATFORM_TYPE_3] = platform3Texture;

    bool allLoaded = true;
    if (menuBackgroundTexture.id == 0)
    {
        TraceLog(LOG_WARNING, "AVISO: assets/titleScreen/background.png nao carregado");
        allLoaded = false;
    }
    if (startButtonTexture.id == 0)
    {
        TraceLog(LOG_WARNING, "AVISO: assets/titleScreen/start_button.png nao carregado");
        allLoaded = false;
    }
    if (gameBackgroundTexture.id == 0)
    {
        TraceLog(LOG_WARNING, "AVISO: assets/gameplay/background.png nao carregado");
        allLoaded = false;
    }
    if (platform1Texture.id == 0)
    {
        TraceLog(LOG_WARNING, "AVISO: assets/gameplay/plataforma1.png nao carregado");
        allLoaded = false;
    }
    if (platform2Texture.id == 0)
    {
        TraceLog(LOG_WARNING, "AVISO: assets/gameplay/plataforma2.png nao carregado");
        allLoaded = false;
    }
    if (platform3Texture.id == 0)
    {
        TraceLog(LOG_WARNING, "AVISO: assets/gameplay/plataforma3.png nao carregado");
        allLoaded = false;
    }
    if (player.idleAnim.texture.id == 0)
    {
        TraceLog(LOG_WARNING, "AVISO: assets/player/player_Idle.png nao carregado");
        allLoaded = false;
    }
    if (player.walkAnim.texture.id == 0)
    {
        TraceLog(LOG_WARNING, "AVISO: assets/player/player_walk.png nao carregado");
        allLoaded = false;
    }
    if (player.jumpAnim.texture.id == 0)
    {
        TraceLog(LOG_WARNING, "AVISO: assets/player/player_Jump.png nao carregado");
        allLoaded = false;
    }
    if (gameOverTexture.id == 0)
    {
        TraceLog(LOG_WARNING, "AVISO: assets/gameplay/dead.png nao carregado");
        allLoaded = false;
    }

    return allLoaded;
}

void UnloadGameAssets()
{
    UnloadTexture(menuBackgroundTexture);
    UnloadTexture(startButtonTexture);
    UnloadTexture(gameBackgroundTexture);
    UnloadTexture(platform1Texture);
    UnloadTexture(platform2Texture);
    UnloadTexture(platform3Texture);
    UnloadTexture(player.idleAnim.texture);
    UnloadTexture(player.walkAnim.texture);
    UnloadTexture(player.jumpAnim.texture);
    UnloadTexture(gameOverTexture);
}

void InitPlayer()
{
    
    Platform initialPlatform = platforms[0];

    player.position = (Vector2){
        initialPlatform.rect.x + initialPlatform.rect.width / 2.0f,
        initialPlatform.rect.y - 2.0f
    };

    player.velocity = (Vector2){0, 0};
    player.hitbox = (Rectangle){
        player.position.x - PLAYER_HITBOX_WIDTH / 2.0f, 
        player.position.y - PLAYER_HITBOX_HEIGHT,
        PLAYER_HITBOX_WIDTH,
        PLAYER_HITBOX_HEIGHT};

    player.previousHitbox = player.hitbox;
    player.state = IDLE;
    player.prevState = IDLE;
    player.facingRight = true;
    player.onGround = true; 
    player.currentPlatform = 0;

    player.idleAnim.frames = 5;
    player.idleAnim.frameTime = 0.15f;
    player.idleAnim.currentFrame = 0;
    player.idleAnim.elapsedTime = 0.0f;
    player.idleAnim.frameWidth = player.idleAnim.texture.width / player.idleAnim.frames;
    player.idleAnim.frameHeight = player.idleAnim.texture.height;

    player.walkAnim.frames = 8;
    player.walkAnim.frameTime = 0.1f;
    player.walkAnim.currentFrame = 0;
    player.walkAnim.elapsedTime = 0.0f;
    player.walkAnim.frameWidth = player.walkAnim.texture.width / player.walkAnim.frames;
    player.walkAnim.frameHeight = player.walkAnim.texture.height;

    player.jumpAnim.frames = 8;
    player.jumpAnim.frameTime = 0.1f;
    player.jumpAnim.currentFrame = 0;
    player.jumpAnim.elapsedTime = 0.0f;
    player.jumpAnim.frameWidth = player.jumpAnim.texture.width / player.jumpAnim.frames;
    player.jumpAnim.frameHeight = player.jumpAnim.texture.height;

    player.platformsHit = 0;
    score = 0;
    startYPosition = player.position.y - PLAYER_HITBOX_HEIGHT; 
}

void InitPlatforms()
{
    for (int i = 0; i < MAX_PLATFORMS; i++)
    {
        platforms[i].active = false;
    }

    platforms[0] = (Platform){
        .rect = {
            SCREEN_WIDTH / 2.0f - 100,
            SCREEN_HEIGHT - 100, 
            200,
            PLATFORM_HEIGHT
        },
        .type = PLATFORM_TYPE_1,
        .active = true};

    
    float lastY = SCREEN_HEIGHT - 100; 
    float lastX = SCREEN_WIDTH / 2.0f; 
    for (int i = 1; i < 8; i++)
    {
        int gap = GetRandomValue(PLATFORM_MIN_GAP, PLATFORM_MAX_GAP);
        lastY -= gap;

        int offsetX = GetRandomValue(-MAX_HORIZONTAL_GAP, MAX_HORIZONTAL_GAP);
        lastX += offsetX;

        if (lastX < 50)
            lastX = 50;
        else if (lastX > SCREEN_WIDTH - 50)
            lastX = SCREEN_WIDTH - 50;

        float width = GetRandomValue(80, 180);

        platforms[i] = (Platform){
            .rect = {lastX - width / 2.0f, lastY, width, PLATFORM_HEIGHT},
            .type = (PlatformType)GetRandomValue(0, 2),
            .active = true};
    }
}

void GeneratePlatform(float refX, float refY)
{
    for (int i = 0; i < MAX_PLATFORMS; i++)
    {
        if (!platforms[i].active)
        {
            int gap = GetRandomValue(PLATFORM_MIN_GAP, PLATFORM_MAX_GAP);
            float newY = refY - gap;

            int offsetX = GetRandomValue(-MAX_HORIZONTAL_GAP, MAX_HORIZONTAL_GAP);
            float newX = refX + offsetX;

            if (newX < 50)
                newX = 50;
            else if (newX > SCREEN_WIDTH - 50)
                newX = SCREEN_WIDTH - 50;

            float width = GetRandomValue(80, 180);

            platforms[i] = (Platform){
                .rect = {newX - width / 2.0f, newY, width, PLATFORM_HEIGHT},
                .type = (PlatformType)GetRandomValue(0, 2),
                .active = true};
            return;
        }
    }
}

void UpdateAnimation(Animation *anim, float deltaTime, bool reset)
{
    if (reset)
    {
        anim->currentFrame = 0;
        anim->elapsedTime = 0.0f;
        return;
    }
    anim->elapsedTime += deltaTime;
    if (anim->elapsedTime >= anim->frameTime)
    {
        anim->currentFrame = (anim->currentFrame + 1) % anim->frames;
        anim->elapsedTime = 0.0f;
    }
}


void UpdatePlayer()
{
    player.previousHitbox = player.hitbox;
    player.prevState = player.state;

    bool moving = false;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        player.velocity.x = -PLAYER_SPEED * gameSpeed;
        player.facingRight = false;
        moving = true;
    }
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        player.velocity.x = PLAYER_SPEED * gameSpeed;
        player.facingRight = true;
        moving = true;
    }
    else
    {
        player.velocity.x = 0;
    }


    if (IsKeyPressed(KEY_SPACE) && player.onGround)
    {
        player.velocity.y = JUMP_FORCE;
        player.onGround = false;
        player.state = JUMPING;

        Platform *current = &platforms[player.currentPlatform];
        float refX = current->rect.x + current->rect.width / 2.0f;
        float refY = current->rect.y;
        GeneratePlatform(refX, refY);
    }


    player.velocity.y += GRAVITY * GetFrameTime() * gameSpeed;


    if (player.velocity.y > MAX_FALL_SPEED)
    {
        player.velocity.y = MAX_FALL_SPEED;
    }


    player.position.x += player.velocity.x * GetFrameTime();
    player.position.y += player.velocity.y * GetFrameTime();

    player.hitbox.x = player.position.x - PLAYER_HITBOX_WIDTH / 2.0f;
    player.hitbox.y = player.position.y - PLAYER_HITBOX_HEIGHT;

   
    player.onGround = false;

    if (player.velocity.y >= 0)
    {
        for (int i = 0; i < MAX_PLATFORMS; i++)
        {
            if (!platforms[i].active)
                continue;


            Rectangle playerFeetArea = {
                player.hitbox.x,
                player.hitbox.y + player.hitbox.height - 10, 
                player.hitbox.width,
                15 
            };

            if (CheckCollisionRecs(playerFeetArea, platforms[i].rect))
            {

                if (player.previousHitbox.y + player.previousHitbox.height <= platforms[i].rect.y + 1.0f) 
                {
                    player.position.y = platforms[i].rect.y;
                    player.velocity.y = 0;
                    player.onGround = true;
                    player.currentPlatform = i;
                    break; 
                }
            }
        }
    }

    if (!player.onGround)
    {
        player.state = JUMPING;
    }
    else if (moving)
    {
        player.state = WALKING;
    }
    else
    {
        player.state = IDLE;
    }

 
    bool resetAnimation = (player.prevState != player.state);
    switch (player.state)
    {
    case IDLE:
        UpdateAnimation(&player.idleAnim, GetFrameTime() * gameSpeed, resetAnimation);
        break;
    case WALKING:
        UpdateAnimation(&player.walkAnim, GetFrameTime() * gameSpeed, resetAnimation);
        break;
    case JUMPING:
        UpdateAnimation(&player.jumpAnim, GetFrameTime() * gameSpeed, resetAnimation);
        break;
    }

    float heightDifference = startYPosition - (player.hitbox.y);
    if (heightDifference > score)
    {
        score = (int)heightDifference;
    }

    gameSpeed = 1.0f + (score * 0.0005f);
    if (gameSpeed > 2.5f)
        gameSpeed = 2.5f;
}

void UpdateGameCamera()
{
    
    camera.target.x = player.position.x;
    camera.target.y = player.position.y - SCREEN_HEIGHT / 3.0f; 

    if (camera.target.y > startYPosition - SCREEN_HEIGHT / 2.0f + 50)
    {
        camera.target.y = startYPosition - SCREEN_HEIGHT / 2.0f + 50;
    }

    camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
}


void UpdatePlatforms()
{
    float bottomLimit = camera.target.y + SCREEN_HEIGHT / 2.0f + 100;
    for (int i = 0; i < MAX_PLATFORMS; i++)
    {
        if (platforms[i].active && platforms[i].rect.y > bottomLimit)
        {
            platforms[i].active = false;
        }
    }

    float highestActivePlatformY = -INFINITY;
    for (int i = 0; i < MAX_PLATFORMS; i++)
    {
        if (platforms[i].active)
        {
            if (platforms[i].rect.y < highestActivePlatformY || highestActivePlatformY == -INFINITY)
            {
                highestActivePlatformY = platforms[i].rect.y;
            }
        }
    }

    if (highestActivePlatformY == -INFINITY || highestActivePlatformY > camera.target.y - SCREEN_HEIGHT / 2.0f + 150)
    {
        float refX = SCREEN_WIDTH / 2.0f;
        if (highestActivePlatformY != -INFINITY)
        {
            for (int i = 0; i < MAX_PLATFORMS; i++)
            {
                if (platforms[i].active && platforms[i].rect.y == highestActivePlatformY)
                {
                    refX = platforms[i].rect.x + platforms[i].rect.width / 2.0f;
                    break;
                }
            }
        }
        else
        {
            highestActivePlatformY = camera.target.y - SCREEN_HEIGHT / 2.0f;
        }

        for (int j = 0; j < 5; j++)
        {
            GeneratePlatform(refX, highestActivePlatformY - (j * (PLATFORM_MAX_GAP + PLATFORM_HEIGHT)));
        }
    }
}

void CheckGameOver()
{
    if (player.position.y > camera.target.y + (SCREEN_HEIGHT / 2.0f) + 50)
    {
        gameState = GAME_OVER;
        if (score > highScore)
            highScore = score;
    }
}

void DrawParallaxBackground(Texture2D texture, float parallaxFactor)
{
    if (texture.id == 0)
        return;

    float cameraTopY = camera.target.y - SCREEN_HEIGHT / 2.0f;
    float bgY_unwrapped = cameraTopY * parallaxFactor;
    float offsetY = fmod(-bgY_unwrapped, texture.height);
    if (offsetY > 0) offsetY -= texture.height;

    for (int y = (int)(offsetY - texture.height); y < SCREEN_HEIGHT; y += texture.height)
    {
        
        float bgX_unwrapped = camera.target.x * parallaxFactor;
        float offsetX = fmod(-bgX_unwrapped, texture.width);
        if (offsetX > 0) offsetX -= texture.width;

        for (int x = (int)(offsetX - texture.width); x < SCREEN_WIDTH; x += texture.width)
        {
            DrawTexture(texture, x, y, WHITE);
    
        }
    }
}


void DrawPlayer()
{
    Animation *currentAnim = NULL;

    switch (player.state)
    {
    case IDLE:
        currentAnim = &player.idleAnim;
        break;
    case WALKING:
        currentAnim = &player.walkAnim;
        break;
    case JUMPING:
        currentAnim = &player.jumpAnim;
        break;
    }

    if (currentAnim->texture.id == 0)
    {
        DrawRectangleRec(player.hitbox,
                         player.state == JUMPING ? RED : player.state == WALKING ? BLUE
                                                                                : GREEN);
        return;
    }

    Rectangle src;
    src.y = 0.0f;
    src.height = (float)currentAnim->frameHeight;

    if (player.facingRight)
    {
        src.x = (float)currentAnim->currentFrame * currentAnim->frameWidth;
        src.width = (float)currentAnim->frameWidth;
    }
    else
    {
        src.x = (float)(currentAnim->currentFrame + 1) * currentAnim->frameWidth;
        src.width = -(float)currentAnim->frameWidth;
    }

    Rectangle dest = {
        player.position.x,
        player.position.y,
        (float)currentAnim->frameWidth,
        (float)currentAnim->frameHeight};

    Vector2 origin = {
        (float)currentAnim->frameWidth / 2.0f,
        (float)currentAnim->frameHeight};

    DrawTexturePro(currentAnim->texture, src, dest, origin, 0.0f, WHITE);

}

void DrawPlatforms()
{
    for (int i = 0; i < MAX_PLATFORMS; i++)
    {
        if (!platforms[i].active)
            continue;

        if (platformTextures[platforms[i].type].id != 0)
        {
            DrawTexturePro(
                platformTextures[platforms[i].type],
                (Rectangle){0, 0, (float)platformTextures[platforms[i].type].width, (float)platformTextures[platforms[i].type].height},
                platforms[i].rect,
                (Vector2){0, 0},
                0.0f,
                WHITE);
        }
        else
        {
            Color colors[] = {BROWN, DARKBROWN, BEIGE};
            DrawRectangleRec(platforms[i].rect, colors[platforms[i].type]);
        }
    }
}

void DrawMenu()
{
    if (menuBackgroundTexture.id != 0)
    {
        DrawTexture(menuBackgroundTexture, 0, 0, WHITE);
    }
    else
    {
        ClearBackground(DARKBLUE);
        DrawText("ENDLESS JUMPING", SCREEN_WIDTH / 2 - 180, 150, 40, WHITE);
    }

    Rectangle btnRect = {SCREEN_WIDTH / 2.0f - 128, SCREEN_HEIGHT / 2.0f, 256, 128};

    if (startButtonTexture.id != 0)
    {
        DrawTexture(startButtonTexture, btnRect.x, btnRect.y, WHITE);
    }
    else
    {
        DrawRectangleRec(btnRect, GREEN);
        DrawText("PLAY", SCREEN_WIDTH / 2 - 40, SCREEN_HEIGHT / 2 + 40, 40, WHITE);
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ENTER))
    {
        Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, btnRect))
        {
            gameState = PLAYING;
            InitPlatforms();
            InitPlayer();
            score = 0;
            gameSpeed = 1.0f;
        }
    }

    if (highScore > 0)
    {
        DrawText(TextFormat("High Score: %d", highScore),
                 SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 100, 20, BLACK);
    }

    DrawText("Use A/D para mover e ESPACO para pular",
             SCREEN_WIDTH / 2 - 180, SCREEN_HEIGHT - 50, 20, WHITE);
}

void DrawGameOver()
{
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));

    if (gameOverTexture.id != 0)
    {
        float scale = 0.7f;
        Vector2 position = {
            SCREEN_WIDTH / 2.0f - (gameOverTexture.width * scale) / 2.0f,
            SCREEN_HEIGHT / 2.0f - (gameOverTexture.height * scale) / 2.0f - 50};
        DrawTextureEx(gameOverTexture, position, 0.0f, scale, WHITE);
    }
    else
    {
        DrawText("CAIU E PERDEU!", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 80, 40, RED);
    }

    DrawText(TextFormat("Plataformas: %d", score), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 20, 30, WHITE);

    if (highScore > 0)
    {
        DrawText(TextFormat("Recorde: %d", highScore), SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 + 20, 30, GOLD);
    }

    DrawText("Pressione ENTER para voltar ao inicio", SCREEN_WIDTH / 2 - 220, SCREEN_HEIGHT - 60, 20, LIGHTGRAY);
}

void DrawHUD()
{
    DrawText(TextFormat("Plataformas: %d", score), 10, 10, 20, WHITE);
    DrawText(TextFormat("Velocidade: %.1fx", gameSpeed), 10, 35, 16, GREEN);
    DrawText("ESC: Menu", SCREEN_WIDTH - 100, 10, 20, LIGHTGRAY);
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Endless Jumping Game");
    SetTargetFPS(60);

    if (!LoadGameAssets())
    {
        TraceLog(LOG_WARNING, "Alguns assets nao carregados, usando fallbacks");
    }

    srand(time(NULL));
    camera.zoom = 1.0f;

    while (!WindowShouldClose())
    {
        switch (gameState)
        {
        case MENU:
            break;

        case PLAYING:
            UpdatePlayer();
            UpdateGameCamera();
            UpdatePlatforms();
            CheckGameOver();
            if (IsKeyPressed(KEY_ESCAPE))
                gameState = MENU;
            break;

        case GAME_OVER:
            if (IsKeyPressed(KEY_ENTER))
            {
                gameState = MENU;
            }
            break;
        }

        BeginDrawing();
        ClearBackground(SKYBLUE);

        switch (gameState)
        {
        case MENU:
            DrawMenu();
            break;

        case PLAYING:
            BeginMode2D(camera);
            DrawParallaxBackground(gameBackgroundTexture, 0.2f); 
            DrawPlatforms();
            DrawPlayer();
            EndMode2D();
            DrawHUD();
            break;

        case GAME_OVER:
            BeginMode2D(camera);
            DrawParallaxBackground(gameBackgroundTexture, 0.2f); 
            DrawPlatforms();
            DrawPlayer();
            EndMode2D();
            DrawGameOver();
            break;
        }

        EndDrawing();
    }

    UnloadGameAssets();
    CloseWindow();
    return 0;
}