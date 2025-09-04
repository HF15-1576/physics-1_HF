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
float x = 650;
float y = 500;
float frequency = 1.0f;
float amplitude = 100.0f;

void update()
{
		dt = 1.0f / TARGET_FPS;
        time += dt;

		x = x + (-sin(time * frequency)) * frequency * amplitude * dt;
		y = y + (cos(time * frequency)) * frequency * amplitude * dt;
}

void draw()
{
        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Uhh Stinky", 440, 250, 50, DARKPURPLE);


            GuiSliderBar(Rectangle{ 60, 770, 1000, 10 }, "Time", TextFormat("%.2f", time), &time, 0, 30);

			DrawCircle(x, y, 80, PINK);



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
