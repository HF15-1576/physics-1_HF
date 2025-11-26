#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <cassert>

enum ColliderType
{
    COLLIDER_TYPE_INVALID,
    COLLIDER_TYPE_CIRCLE,
    COLLIDER_TYPE_HALF_SPACE
    // COLLIDER_BOX <-- not implemented yet
};

struct CircleCollider
{
    float radius = 0.0f;
};

struct HalfSpaceCollider
{
    Vector2 normal = { 0.0f, 1.0f };
};

struct BoxCollider
{
    Vector2 extents = { 0.0f, 0.0f };
};

struct PhysicsBody
{
    Vector2 position = Vector2Zeros;
    Vector2 velocity = Vector2Zeros;
    Vector2 net_force = Vector2Zeros;

    float drag = 1.0f;
    float inv_mass = 1.0f;
    float gravity_scale = 1.0f;

    float friction_coeff = 1.0f;
    float restitution_coeff = 1.0f;

    ColliderType collider_type = COLLIDER_TYPE_INVALID;

    CircleCollider circle;
    HalfSpaceCollider halfspace;
    BoxCollider box;

    bool collision = false;
};

struct HitPair
{
    PhysicsBody* a = nullptr;
    PhysicsBody* b = nullptr;
    Vector2 mtv = Vector2Zeros;
};

// Physics Simulation
struct PhysicsWorld
{
    Vector2 gravity = { 0.0f, 9.81f };
    std::vector<PhysicsBody> entities;
};

// MTV points FROM 2 TO 1
bool CircleCircle(Vector2 pos1, float rad1, Vector2 pos2, float rad2, Vector2* mtv = nullptr)
{
    float radii_sum = rad1 + rad2;
    float distance = Vector2Distance(pos1, pos2);
    bool collision = distance <= radii_sum;

    if (collision && mtv != nullptr)
    {
        float mtv_magnitude = radii_sum - distance;
        Vector2 mtv_direction = Vector2Normalize(pos1 - pos2);
        *mtv = mtv_direction * mtv_magnitude;
    }

    return collision;
}

// MTV points FROM half-space TO circle
bool CircleHalfSpace(Vector2 pos_circle, float rad, Vector2 pos_half_space, Vector2 normal, Vector2* mtv = nullptr)
{
    Vector2 to_circle = pos_circle - pos_half_space;
    float proj = Vector2DotProduct(to_circle, normal);
    bool collision = proj <= rad;

    if (collision && mtv != nullptr)
    {
        float mtv_magnitude = rad - proj;
        *mtv = normal * mtv_magnitude;
    }

    return collision;
}

bool IsMassInfinite(const PhysicsBody& entity)
{
    return entity.inv_mass <= FLT_EPSILON;
}

// Function declarations
void UpdateMotion(PhysicsWorld& world);
std::vector<HitPair> DetectCollisions(PhysicsWorld& world);
void ValidateResolutionVectors(std::vector<HitPair>& collisions);
void ResolveCollisions(std::vector<HitPair> collisions);
void ResolveVelocity(PhysicsBody& a, PhysicsBody& b, Vector2 mtv);
void ResolvePosition(PhysicsBody& a, PhysicsBody& b, Vector2 mtv);
void Update(PhysicsWorld& world);
void Draw(const PhysicsWorld& world);
void DrawForces(const PhysicsWorld& world);

