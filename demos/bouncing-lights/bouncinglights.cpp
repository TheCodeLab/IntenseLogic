#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <SDL.h>

#include <iostream>
#include <utility>
#include <ctime>
#include <math.h>

#include "tgl/tgl.h"
#include "debugdraw.hpp"
#include "bulletspace.hpp"
#include "ball.hpp"

using namespace std;
using namespace BouncingLights;

extern "C" {
#include "asset/image.h"
#include "graphics/transform.h"
#include "math/matrix.h"
#include "graphics/context-internal.h"
#include "graphics/graphics.h"
#include "graphics/renderer.h"
#include "graphics/tex.h"
#include "util/log.h"
#include "helper.h"
}

#ifndef M_PI
#define M_PI 3.1415926535
#endif

#ifdef WIN32
#define ex extern "C" __declspec(dllexport)
#else
#define ex extern "C"
#endif

const btScalar arenaWidth = 128;

extern ilA_fs demo_fs;

float rand_float(unsigned *seedp)
{
    return (float)rand_r(seedp) / RAND_MAX;
}

void add_objects(BulletSpace &bs, BallRenderer &ball, ilG_handle lights, ilG_handle r, btCollisionShape *shape, unsigned num, unsigned *seedp)
{
    for (unsigned i = 0; i < num; i++) {
        btVector3 vec
            (rand_r(seedp) % 128,
             rand_r(seedp) % 32 + 50,
             rand_r(seedp) % 128);
        float brightness = rand_float(seedp) + 1;
        il_vec3 col = il_vec3_new
            (rand_float(seedp) * brightness,
             rand_float(seedp) * brightness,
             rand_float(seedp) * brightness);
        auto state = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), vec));
        float mass = 1.f;
        btVector3 inertia(0,0,0);
        shape->calculateLocalInertia(mass, inertia);
        btRigidBody::btRigidBodyConstructionInfo ballRigidBodyCI(mass, state, shape, inertia);
        auto id = bs.addBody(ballRigidBodyCI);
        bs.getBody(id).setRestitution(1.0);
        bs.add(r, id);
        bs.add(lights, id);
        ilG_light l;
        l.color = col;
        l.radius = brightness * 10;
        ilG_handle_addLight(lights, l);
        ball.add_col(id.getId(), col);
    }
}

struct gtick_ctx {
    gtick_ctx(BulletSpace &bs, ilG_context *context) : bs(bs), context(context)
    {}
    BulletSpace &bs;
    ilG_context *context;
};

static void gtick(const il_value *data, il_value *ctx)
{
    (void)data;
    gtick_ctx &c = *reinterpret_cast<gtick_ctx*>(il_value_tomvoid(ctx));
    c.bs.projection = il_mat_perspective(45, c.context->width / (float)c.context->height, .5, 1000);
}

