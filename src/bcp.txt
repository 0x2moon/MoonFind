#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// Constantes do jogo
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define GRAVITY 980.0f
#define JUMP_FORCE -400.0f
#define PLAYER_SPEED 200.0f
#define MAX_PLATFORMS 20
#define PLATFORM_MIN_GAP 50   // Reduzido de 80
#define PLATFORM_MAX_GAP 100  // Reduzido de 125
#define MAX_HORIZONTAL_GAP 160 // Nova constante
#define PLAYER_HITBOX_WIDTH 32
#define PLAYER_HITBOX_HEIGHT 32
#define MAX_FALL_SPEED 800.0f

// Estados do jogo
typedef enum {
    MENU,
    PLAYING,
    GAME_OVER
} GameState;

// Estados do jogador
typedef enum {
    IDLE,
    WALKING,
    JUMPING
} PlayerState;

// Tipos de plataforma
typedef enum {
    PLATFORM_TYPE_1,
    PLATFORM_TYPE_2,
    PLATFORM_TYPE_3
} PlatformType;

// Estrutura para animações
typedef struct {
    Texture2D texture;
    int frames;
    int currentFrame;
    float frameTime;
    float elapsedTime;
    float frameWidth;
    float frameHeight;
} Animation;

// Estrutura do jogador
typedef struct {
    Vector2 position;
    Vector2 velocity;
    Rectangle hitbox;
    Rectangle previousHitbox; // Hitbox do frame anterior
    PlayerState state;
    PlayerState prevState;
    bool facingRight;
    bool onGround;
    int currentPlatform;

    // Animações
    Animation idleAnim;
    Animation walkAnim;
    Animation jumpAnim;
} Player;

// Estrutura das plataformas
typedef struct {
    Rectangle rect;
    PlatformType type;
    bool active;
} Platform;

// Variáveis globais
GameState gameState = MENU;
Player player;
Platform platforms[MAX_PLATFORMS];
Camera2D camera = {0};
float score = 0.0f;
float highScore = 0.0f;
float gameSpeed = 1.0f;
float startYPosition = 0;

// Texturas
Texture2D menuBackgroundTexture;
Texture2D startButtonTexture;
Texture2D gameBackgroundTexture;
Texture2D platform1Texture;
Texture2D platform2Texture;
Texture2D platform3Texture;
Texture2D platformTextures[3];

// Declarações de funções
bool LoadGameAssets();
void UnloadGameAssets();
void InitPlayer();
void InitPlatforms();
void GeneratePlatform(float refX, float refY); // Alterado para receber referência x e y
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

// Função para carregar texturas
bool LoadGameAssets() {
    // Menu assets
    menuBackgroundTexture = LoadTexture("assets/titleScreen/background.png");
    startButtonTexture = LoadTexture("assets/titleScreen/start_button.png");

    // Gameplay assets
    gameBackgroundTexture = LoadTexture("assets/gameplay/background.png");
    platform1Texture = LoadTexture("assets/gameplay/plataforma1.png");
    platform2Texture = LoadTexture("assets/gameplay/plataforma2.png");
    platform3Texture = LoadTexture("assets/gameplay/plataforma3.png");

    // Player assets
    player.idleAnim.texture = LoadTexture("assets/player/player_Idle.png");
    player.walkAnim.texture = LoadTexture("assets/player/player_walk.png");
    player.jumpAnim.texture = LoadTexture("assets/player/player_Jump.png");

    // Configurar array de plataformas
    platformTextures[PLATFORM_TYPE_1] = platform1Texture;
    platformTextures[PLATFORM_TYPE_2] = platform2Texture;
    platformTextures[PLATFORM_TYPE_3] = platform3Texture;

    // Verificar se as texturas foram carregadas
    bool allLoaded = true;

    if (menuBackgroundTexture.id == 0) {
        TraceLog(LOG_WARNING, "AVISO: assets/titleScreen/background.png nao carregado");
        allLoaded = false;
    }
    if (startButtonTexture.id == 0) {
        TraceLog(LOG_WARNING, "AVISO: assets/titleScreen/start_button.png nao carregado");
        allLoaded = false;
    }
    if (gameBackgroundTexture.id == 0) {
        TraceLog(LOG_WARNING, "AVISO: assets/gameplay/background.png nao carregado");
        allLoaded = false;
    }
    if (platform1Texture.id == 0) {
        TraceLog(LOG_WARNING, "AVISO: assets/gameplay/plataforma1.png nao carregado");
        allLoaded = false;
    }
    if (platform2Texture.id == 0) {
        TraceLog(LOG_WARNING, "AVISO: assets/gameplay/plataforma2.png nao carregado");
        allLoaded = false;
    }
    if (platform3Texture.id == 0) {
        TraceLog(LOG_WARNING, "AVISO: assets/gameplay/plataforma3.png nao carregado");
        allLoaded = false;
    }
    if (player.idleAnim.texture.id == 0) {
        TraceLog(LOG_WARNING, "AVISO: assets/player/player_Idle.png nao carregado");
        allLoaded = false;
    }
    if (player.walkAnim.texture.id == 0) {
        TraceLog(LOG_WARNING, "AVISO: assets/player/player_walk.png nao carregado");
        allLoaded = false;
    }
    if (player.jumpAnim.texture.id == 0) {
        TraceLog(LOG_WARNING, "AVISO: assets/player/player_Jump.png nao carregado");
        allLoaded = false;
    }

    return allLoaded;
}

