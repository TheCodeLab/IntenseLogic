#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <GL/glew.h>

#include <iostream>
#include <utility>

#include "debugdraw.hpp"

using namespace std;
using namespace il::bouncinglights;

extern "C" {
#include <GL/glew.h>
#include "asset/image.h"
#include "common/positionable.h"
#include "common/world.h"
#include "graphics/camera.h"
#include "graphics/glutil.h"
#include "math/matrix.h"
#include "graphics/stage.h"
}

static btBroadphaseInterface* broadphase;
static btDefaultCollisionConfiguration* collisionConfiguration;
static btCollisionDispatcher* dispatcher;
static btSequentialImpulseConstraintSolver* solver;
static btDiscreteDynamicsWorld* dynamicsWorld;
static btCollisionShape *ball_shape;
static il_world *world;
static ilG_camera *camera;
static btKinematicCharacterController *player;
static btPairCachingGhostObject *ghostObject;
static btSphereShape *playerShape;
static btVector3 playerWalk;
static ilG_stage debugstage;
static DebugDraw *debugdraw;

#ifdef WIN32
#define ex extern "C" __declspec(dllexport)
#else
#define ex extern "C"
#endif

ex void debug_draw(ilG_stage *self)
{
    (void)self;
    debugdraw->render();
}

ex ilG_stage *init_stage(ilG_context *context)
{
    debugdraw = new DebugDraw(context);
    dynamicsWorld->setDebugDrawer(debugdraw);

    debugstage.context = context;
    debugstage.run = debug_draw;
    debugstage.name = "Bullet Debug Draw";
    return &debugstage;
}

ex void custom_data_func(struct ilG_material *self, il_positionable *pos, GLuint loc, void *user)
{
    (void)self; (void)user;
    const il_vector *col = il_table_getsa(&pos->base.storage, "color");
    if (!col) {
        return;
    }
    glUniform4f(loc, il_vector_getf(col, 0),
                     il_vector_getf(col, 1),
                     il_vector_getf(col, 2),
                     il_vector_getf(col, 3));
}

ex void set_world(il_world *w)
{
    world = w;
}

ex void set_camera(ilG_camera *cam)
{
    camera = cam;
    playerShape = new btSphereShape(1);
    ghostObject = new btPairCachingGhostObject();
    ghostObject->setCollisionShape(playerShape);
    ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
    player = new btKinematicCharacterController(ghostObject, playerShape, .5, 2);
    player->setGravity(0);
    dynamicsWorld->addCollisionObject(ghostObject,btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::AllFilter);
    dynamicsWorld->addAction(player);
    playerWalk = btVector3(0,0,0);
    il_vec3 vec = cam->positionable.position;
    il_quat rot = cam->positionable.rotation;
    ghostObject->setWorldTransform(btTransform(btQuaternion(rot.x, rot.y, rot.z, rot.w), btVector3(vec.x, vec.y, vec.z)));
    player->warp(btVector3(vec.x, vec.y, vec.z));
}

ex void set_walk_direction(il_vec3 vec)
{
    playerWalk = btVector3(vec.x, vec.y, vec.z);
}

ex void add_heightmap(ilA_img *hm, float w, float h, float height)
{
    unsigned char *mem = new unsigned char[hm->width * hm->height];
    memcpy(mem, hm->data, hm->width*hm->height);
    btCollisionShape *heightmap_shape = new btHeightfieldTerrainShape(hm->width, hm->height, mem, height/255.f, 0, height, 1, PHY_UCHAR, false);
    heightmap_shape->setLocalScaling(btVector3(w/hm->width, 1, h/hm->height));
    btTransform trans = btTransform(btQuaternion(0,0,0,1), btVector3(w/2, height/2, h/2));
    btVector3 min, max, scaling;
    heightmap_shape->getAabb(trans, min, max);
    //printf("min(%f %f %f) max(%f %f %f)\n", min.x(), min.y(), min.z(), max.x(), max.y(), max.z());
    scaling = heightmap_shape->getLocalScaling();
    //printf("scale(%f %f %f)\n", scaling.x(), scaling.y(), scaling.z());
    btDefaultMotionState *heightmap_state = new btDefaultMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, heightmap_state, heightmap_shape, btVector3(0,0,0));
    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
    groundRigidBody->setRestitution(1.0);
    dynamicsWorld->addRigidBody(groundRigidBody);
}

