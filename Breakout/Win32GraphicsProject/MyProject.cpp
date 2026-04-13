#include "MyProject.h"
#include <Windowsx.h> // for GET__LPARAM macros
#include <SpriteBatch.h>
#include <d3d11.h>
#include <SimpleMath.h>
#include <DirectXColors.h>
#include <sstream>
#include "Collision2D.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

// helper function
float RandFloat() { return float(rand()) / float(RAND_MAX); }

//----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nShowCmd)
{
    MyProject application(hInstance);

    if (application.InitWindowsApp(L"BREAKOUT!", nShowCmd) == false)
    {
        return 0;
    }

    if (application.InitializeDirect3D())
    {
        application.SetDepthStencil(true);
        application.InitializeTextures();
        application.MessageLoop();
    }

    return 0;
}

//----------------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------------
MyProject::MyProject(HINSTANCE hInstance)
    : DirectXClass(hInstance)
{
    mousePos = Vector2(clientWidth * 0.5f, clientHeight * 0.5f);
    buttonDown = false;
    spriteBatch = NULL;
    commonStates = NULL;

    ClearColor = Color(DirectX::Colors::DarkGray.v);

    gameState = MENU;
    score = 0;
    bricksRemaining = 0;
    ballVelocity = Vector2(0, 0);
}

//----------------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------------
MyProject::~MyProject()
{
    delete spriteBatch;
    delete commonStates;
}

//----------------------------------------------------------------------------------------------
// Initialize any textures we need to use here
//----------------------------------------------------------------------------------------------
void MyProject::InitializeTextures()
{
    // initialize the sprite batch and states
    spriteBatch = new DirectX::SpriteBatch(DeviceContext);
    commonStates = new CommonStates(D3DDevice);

    ballTex.Load(D3DDevice, L"..\\Textures\\sphere-04.png");
    brickTex.Load(D3DDevice, L"..\\Textures\\block_purple.png");
    paddleTex.Load(D3DDevice, L"..\\Textures\\paddle.png");

    Vector2 paddlePos((float)clientWidth * 0.5f, (float)clientHeight - 50.0f);
    paddle.Initialize(&paddleTex, paddlePos, 0.0f, 0.5f, Color(DirectX::Colors::White.v), 0.5f);
    paddle.SetPivot(Sprite::Center);

    Vector2 ballPos((float)clientWidth * 0.5f, (float)clientHeight * 0.7f);
    ball.Initialize(&ballTex, ballPos, 0.0f, 0.5f, Color(DirectX::Colors::White.v), 0.5f);
    ball.SetPivot(Sprite::Center);
    ballVelocity = Vector2(400.0f, -400.0f);

    float marginX = 30.0f;
    float marginY = 30.0f;
    float spacingX = 1.0f;
    float spacingY = 1.0f;

    float brickScale = 0.2f;
    int brickW = (int)(brickTex.GetWidth() * brickScale);
    int brickH = (int)(brickTex.GetHeight() * brickScale);

    int index = 0;
    for (int r = 0; r < NUM_ROWS; r++)
    {
        for (int c = 0; c < NUM_COLS; c++)
        {
            float x = marginX + c * (brickW + spacingX);
            float y = marginY + r * (brickH + spacingY);

            bricks[index].sprite.Initialize(&brickTex,Vector2(x, y),0.0f,0.2f,Color(DirectX::Colors::White.v),0.4f);
            bricks[index].sprite.SetPivot(Sprite::Center);
            bricks[index].enabled = true;

            isSpeedBrick[index] = false;
            isSizeBrick[index] = false;
            ++index;
        }
    }

    bricksRemaining = NUM_BRICKS;

    if (NUM_BRICKS > 0)
    {
        isSpeedBrick[0] = true;
        bricks[0].sprite.SetColor(Color(DirectX::Colors::Red.v));
    }
    if (NUM_BRICKS > NUM_COLS + 1)
    {
        isSizeBrick[NUM_COLS + 1] = true;
        bricks[NUM_COLS + 1].sprite.SetColor(Color(DirectX::Colors::Green.v));
    }

    score = 0;

    startButton.Initialize(&paddleTex,
        Vector2((float)clientWidth * 0.5f, (float)clientHeight * 0.4f),0.0f, 2.0f, Color(DirectX::Colors::White.v), 0.3f);
    startButton.SetPivot(Sprite::Center);

    quitButton.Initialize(&paddleTex,
        Vector2((float)clientWidth * 0.5f, (float)clientHeight * 0.8f),0.0f, 2.0f, Color(DirectX::Colors::White.v), 0.3f);
    quitButton.SetPivot(Sprite::Center);

    gameState = MENU;
}

