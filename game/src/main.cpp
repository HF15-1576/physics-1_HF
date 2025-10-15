/*
This project uses the Raylib framework to provide us functionality for math, graphics, GUI, input etc.
See documentation here: https://www.raylib.com/, and examples here: https://www.raylib.com/examples.html
*/

#include "raylib.h"
#include "raymath.h"
#include <vector>

struct PhysicsBody
{
    Vector2 position = Vector2Zeros;
    Vector2 velocity = Vector2Zeros;
	float drag = 0.0f; 
    float invMass = 1.0f;
	float gravityScale = 1.0f; 

    bool collision = false;

    float radius = 0.0f;

};

// Physics Simulation
struct PhysicsWorld
{
    Vector2 gravity = { 0.0f, 150.0f };
    std::vector<PhysicsBody> entities;
};

bool CircleCircle(Vector2 pos1, float rad1, Vector2 pos2, float rad2)
{
    // Circle should turn red when overlapping once this function is implemented correctly.
    float dx = pos2.x - pos1.x;
    float dy = pos2.y - pos1.y;
    float distanceSq = dx * dx + dy * dy;
    float radiusSum = rad1 + rad2;
    return distanceSq <= (radiusSum * radiusSum);
}

int main()
{
    InitWindow(1200, 800, "(GAME2005)_Hunter-Faichney");
    SetTargetFPS(60);

	PhysicsWorld world;
    world.entities.push_back({}); 
    
    // Static Circle
	PhysicsBody* circle = &world.entities.back();
	circle->position = { 600.0f, 400.0f };
	circle->radius = 40.0f;
	circle->gravityScale = 0.0f;

    // Moving Circle 
	world.entities.push_back({});
	circle = &world.entities.back();
	circle->radius = 30.0f;
	circle->gravityScale = 0.0f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        circle->position = GetMousePosition();

        // Motion loop
        for (size_t i = 0; i < world.entities.size(); i++)
        {
            PhysicsBody& e = world.entities[i];
            Vector2 acc = world.gravity * e.gravityScale;

            e.velocity += acc * dt;             // v = a * t
            e.position += e.velocity * dt;      // p = v * t

            // Reset collision render status
            e.collision = false;
        }

        // Collision loop (Test every object against all other objects)
        for (size_t i = 0; i < world.entities.size(); i++)
        {
            for (size_t j = i + 1; j < world.entities.size(); j++)
            {
                PhysicsBody& a = world.entities[i];
                PhysicsBody& b = world.entities[j];
                bool collision = CircleCircle(a.position, a.radius, b.position, b.radius);
                a.collision |= collision;
                b.collision |= collision;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);
        for (const PhysicsBody& e : world.entities)
        {
            DrawCircleV(e.position, e.radius, e.collision ? RED : GREEN);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
