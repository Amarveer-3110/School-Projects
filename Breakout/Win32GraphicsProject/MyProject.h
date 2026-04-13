#ifndef _MyProject_h
#define _MyProject_h
#include  <CommonStates.h>
#include "DirectX.h"
#include "Font.h"
#include "TextureType.h"
#include "Sprite.h"
#include "Collision2D.h"

// forward declare the sprite batch
namespace DirectX { class SpriteBatch; };

struct Block
{
    Sprite sprite;
    bool   enabled;
};

//----------------------------------------------------------------------------------------------
// Main project class
//----------------------------------------------------------------------------------------------
class MyProject : public DirectXClass
{
public:
    MyProject(HINSTANCE hInstance);
    ~MyProject();

    void InitializeTextures();
    LRESULT ProcessWindowMessages(UINT msg, WPARAM wParam, LPARAM lParam);
    void Render(void);
    void Update(float deltaTime);

private:
    // sprite batch 
    DirectX::SpriteBatch* spriteBatch;
    DirectX::CommonStates* commonStates;

    Vector2 mousePos;
    bool buttonDown;

    void OnMouseDown();

    enum GameState
    {
        MENU,
        PLAYING,
        WIN,
        GAME_OVER
    };

    GameState gameState;

    TextureType ballTex;
    TextureType brickTex;
    TextureType paddleTex;

    Sprite ball;
    Sprite paddle;

    static const int NUM_ROWS = 2;
    static const int NUM_COLS = 13;
    static const int NUM_BRICKS = NUM_ROWS * NUM_COLS;
    Block bricks[NUM_BRICKS];

    bool isSpeedBrick[NUM_BRICKS];
    bool isSizeBrick[NUM_BRICKS];

    Vector2 ballVelocity;

    int score;
    int bricksRemaining;

    Sprite startButton;
    Sprite quitButton;
};

#endif