//----------------------------------------------------------------------------------------------
// Window message handler
//----------------------------------------------------------------------------------------------
LRESULT MyProject::ProcessWindowMessages(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_MOUSEMOVE:
        mousePos.x = (float)GET_X_LPARAM(lParam);
        mousePos.y = (float)GET_Y_LPARAM(lParam);
        return 0;

    case WM_LBUTTONUP:
        buttonDown = false;
        mousePos.x = (float)GET_X_LPARAM(lParam);
        mousePos.y = (float)GET_Y_LPARAM(lParam);
        break;

    case WM_LBUTTONDOWN:
        buttonDown = true;
        mousePos.x = (float)GET_X_LPARAM(lParam);
        mousePos.y = (float)GET_Y_LPARAM(lParam);
        OnMouseDown();
        break;
    }

    return DirectXClass::ProcessWindowMessages(msg, wParam, lParam);
}

//----------------------------------------------------------------------------------------------
// Called when the mouse is pressed
//----------------------------------------------------------------------------------------------
void MyProject::OnMouseDown()
{
    if (gameState == MENU)
    {
        if (startButton.ContainsPoint(mousePos))
        {
            score = 0;
            bricksRemaining = NUM_BRICKS;

            int index = 0;
            float marginX = 60.0f;
            float marginY = 30.0f;
            float spacingX = 1.0f;
            float spacingY = 1.0f;
            float brickScale = 0.2f;
            int brickW = (int)(brickTex.GetWidth() * brickScale);
            int brickH = (int)(brickTex.GetHeight() * brickScale);

            for (int r = 0; r < NUM_ROWS; r++)
            {
                for (int c = 0; c < NUM_COLS; c++)
                {
                    float x = marginX + c * (brickW + spacingX);
                    float y = marginY + r * (brickH + spacingY);

                    bricks[index].sprite.SetPosition(Vector2(x, y));
                    bricks[index].enabled = true;
                    bricks[index].sprite.SetColor(Color(DirectX::Colors::White.v));
                    isSpeedBrick[index] = false;
                    isSizeBrick[index] = false;
                    ++index;
                }
            }

            if (NUM_BRICKS > 0)
            {
                isSpeedBrick[0] = true;
                bricks[0].sprite.SetColor(Color(DirectX::Colors::Red.v));
            }
            if (NUM_BRICKS > NUM_COLS + 1)
            {
                isSizeBrick[NUM_COLS + 1] = true;
                bricks[NUM_COLS + 1].sprite.SetColor(Color(DirectX::Colors::Green.v));
            }

            paddle.SetScale(0.5f);
            paddle.SetPosition(Vector2((float)clientWidth * 0.5f, (float)clientHeight - 50.0f));

            ball.SetPosition(Vector2((float)clientWidth * 0.5f, (float)clientHeight * 0.7f));
            ballVelocity = Vector2(400.0f, -400.0f);

            gameState = PLAYING;
        }
        else if (quitButton.ContainsPoint(mousePos))
        {
            PostQuitMessage(0);
        }
    }
    else if (gameState == WIN || gameState == GAME_OVER)
    {
        gameState = MENU;
    }
}