ex void add_ball(il_positionable *pos)
{
    btQuaternion rot = btQuaternion(pos->rotation.x, pos->rotation.y, pos->rotation.z, pos->rotation.w);
    btVector3 vec = btVector3(pos->position.x, pos->position.y, pos->position.z);
    btDefaultMotionState *state = new btDefaultMotionState(btTransform(rot, vec));
    float mass = 1.f;
    btVector3 inertia(0,0,0);
    ball_shape->calculateLocalInertia(mass, inertia);
    btRigidBody::btRigidBodyConstructionInfo ballRigidBodyCI(mass, state, ball_shape, inertia);
    btRigidBody* ballRigidBody = new btRigidBody(ballRigidBodyCI);
    ballRigidBody->setRestitution(1.0);
    dynamicsWorld->addRigidBody(ballRigidBody);
    il_table_setsp(&pos->base.storage, "rigidbody", il_opaque(ballRigidBody, [](void *p) {delete (btRigidBody*)p;}));
}

ex void update(int debug)
{
    player->setWalkDirection(playerWalk);
    dynamicsWorld->stepSimulation(1/20.f, 10, 1/60.f);
    il_worldIterator *it = NULL;
    il_positionable *pos;
    for (pos = il_world_iterate(world, &it); pos; pos = il_world_iterate(world, &it)) {
        btRigidBody *body = (btRigidBody*)il_table_mgetsp(&pos->base.storage, "rigidbody");
        if (!body) {
            continue;
        }
        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);
        btVector3 vec = trans.getOrigin();
        pos->position.x = vec.getX();
        pos->position.y = vec.getY();
        pos->position.z = vec.getZ();
        btQuaternion rot = trans.getRotation();
        pos->rotation.x = rot.getX();
        pos->rotation.y = rot.getY();
        pos->rotation.z = rot.getZ();
        pos->rotation.w = rot.getW();
    }
    btTransform trans = ghostObject->getWorldTransform();
    btVector3 vec = trans.getOrigin();
    camera->positionable.position.x = vec.x();
    camera->positionable.position.y = vec.y();
    camera->positionable.position.z = vec.z();
    if (debug) {
        dynamicsWorld->debugDrawWorld();
        debugdraw->upload();
    }
}

ex int il_bootstrap(int argc, char **argv)
{
    (void)argc; (void)argv;
    broadphase = new btDbvtBroadphase();
    broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver;
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0,-10,0));
    ball_shape = new btSphereShape(.25);

    btScalar arenaWidth = 128;
    int i;
    btCollisionShape* groundShape[4];
    btDefaultMotionState *groundMotionState[4]; 
    btRigidBody* groundRigidBody[4];
    btVector3 norms[4] = {
        btVector3(1, 0, 0),
        btVector3(-1, 0, 0),
        btVector3(0, 0, 1),
        btVector3(0, 0, -1)
    };
    btVector3 positions[4] = {
        btVector3(0, 0, 0),
        btVector3(arenaWidth, 0, 0),
        btVector3(0, 0, 0),
        btVector3(0, 0, arenaWidth)
    };
    for (i = 0; i < 4; i++) {
        groundShape[i] = new btStaticPlaneShape(norms[i],1);
        groundMotionState[i] = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),positions[i]));
        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0,groundMotionState[i],groundShape[i],btVector3(0,0,0));
        groundRigidBody[i] = new btRigidBody(groundRigidBodyCI);
        groundRigidBody[i]->setRestitution(0.5);
        dynamicsWorld->addRigidBody(groundRigidBody[i]);
    }

    return 0;
}