// Função para descarregar texturas
void UnloadGameAssets() {
    UnloadTexture(menuBackgroundTexture);
    UnloadTexture(startButtonTexture);
    UnloadTexture(gameBackgroundTexture);
    UnloadTexture(platform1Texture);
    UnloadTexture(platform2Texture);
    UnloadTexture(platform3Texture);
    UnloadTexture(player.idleAnim.texture);
    UnloadTexture(player.walkAnim.texture);
    UnloadTexture(player.jumpAnim.texture);
}

// Inicializar jogador
void InitPlayer() {
    player.position = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT - 150};
    player.velocity = (Vector2){0, 0};
    
    player.hitbox = (Rectangle){
        player.position.x - PLAYER_HITBOX_WIDTH / 2,
        player.position.y - PLAYER_HITBOX_HEIGHT,
        PLAYER_HITBOX_WIDTH,
        PLAYER_HITBOX_HEIGHT
    };
    player.previousHitbox = player.hitbox; // Inicializar previousHitbox

    player.state = IDLE;
    player.prevState = IDLE;
    player.facingRight = true;
    player.onGround = true;
    player.currentPlatform = 0;

    // Configurar animação IDLE
    player.idleAnim.frames = 5;
    player.idleAnim.frameTime = 0.15f;
    player.idleAnim.currentFrame = 0;
    player.idleAnim.elapsedTime = 0.0f;
    player.idleAnim.frameWidth = player.idleAnim.texture.width / player.idleAnim.frames;
    player.idleAnim.frameHeight = player.idleAnim.texture.height;

    // Configurar animação WALK
    player.walkAnim.frames = 8;
    player.walkAnim.frameTime = 0.1f;
    player.walkAnim.currentFrame = 0;
    player.walkAnim.elapsedTime = 0.0f;
    player.walkAnim.frameWidth = player.walkAnim.texture.width / player.walkAnim.frames;
    player.walkAnim.frameHeight = player.walkAnim.texture.height;

    // Configurar animação JUMP
    player.jumpAnim.frames = 8;
    player.jumpAnim.frameTime = 0.1f;
    player.jumpAnim.currentFrame = 0;
    player.jumpAnim.elapsedTime = 0.0f;
    player.jumpAnim.frameWidth = player.jumpAnim.texture.width / player.jumpAnim.frames;
    player.jumpAnim.frameHeight = player.jumpAnim.texture.height;

    startYPosition = player.position.y;
}

// Inicializar plataformas
void InitPlatforms() {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        platforms[i].active = false;
    }

    // Plataforma inicial
    platforms[0] = (Platform){
        .rect = {
            SCREEN_WIDTH / 2 - 100,
            SCREEN_HEIGHT - 50,
            200,
            32
        },
        .type = PLATFORM_TYPE_1,
        .active = true
    };

    // Gerar plataformas iniciais com distâncias controladas
    float lastX = SCREEN_WIDTH / 2; // Centro da plataforma inicial
    float lastY = SCREEN_HEIGHT - 50;
    
    for (int i = 1; i < 8; i++) {
        int gap = GetRandomValue(PLATFORM_MIN_GAP, PLATFORM_MAX_GAP);
        lastY -= gap;
        
        // Nova posição baseada na anterior com limite horizontal
        int offsetX = GetRandomValue(-MAX_HORIZONTAL_GAP, MAX_HORIZONTAL_GAP);
        lastX += offsetX;
        
        // Garantir que não sai da tela
        if (lastX < 100) lastX = 100;
        else if (lastX > SCREEN_WIDTH - 100) lastX = SCREEN_WIDTH - 100;
        
        float width = GetRandomValue(80, 180);
        
        platforms[i] = (Platform){
            .rect = {
                lastX - width/2, // Centralizado
                lastY,
                width,
                32
            },
            .type = (PlatformType)GetRandomValue(0, 2),
            .active = true
        };
    }
}