//----------------------------------------------------------------------------------------------
// Called by the render loop to render a single frame
//----------------------------------------------------------------------------------------------
void MyProject::Render(void)
{
    spriteBatch->Begin(SpriteSortMode_BackToFront, commonStates->NonPremultiplied());

    if (gameState == MENU)
    {
        startButton.Draw(spriteBatch);
        quitButton.Draw(spriteBatch);

        font.PrintMessage(clientWidth / 2 - 90, clientHeight / 2 - 150, L"BREAKOUT", FC_GREEN);
        font.PrintMessage((int)(startButton.GetPosition().x - 40),(int)(startButton.GetPosition().y - 10),L"Start", FC_BLACK);
        font.PrintMessage((int)(quitButton.GetPosition().x - 35),(int)(quitButton.GetPosition().y - 10),L"Quit", FC_BLACK);
    }
    else
    {
        for (int i = 0; i < NUM_BRICKS; i++)
        {
            if (bricks[i].enabled) bricks[i].sprite.Draw(spriteBatch);
        }

        paddle.Draw(spriteBatch);
        ball.Draw(spriteBatch);

        std::wostringstream ss;
        ss << L"Score: " << score;
        font.PrintMessage(10, 10, ss.str().c_str(), FC_GREEN);

        if (gameState == WIN)
        {
            font.PrintMessage(clientWidth / 2 - 60, clientHeight / 2 - 20, L"You Win!", FC_BLUE);
            font.PrintMessage(clientWidth / 2 - 140, clientHeight / 2 + 20, L"Click to return to menu", FC_BLUE);
        }
        else if (gameState == GAME_OVER)
        {
            font.PrintMessage(clientWidth / 2 - 70, clientHeight / 2 - 20, L"Game Over", FC_RED);
            font.PrintMessage(clientWidth / 2 - 140, clientHeight / 2 + 20, L"Click to return to menu", FC_RED);
        }
    }

    spriteBatch->End();

    DirectXClass::Render();
}

//----------------------------------------------------------------------------------------------
// Called every frame to update objects.
//----------------------------------------------------------------------------------------------
void MyProject::Update(float deltaTime)
{
    if (gameState != PLAYING) return;

    Vector2 paddlePos = paddle.GetPosition();
    paddlePos.x = mousePos.x;

    Vector2 paddleExt = paddle.GetExtents();
    float halfW = paddleExt.x;

    if (paddlePos.x < halfW) paddlePos.x = halfW;
    if (paddlePos.x > clientWidth - halfW) paddlePos.x = (float)clientWidth - halfW;
    paddle.SetPosition(paddlePos);

    Vector2 ballPos = ball.GetPosition();
    ballPos += ballVelocity * deltaTime;

    float radius = ball.GetExtents().x;

    if (ballPos.x - radius < 0.0f)
    {
        ballPos.x = radius;
        ballVelocity.x *= -1.0f;
    }
    if (ballPos.x + radius > (float)clientWidth)
    {
        ballPos.x = (float)clientWidth - radius;
        ballVelocity.x *= -1.0f;
    }
    if (ballPos.y - radius < 0.0f)
    {
        ballPos.y = radius;
        ballVelocity.y *= -1.0f;
    }
    if (ballPos.y + radius > (float)clientHeight)
    {
        ballPos.y = (float)clientHeight - radius;
        gameState = GAME_OVER;
    }

    Circle ballCircle(ballPos, radius);

    Box2D paddleBox(paddlePos, paddleExt);
    if (Collision2D::BoxCircleCheck(paddleBox, ballCircle))
    {
        ballPos.y = paddlePos.y - paddleExt.y - radius - 1.0f;
        ballCircle.center = ballPos;

        float t = (ballPos.x - paddlePos.x) / paddleExt.x;
        ballVelocity.y = -fabsf(ballVelocity.y);
        ballVelocity.x = 200.0f * t;
    }

    for (int i = 0; i < NUM_BRICKS; i++)
    {
        if (!bricks[i].enabled) continue;

        Vector2 bPos = bricks[i].sprite.GetPosition();
        Vector2 bExt = bricks[i].sprite.GetExtents();
        Box2D brickBox(bPos, bExt);

        if (Collision2D::BoxCircleCheck(brickBox, ballCircle))
        {
            bricks[i].enabled = false;
            bricksRemaining--;
            score += 10;

            ballVelocity.y *= -1.0f;

            if (isSpeedBrick[i])
            {
                ballVelocity *= 1.3f;
            }
            if (isSizeBrick[i])
            {
                float s = paddle.GetScale();
                paddle.SetScale(s * 1.3f);
            }

            break;
        }
    }

    if (bricksRemaining <= 0)
    {
        gameState = WIN;
    }

    ball.SetPosition(ballPos);
}
