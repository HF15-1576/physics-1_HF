/*
This project uses the Raylib framework to provide us functionality for math, graphics, GUI, input etc.
See documentation here: https://www.raylib.com/, and examples here: https://www.raylib.com/examples.html
*/

#include "raylib.h"
#include "raymath.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

const unsigned int TARGET_FPS = 50; // Target frames per second
float dt = 1.0f / TARGET_FPS; // Delta time (time per frame)
float time = 0;

// Launch variables
Vector2 launchPos = { 600, 700 };
float launchAngle = 30.0f;
float launchSpeed = 100.0f;

Vector2 velocity = { 0, 0 };

void update()
{
		dt = 1.0f / TARGET_FPS;
        time += dt;

		// Update launch parameters with GUI
		float rad = launchAngle * DEG2RAD;
		velocity.x = launchSpeed * cos(rad);
        velocity.y = launchSpeed * -sin(rad);
		
}

void draw()
{
        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Enraged Birds launch distance", 10, GetScreenHeight() - 30, 20, DARKPURPLE);


			GuiSliderBar(Rectangle{ 60, 5, 200, 10 }, " X Point", TextFormat("%.0f", launchPos.x), &launchPos.x, 0, GetScreenWidth());
			GuiSliderBar(Rectangle{ 350, 5, 200, 10 }, " Y Point", TextFormat("%.0f", launchPos.y), &launchPos.y, 0, GetScreenHeight());
			GuiSliderBar(Rectangle{ 640, 5, 200, 10 }, " Angle", TextFormat("%.0f", launchAngle), &launchAngle, 1, 90);
			GuiSliderBar(Rectangle{ 930, 5, 200, 10 }, " Speed", TextFormat("%.0f", launchSpeed), &launchSpeed, 0, 1500);

			float visualScale = 0.2f;
			Vector2 velocityVector = Vector2Scale(velocity, visualScale);
			Vector2 endPos = Vector2Add(launchPos, velocityVector);
			DrawLineV(launchPos, endPos, RED);
			DrawText("Velocity", endPos.x + 10, endPos.y, 10, RED);

			



        EndDrawing();

}


int main()
{
    InitWindow(1200, 800, "GAME2005-Hunter_Faichney-101547907");
    SetTargetFPS(TARGET_FPS);

	while (!WindowShouldClose()) //Calls towards update and draw methods
    {
		update();
		draw();

    }

    CloseWindow();
    return 0;
}