// Gerar nova plataforma baseada na plataforma anterior
void GeneratePlatform(float refX, float refY) {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!platforms[i].active) {
            int gap = GetRandomValue(PLATFORM_MIN_GAP, PLATFORM_MAX_GAP);
            float newY = refY - gap;
            
            // Distância horizontal limitada (máx 160 pixels)
            int offsetX = GetRandomValue(-MAX_HORIZONTAL_GAP, MAX_HORIZONTAL_GAP);
            float newX = refX + offsetX;
            
            // Garantir que não sai dos limites da tela
            if (newX < 50) newX = 50;
            else if (newX > SCREEN_WIDTH - 50) newX = SCREEN_WIDTH - 50;
            
            float width = GetRandomValue(80, 180);
            
            platforms[i] = (Platform){
                .rect = {
                    newX - width/2, // Centralizado no ponto X
                    newY,
                    width,
                    32
                },
                .type = (PlatformType)GetRandomValue(0, 2),
                .active = true
            };
            return;
        }
    }
}

// Atualizar animação
void UpdateAnimation(Animation *anim, float deltaTime, bool reset) {
    if (reset) {
        anim->currentFrame = 0;
        anim->elapsedTime = 0.0f;
        return;
    }

    anim->elapsedTime += deltaTime;
    if (anim->elapsedTime >= anim->frameTime) {
        anim->currentFrame = (anim->currentFrame + 1) % anim->frames;
        anim->elapsedTime = 0.0f;
    }
}

// Atualizar jogador - COLISÃO CORRIGIDA
void UpdatePlayer() {
    // Salvar hitbox anterior
    player.previousHitbox = player.hitbox;

    player.prevState = player.state;

    // Input horizontal
    bool moving = false;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        player.velocity.x = -PLAYER_SPEED * gameSpeed;
        player.facingRight = false;
        moving = true;
    }
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        player.velocity.x = PLAYER_SPEED * gameSpeed;
        player.facingRight = true;
        moving = true;
    }
    else {
        player.velocity.x = 0;
    }

    // Determinar estado
    if (!player.onGround) {
        player.state = JUMPING;
    }
    else if (moving) {
        player.state = WALKING;
    }
    else {
        player.state = IDLE;
    }

    // Pulo
    if ((IsKeyPressed(KEY_SPACE) && player.onGround)) {
        player.velocity.y = JUMP_FORCE;
        player.onGround = false;
        player.state = JUMPING;
        platforms[player.currentPlatform].active = false;
        
        // Gerar nova plataforma baseada na posição da plataforma atual
        Platform* current = &platforms[player.currentPlatform];
        float refX = current->rect.x + current->rect.width/2; // Centro da plataforma
        float refY = current->rect.y;
        GeneratePlatform(refX, refY);
    }

    // Aplicar gravidade
    player.velocity.y += GRAVITY * GetFrameTime() * gameSpeed;
    
    // Limitar velocidade de queda
    if (player.velocity.y > MAX_FALL_SPEED) {
        player.velocity.y = MAX_FALL_SPEED;
    }

    // Atualizar posição
    player.position.x += player.velocity.x * GetFrameTime();
    player.position.y += player.velocity.y * GetFrameTime();

    // Atualizar hitbox
    player.hitbox.x = player.position.x - PLAYER_HITBOX_WIDTH / 2;
    player.hitbox.y = player.position.y - PLAYER_HITBOX_HEIGHT;

    // Verificar colisões com plataformas - SISTEMA CORRIGIDO
    player.onGround = false;
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!platforms[i].active) continue;

        if (player.velocity.y >= 0) {
            // Calcular limites
            float previousBottom = player.previousHitbox.y + player.previousHitbox.height;
            float currentBottom = player.hitbox.y + player.hitbox.height;
            float platformTop = platforms[i].rect.y;
            
            // Verificar se o jogador atravessou a plataforma
            if (previousBottom <= platformTop && currentBottom >= platformTop) {
                // Verificar sobreposição horizontal
                if (player.hitbox.x + player.hitbox.width > platforms[i].rect.x &&
                    player.hitbox.x < platforms[i].rect.x + platforms[i].rect.width) {
                    
                    // CORREÇÃO: Posicionar base do jogador no topo da plataforma
                    player.position.y = platformTop;
                    
                    // Atualizar hitbox imediatamente
                    player.hitbox.y = player.position.y - PLAYER_HITBOX_HEIGHT;
                    
                    player.velocity.y = 0;
                    player.onGround = true;
                    player.currentPlatform = i;
                    break;
                }
            }
        }
    }

    // Atualizar animação
    bool resetAnimation = (player.prevState != player.state);

    switch (player.state) {
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

    // Atualizar pontuação
    float heightDifference = startYPosition - player.position.y;
    if (heightDifference > score) {
        score = heightDifference;
    }

    // Aumentar velocidade do jogo
    gameSpeed = 1.0f + (score * 0.0005f);
    if (gameSpeed > 2.5f) gameSpeed = 2.5f;
}