int main()
{
    PhysicsWorld world;

    PhysicsBody* entity = nullptr;

    // Half-space ground
    world.entities.push_back({});
    entity = &world.entities.back();
    entity->position = { 400.0f, 600.0f };
    entity->gravity_scale = 0.0f;
    entity->collider_type = COLLIDER_TYPE_HALF_SPACE;
    entity->halfspace.normal = Vector2Rotate(Vector2UnitX, -30.0f * DEG2RAD); // angle of slope
    entity->inv_mass = 0.0f;

    // Red Sphere: 2 kg, friction 0.1
    world.entities.push_back({});
    entity = &world.entities.back();
    entity->position = { 300.0f, 400.0f };
    entity->gravity_scale = 1.0f;
    entity->collider_type = COLLIDER_TYPE_CIRCLE;
    entity->circle.radius = 20.0f;
    entity->inv_mass = 1.0f / 2.0f;
    entity->friction_coeff = 0.1f;

    // Green Sphere: 2 kg, friction 0.8
    world.entities.push_back({});
    entity = &world.entities.back();
    entity->position = { 350.0f, 400.0f };
    entity->gravity_scale = 1.0f;
    entity->collider_type = COLLIDER_TYPE_CIRCLE;
    entity->circle.radius = 20.0f;
    entity->inv_mass = 1.0f / 2.0f;
    entity->friction_coeff = 0.8f;

    // Blue Sphere: 8 kg, friction 0.1
    world.entities.push_back({});
    entity = &world.entities.back();
    entity->position = { 400.0f, 400.0f };
    entity->gravity_scale = 1.0f;
    entity->collider_type = COLLIDER_TYPE_CIRCLE;
    entity->circle.radius = 20.0f;
    entity->inv_mass = 1.0f / 8.0f;
    entity->friction_coeff = 0.1f;

    // Yellow Sphere: 8 kg, friction 0.8
    world.entities.push_back({});
    entity = &world.entities.back();
    entity->position = { 450.0f, 400.0f };
    entity->gravity_scale = 1.0f;
    entity->collider_type = COLLIDER_TYPE_CIRCLE;
    entity->circle.radius = 20.0f;
    entity->inv_mass = 1.0f / 8.0f;
    entity->friction_coeff = 0.8f;

    // Validate half-spaces
    for (const PhysicsBody& e : world.entities)
    {
        if (e.collider_type == COLLIDER_TYPE_HALF_SPACE)
            assert(IsMassInfinite(e));
    }

    bool is_first_frame = true;
    InitWindow(800, 800, "Physics-1");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if (!is_first_frame)
            Update(world); // motion + collisions + friction
        else
            is_first_frame = false;

        BeginDrawing();
        ClearBackground(WHITE);

        Draw(world);
        

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

// Motion loop
void UpdateMotion(PhysicsWorld& world)
{
    float dt = GetFrameTime();

    for (PhysicsBody& e : world.entities)
    {
        if (e.collider_type != COLLIDER_TYPE_CIRCLE) continue;

        Vector2 force_total = e.net_force + world.gravity * (1.0f / e.inv_mass) * e.gravity_scale;

        // Half-space collisions
        for (const PhysicsBody& half : world.entities)
        {
            if (half.collider_type != COLLIDER_TYPE_HALF_SPACE) continue;

            Vector2 mtv;
            if (CircleHalfSpace(e.position, e.circle.radius, half.position, half.halfspace.normal, &mtv))
            {
                float proj = Vector2DotProduct(force_total, half.halfspace.normal);
                Vector2 force_normal = half.halfspace.normal * proj;

                Vector2 tangent = { -half.halfspace.normal.y, half.halfspace.normal.x };
                tangent = Vector2Normalize(tangent);

                float vel_along_tangent = Vector2DotProduct(e.velocity, tangent);
                if (vel_along_tangent > 0) tangent *= -1.0f;

                Vector2 force_friction = tangent * (Vector2Length(force_normal) * e.friction_coeff);

                force_total -= force_friction;
                force_total -= force_normal;
            }
        }

        Vector2 acc = force_total * e.inv_mass;
        e.velocity += acc * dt;
        e.position += e.velocity * dt;

        e.net_force = Vector2Zeros;
        e.collision = false;
    }
}

// Collision detection
std::vector<HitPair> DetectCollisions(PhysicsWorld& world)
{
    std::vector<HitPair> collisions;

    for (size_t i = 0; i < world.entities.size(); i++)
    {
        for (size_t j = i + 1; j < world.entities.size(); j++)
        {
            PhysicsBody& a = world.entities[i];
            PhysicsBody& b = world.entities[j];
            assert(a.collider_type != COLLIDER_TYPE_INVALID && b.collider_type != COLLIDER_TYPE_INVALID);

            bool collision = false;
            Vector2 mtv = Vector2Zeros;

            if (a.collider_type == COLLIDER_TYPE_CIRCLE && b.collider_type == COLLIDER_TYPE_CIRCLE)
            {
                collision = CircleCircle(a.position, a.circle.radius, b.position, b.circle.radius, &mtv);
            }
            else if (a.collider_type == COLLIDER_TYPE_CIRCLE && b.collider_type == COLLIDER_TYPE_HALF_SPACE)
            {
                collision = CircleHalfSpace(a.position, a.circle.radius, b.position, b.halfspace.normal, &mtv);
            }
            else if (a.collider_type == COLLIDER_TYPE_HALF_SPACE && b.collider_type == COLLIDER_TYPE_CIRCLE)
            {
                collision = CircleHalfSpace(b.position, b.circle.radius, a.position, a.halfspace.normal, &mtv);
            }

            a.collision |= collision;
            b.collision |= collision;

            if (collision)
            {
                HitPair pair;
                pair.a = &a;
                pair.b = &b;
                pair.mtv = mtv;
                collisions.push_back(pair);
            }
        }
    }

    return collisions;
}

// Validation
void ValidateResolutionVectors(std::vector<HitPair>& collisions)
{
    for (HitPair& collision : collisions)
    {
        PhysicsBody*& a = collision.a;
        PhysicsBody*& b = collision.b;
        Vector2& mtv = collision.mtv;

        assert(!(IsMassInfinite(*a) && IsMassInfinite(*b)));

        if (IsMassInfinite(*a))
        {
            PhysicsBody* temp = b;
            b = a;
            a = temp;
        }

        Vector2 dir_BA = Vector2Normalize(a->position - b->position);
        if (Vector2DotProduct(dir_BA, mtv) < 0.0f)
            mtv *= -1.0f;
    }
}

// Resolve
void ResolveVelocity(PhysicsBody& a, PhysicsBody& b, Vector2 mtv)
{
    Vector2 normal = Vector2Normalize(mtv);
    float inv_mass_sum = a.inv_mass + b.inv_mass;
    assert(inv_mass_sum >= FLT_EPSILON);

    Vector2 vel_rel = a.velocity - b.velocity;
    float dot_vel_norm = Vector2DotProduct(vel_rel, normal);

    if (dot_vel_norm > 0.0f) return;

    float e = fminf(a.restitution_coeff, b.restitution_coeff);
    float j = (-(1.0f + e) * dot_vel_norm) / inv_mass_sum;
    Vector2 impulse = normal * j;

    a.velocity += impulse * a.inv_mass;
    b.velocity -= impulse * b.inv_mass;
}

void ResolvePosition(PhysicsBody& a, PhysicsBody& b, Vector2 mtv)
{
    if (IsMassInfinite(b))
        a.position += mtv;
    else
    {
        a.position += mtv * 0.5f;
        b.position -= mtv * 0.5f;
    }
}

void ResolveCollisions(std::vector<HitPair> collisions)
{
    for (HitPair collision : collisions)
    {
        ResolveVelocity(*collision.a, *collision.b, collision.mtv);
        ResolvePosition(*collision.a, *collision.b, collision.mtv);
    }
}

void Update(PhysicsWorld& world)
{
    UpdateMotion(world);
    auto collisions = DetectCollisions(world);
    ValidateResolutionVectors(collisions);
    ResolveCollisions(collisions);
}

// Draw
void Draw(const PhysicsWorld& world)
{
    for (const PhysicsBody& e : world.entities)
    {
        Color color = e.collision ? RED : GREEN;

        if (e.collider_type == COLLIDER_TYPE_CIRCLE)
            DrawCircleV(e.position, e.circle.radius, color);
        else if (e.collider_type == COLLIDER_TYPE_HALF_SPACE)
        {
            Vector2 direction = { -e.halfspace.normal.y, e.halfspace.normal.x };
            Vector2 p0 = e.position + direction * 1000.0f;
            Vector2 p1 = e.position - direction * 1000.0f;
            DrawLineEx(p0, p1, 5.0f, color);
            DrawLineEx(e.position, e.position + e.halfspace.normal * 50.0f, 5.0f, GOLD);
        }
    }
}

void DrawForces(const PhysicsWorld& world)
{
    for (const PhysicsBody& e : world.entities)
    {
        if (e.collider_type != COLLIDER_TYPE_CIRCLE) continue;

        Vector2 force_gravity = world.gravity * (1.0f / e.inv_mass) * e.gravity_scale;
        DrawLineEx(e.position, e.position + force_gravity, 4.0f, PURPLE);

        Vector2 force_normal = Vector2Zeros;
        Vector2 force_friction = Vector2Zeros;

        for (const PhysicsBody& half : world.entities)
        {
            if (half.collider_type != COLLIDER_TYPE_HALF_SPACE) continue;

            Vector2 mtv;
            if (CircleHalfSpace(e.position, e.circle.radius, half.position, half.halfspace.normal, &mtv))
            {
                float proj = Vector2DotProduct(force_gravity, half.halfspace.normal);
                force_normal = half.halfspace.normal * proj;

                Vector2 tangent = { -half.halfspace.normal.y, half.halfspace.normal.x };
                tangent = Vector2Normalize(tangent);

                float vel_along_tangent = Vector2DotProduct(e.velocity, tangent);
                if (vel_along_tangent > 0) tangent *= -1.0f;

                force_friction = tangent * (Vector2Length(force_normal) * e.friction_coeff);
            }
        }

        DrawLineEx(e.position, e.position + force_normal, 4.0f, GREEN);
        DrawLineEx(e.position, e.position + force_friction, 4.0f, ORANGE);
        DrawLineEx(e.position, e.position + e.velocity, 4.0f, RED);
    }
}