#include <fenv.h>
ex void demo_start()
{
    ilG_shaders_addPath("demos/bouncing-lights/");
    ilA_adddir(&demo_fs, "demos/bouncing-lights/", -1);
    //feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
    // Create context
    helper_config c;
    memset(&c, 0, sizeof(c));
    c.sky = true;
    c.geom = true;
    c.lights = true;
    c.out = true;
    c.name = "Bouncing Lights";
    const char *sky[6] = {
        "demos/bouncing-lights/north.png",
        "demos/bouncing-lights/south.png",
        "demos/bouncing-lights/up.png",
        "demos/bouncing-lights/down.png",
        "demos/bouncing-lights/west.png",
        "demos/bouncing-lights/east.png"
    };
    memcpy(c.skytex, sky, sizeof(sky));
    auto h = helper_create(&c);
    *h.ambient_col = il_vec3_new(0.25, 0.25, 0.25);
    ilG_context *context = h.context;
    ilG_context_hint(context, ILG_CONTEXT_HDR, 1);
    //ilG_context_hint(context, ILG_CONTEXT_MSAA, 4);

    // Create character controller
    btSphereShape playerShape(1);
    btPairCachingGhostObject ghostObject;
    ghostObject.setWorldTransform(btTransform(btQuaternion(0,0,0,1), btVector3(64, 50, 64)));
    ghostObject.setCollisionShape(&playerShape);
    ghostObject.setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
    btKinematicCharacterController player(&ghostObject, &playerShape, .5, 2);
    player.setGravity(0);

    // Create world
    btDbvtBroadphase broadphase;
    btGhostPairCallback cb;
    broadphase.getOverlappingPairCache()->setInternalGhostPairCallback(&cb);
    btDefaultCollisionConfiguration collisionConfiguration;
    btCollisionDispatcher dispatcher(&collisionConfiguration);
    btSequentialImpulseConstraintSolver solver;
    BulletSpace world(ghostObject, &dispatcher, &broadphase, &solver, &collisionConfiguration);
    world.build(context);
    ilG_handle_setViewCoords(h.sky, world.id);
    world.world.setGravity(btVector3(0,-10,0));
    DebugDraw debugdraw;
    world.world.setDebugDrawer(&debugdraw);
    world.world.addCollisionObject(&ghostObject,
                                   btBroadphaseProxy::CharacterFilter,
                                   btBroadphaseProxy::StaticFilter|btBroadphaseProxy::AllFilter);
    world.world.addAction(&player);

    // setup invisible arena walls
    int i;
    btStaticPlaneShape groundShape[4] = {
        btStaticPlaneShape(btVector3( 1, 0,  0), 1),
        btStaticPlaneShape(btVector3(-1, 0,  0), 1),
        btStaticPlaneShape(btVector3( 0, 0,  1), 1),
        btStaticPlaneShape(btVector3( 0, 0, -1), 1)
    };
    btDefaultMotionState groundMotionState[4];
    char ground_backing[4*sizeof(btRigidBody)];
    btVector3 positions[4] = {
        btVector3(0, 0, 0),
        btVector3(arenaWidth, 0, 0),
        btVector3(0, 0, 0),
        btVector3(0, 0, arenaWidth)
    };
    for (i = 0; i < 4; i++) {
        groundMotionState[i] = btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),positions[i]));
        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI
            (0,
             &groundMotionState[i],
             &groundShape[i],
             btVector3(0,0,0) );
        char *ptr = ground_backing + sizeof(btRigidBody)*i;
        btRigidBody &groundRigidBody = *new(ptr) btRigidBody(groundRigidBodyCI);
        groundRigidBody.setRestitution(0.5);
        world.world.addRigidBody(&groundRigidBody);
    }

    // Create heightmap physics and render stuff
    ilA_img hm;
    ilA_imgerr res = ilA_img_loadfile(&hm, &demo_fs, "arena-heightmap.png");
    if (res) {
        il_error("Failed to load heightmap: %s", ilA_img_strerror(res));
        return;
    }
    const unsigned height = 50;
    btHeightfieldTerrainShape heightmap_shape
        (hm.width, hm.height, hm.data, height/255.f, 0, height, 1, PHY_UCHAR, false);
    heightmap_shape.setLocalScaling(btVector3(arenaWidth/hm.width, 1, arenaWidth/hm.height));
    btTransform trans = btTransform(btQuaternion(0,0,0,1),
                                    btVector3(arenaWidth/2, height/2, arenaWidth/2));
    /*btVector3 min, max, scaling;
      heightmap_shape.getAabb(trans, min, max);
      printf("min(%f %f %f) max(%f %f %f)\n", min.x(), min.y(), min.z(), max.x(), max.y(), max.z());
      scaling = heightmap_shape.getLocalScaling();
      printf("scale(%f %f %f)\n", scaling.x(), scaling.y(), scaling.z());*/
    btDefaultMotionState heightmap_state(trans);
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI
        (0, &heightmap_state, &heightmap_shape, btVector3(0,0,0));
    auto groundId = world.addBody(groundRigidBodyCI);
    world.getBody(groundId).setRestitution(1.0);
    world.setBodyScale(groundId, il_vec3_new(128, 50, 128));
    ilA_img norm;
    res = ilA_img_height_to_normal(&norm, &hm);
    if (res) {
        il_error("Failed to create normal map: %s", ilA_img_strerror(res));
        return;
    }
    ilG_tex colortex, heighttex, normaltex;
    res = ilG_tex_loadfile(&colortex, &demo_fs, "terrain.png");
    if (res) {
        il_error("Failed to load heightmap texture: %s", ilA_img_strerror(res));
        return;
    }
    ilA_img hmc;
    res = ilA_img_copy(&hmc, &hm);
    if (res) {
        il_error("Failed to copy image: %s", ilA_img_strerror(res));
        return;
    }
    ilG_tex_loadimage(&heighttex, hmc);
    ilG_tex_loadimage(&normaltex, norm);
    ilG_handle hmr = ilG_build
        (ilG_heightmap_builder(hm.width, hm.height,
                               heighttex, normaltex, colortex), &context->manager);
    ilG_handle_addRenderer(h.geom, hmr);
    world.add(hmr, groundId);

    // Create ball renderer and common bullet stuff
    btSphereShape ball_shape(1);
    BallRenderer ball;
    ilG_handle ball_r = ilG_build(ball.builder(), &context->manager);
    ilG_handle_addRenderer(h.geom, ball_r);
    unsigned seed = time(NULL);
    add_objects(world, ball, h.lights, ball_r, &ball_shape, 100, &seed);

    gtick_ctx ctx(world, context);
    il_storage_void sv;
    sv.data = &ctx;
    sv.dtor = NULL;
    ilE_register(&context->tick, ILE_DONTCARE, ILE_ANY, gtick, il_value_opaque(sv));

    ilG_context_start(context);

    float yaw = 0, pitch = 0;
    il_quat rot = il_quat_new(0,0,0,1);
    SDL_Event ev;
    while (1) {
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                il_log("Stopping");
                ilG_context_end(context);
                return;
            case SDL_MOUSEMOTION:
                if (ev.motion.state & SDL_BUTTON_LMASK) {
                    const float s = 0.01;
                    yaw = fmodf(yaw + ev.motion.xrel * s, M_PI * 2);
                    pitch = max((float)-M_PI/2, min((float)M_PI/2, pitch + ev.motion.yrel * s));
                    rot = il_quat_mul
                        (il_quat_fromAxisAngle(0,1,0, -yaw),
                         il_quat_fromAxisAngle(1,0,0, -pitch));
                    ghostObject.setWorldTransform(btTransform(btQuaternion(rot.x,rot.y,rot.z,rot.w),
                                                              ghostObject.getWorldTransform().getOrigin()));
                }
                break;
            }
        }
        ilG_client_queue_read(&context->manager.client);
        for (unsigned i = 0; i < context->manager.client.read.length; i++) {
            ilG_client_msg msg = context->manager.client.read.data[i];
            if (msg.type == ilG_client_msg::ILG_FAILURE) {
                il_log("Renderer %u failed: %s", msg.v.failure.id, msg.v.failure.msg);
            }
        }
        {
            const uint8_t *state = SDL_GetKeyboardState(NULL);
            il_vec3 playerwalk = il_vec3_new
                (state[SDL_SCANCODE_D] - state[SDL_SCANCODE_A],
                 state[SDL_SCANCODE_R] - state[SDL_SCANCODE_F],
                 state[SDL_SCANCODE_S] - state[SDL_SCANCODE_W]);
            const float speed = 10 / 60.f; // speed/s over ticks/s
            playerwalk.x *= speed;
            playerwalk.y *= speed;
            playerwalk.z *= speed;
            playerwalk = il_vec3_rotate(playerwalk, rot);
            player.setWalkDirection(btVector3(playerwalk.x, playerwalk.y, playerwalk.z));
        }
        world.step(1/60.f);
        struct timespec ts = {0, 1000000000 / 60};
        nanosleep(&ts, &ts);
    }
}