// Atualizar câmera
void UpdateGameCamera() {
    camera.target.x = player.position.x;
    camera.target.y = player.position.y - SCREEN_HEIGHT / 3;

    // Limitar movimento vertical
    if (camera.target.y < SCREEN_HEIGHT / 2) {
        camera.target.y = SCREEN_HEIGHT / 2;
    }

    camera.offset = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
}

// Atualizar plataformas
void UpdatePlatforms() {
    // Remover plataformas abaixo da tela
    float bottomLimit = camera.target.y + SCREEN_HEIGHT + 100;
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (platforms[i].active && platforms[i].rect.y > bottomLimit) {
            platforms[i].active = false;
        }
    }

    // Gerar novas plataformas
    float highestY = camera.target.y - SCREEN_HEIGHT;
    bool needPlatforms = true;

    while (needPlatforms) {
        needPlatforms = false;
        float lowestPlatform = camera.target.y + SCREEN_HEIGHT;

        for (int i = 0; i < MAX_PLATFORMS; i++) {
            if (platforms[i].active && platforms[i].rect.y < lowestPlatform) {
                lowestPlatform = platforms[i].rect.y;
            }
        }

        if (lowestPlatform > highestY) {
            // Para geração acima, usamos a plataforma mais baixa como referência
            // Mas precisamos de uma posição de referência (x) para a geração
            // Vamos usar a posição x do jogador
            GeneratePlatform(player.position.x, lowestPlatform);
            needPlatforms = true;
        }
    }
}

// Verificar game over
void CheckGameOver() {
    if (player.position.y > camera.target.y + SCREEN_HEIGHT + 100) {
        gameState = GAME_OVER;
        if (score > highScore) highScore = score;
    }
}

// Desenhar background em paralaxe
void DrawParallaxBackground(Texture2D texture, float parallaxFactor) {
    if (texture.id == 0) return;

    Vector2 pos = {
        camera.target.x - texture.width / 2,
        (camera.target.y * parallaxFactor) - texture.height / 2
    };

    // Desenhar textura repetida
    int startX = (int)(pos.x / texture.width) - 1;
    int endX = startX + SCREEN_WIDTH / texture.width + 2;

    int startY = (int)(pos.y / texture.height) - 1;
    int endY = startY + SCREEN_HEIGHT / texture.height + 2;

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            DrawTexture(texture, x * texture.width, y * texture.height, WHITE);
        }
    }
}

// Desenhar jogador com animação
void DrawPlayer() {
    Animation *currentAnim = NULL;

    switch (player.state) {
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

    if (currentAnim->texture.id == 0) {
        DrawRectangleRec(player.hitbox, 
                        player.state == JUMPING ? RED : 
                        player.state == WALKING ? BLUE : GREEN);
        return;
    }

    // Definir retângulo de origem
    Rectangle src;
    src.y = 0.0f;
    src.height = (float)currentAnim->frameHeight;

    if (player.facingRight) {
        src.x = (float)currentAnim->currentFrame * currentAnim->frameWidth;
        src.width = (float)currentAnim->frameWidth;
    } else {
        src.x = (float)(currentAnim->currentFrame + 1) * currentAnim->frameWidth;
        src.width = -(float)currentAnim->frameWidth;
    }

    // Ajuste para alinhar visual com hitbox
    Rectangle dest = {
        player.position.x,
        player.position.y,
        (float)currentAnim->frameWidth,
        (float)currentAnim->frameHeight
    };

    Vector2 origin = {
        (float)currentAnim->frameWidth / 2.0f,
        (float)currentAnim->frameHeight
    };
    
    DrawTexturePro(currentAnim->texture, src, dest, origin, 0.0f, WHITE);
}

// Desenhar plataformas com alinhamento correto
void DrawPlatforms() {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!platforms[i].active) continue;

        if (platformTextures[platforms[i].type].id != 0) {
            DrawTextureRec(
                platformTextures[platforms[i].type],
                (Rectangle){0, 0, platforms[i].rect.width, platforms[i].rect.height},
                (Vector2){platforms[i].rect.x, platforms[i].rect.y},
                WHITE
            );
        } else {
            Color colors[] = {BROWN, DARKBROWN, BEIGE};
            DrawRectangleRec(platforms[i].rect, colors[platforms[i].type]);
        }
    }
}

// Desenhar menu
void DrawMenu() {
    // Background
    if (menuBackgroundTexture.id != 0) {
        DrawTexture(menuBackgroundTexture, 0, 0, WHITE);
    } else {
        ClearBackground(DARKBLUE);
        DrawText("ENDLESS JUMPING", SCREEN_WIDTH / 2 - 180, 150, 40, WHITE);
    }

    // Botão de start
    Rectangle btnRect = {SCREEN_WIDTH / 2 - 128, SCREEN_HEIGHT / 2, 256, 128};

    if (startButtonTexture.id != 0) {
        DrawTexture(startButtonTexture, btnRect.x, btnRect.y, WHITE);
    } else {
        DrawRectangleRec(btnRect, GREEN);
        DrawText("PLAY", SCREEN_WIDTH / 2 - 40, SCREEN_HEIGHT / 2 + 40, 40, WHITE);
    }

    // Verificar clique
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, btnRect)) {
            gameState = PLAYING;
            InitPlayer();
            InitPlatforms();
            score = 0.0f;
            gameSpeed = 1.0f;
            camera.target = (Vector2){0};
        }
    }

    // High Score
    if (highScore > 0) {
        DrawText(TextFormat("High Score: %.0f", highScore),
                 SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 100, 20, YELLOW);
    }

    // Instruções
    DrawText("Use A/D para mover e ESPACO para pular",
             SCREEN_WIDTH / 2 - 180, SCREEN_HEIGHT - 50, 20, LIGHTGRAY);
}

// Desenhar game over
void DrawGameOver() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.7f));

    DrawText("GAME OVER", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 100, 40, RED);
    DrawText(TextFormat("Score: %.0f", score), SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 40, 30, WHITE);

    if (highScore > 0) {
        DrawText(TextFormat("High Score: %.0f", highScore),
                 SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, 30, YELLOW);
    }

    DrawText("Press R to restart or ESC to menu",
             SCREEN_WIDTH / 2 - 180, SCREEN_HEIGHT / 2 + 60, 20, WHITE);

    if (IsKeyPressed(KEY_R)) {
        gameState = PLAYING;
        InitPlayer();
        InitPlatforms();
        score = 0.0f;
        gameSpeed = 1.0f;
    } else if (IsKeyPressed(KEY_ESCAPE)) {
        gameState = MENU;
        camera.target = (Vector2){0};
    }
}

// Desenhar HUD
void DrawHUD() {
    DrawText(TextFormat("Score: %.0f", score), 10, 10, 20, WHITE);
    DrawText(TextFormat("Speed: %.1fx", gameSpeed), 10, 35, 16, LIGHTGRAY);
    DrawText("ESC: Menu", SCREEN_WIDTH - 100, 10, 20, LIGHTGRAY);
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Endless Jumping Game");
    SetTargetFPS(60);

    if (!LoadGameAssets()) {
        TraceLog(LOG_WARNING, "Alguns assets nao carregados, usando fallbacks");
    }

    srand(time(NULL));
    InitPlayer();
    InitPlatforms();
    camera.zoom = 1.0f;

    while (!WindowShouldClose()) {
        // Atualização
        switch (gameState) {
            case MENU:
                break;
                
            case PLAYING:
                UpdatePlayer();
                UpdateGameCamera();
                UpdatePlatforms();
                CheckGameOver();
                if (IsKeyPressed(KEY_ESCAPE)) gameState = MENU;
                break;
                
            case GAME_OVER:
                if (IsKeyPressed(KEY_ESCAPE)) gameState = MENU;
                break;
        }

        // Renderização
        BeginDrawing();
        ClearBackground(SKYBLUE);

        switch (gameState) {
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