#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <GL/glew.h>

#include <iostream>
#include <utility>

#include "debugdraw.hpp"

using namespace std;
using namespace il::bouncinglights;

extern "C" {
#include <GL/glew.h>
#include "asset/image.h"
#include "common/world.h"
#include "common/world.h"
#include "graphics/camera.h"
#include "graphics/glutil.h"
#include "math/matrix.h"
#include "graphics/renderer.h"
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
static DebugDraw *debugdraw;

#ifdef WIN32
#define ex extern "C" __declspec(dllexport)
#else
#define ex extern "C"
#endif

static void debug_draw(void *ptr)
{
    (void)ptr;
    debugdraw->render();
}

static int debug_build(void *ptr, ilG_context *context)
{
    (void)ptr;
    debugdraw = new DebugDraw(context);
    dynamicsWorld->setDebugDrawer(debugdraw);
    return 1;
}

ex const ilG_renderable debug_renderer = {
    /*.free =*/ NULL,
    /*.draw =*/ debug_draw,
    /*.build =*/ debug_build,
    /*.get_storage =*/ NULL,
    /*.get_complete =*/ NULL,
    /*.add_positionable =*/ NULL,
    /*.add_renderer =*/ NULL,
    /*.name =*/ "Debug Draw"
};

ex void custom_data_func(struct ilG_material *self, il_positionable *pos, GLuint loc, void *user)
{
    (void)self; (void)user;
    const il_vector *col = il_table_getsa(il_positionable_getStorage(pos), "color");
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
    il_vec3 vec = il_positionable_getPosition(&cam->positionable);
    il_quat rot = il_positionable_getRotation(&cam->positionable);
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
    il_quat r = il_positionable_getRotation(pos);
    btQuaternion rot = btQuaternion(r.x, r.y, r.z, r.w);
    il_vec3 p = il_positionable_getPosition(pos);
    btVector3 vec = btVector3(p.x, p.y, p.z);
    btDefaultMotionState *state = new btDefaultMotionState(btTransform(rot, vec));
    float mass = 1.f;
    btVector3 inertia(0,0,0);
    ball_shape->calculateLocalInertia(mass, inertia);
    btRigidBody::btRigidBodyConstructionInfo ballRigidBodyCI(mass, state, ball_shape, inertia);
    btRigidBody* ballRigidBody = new btRigidBody(ballRigidBodyCI);
    ballRigidBody->setRestitution(1.0);
    dynamicsWorld->addRigidBody(ballRigidBody);
    il_storage_void sv;
    sv.data = ballRigidBody;
    sv.dtor = [](void *p) {delete (btRigidBody*)p;};
    il_table_setsp(il_positionable_mgetStorage(pos), "rigidbody", sv);
}

ex void update(int debug)
{
    player->setWalkDirection(playerWalk);
    dynamicsWorld->stepSimulation(1/20.f, 10, 1/60.f);
    il_positionable pos;
    unsigned i;
    for (pos = il_world_iter(world), i = 0; i < il_world_numPositionables(world); pos = il_world_next(&pos), i++) {
        btRigidBody *body = (btRigidBody*)il_table_mgetsp(il_positionable_mgetStorage(&pos), "rigidbody");
        if (!body) {
            continue;
        }
        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);
        btVector3 vec = trans.getOrigin();
        il_vec3 position;
        position.x = vec.getX();
        position.y = vec.getY();
        position.z = vec.getZ();
        //position.w = 1.0;
        il_positionable_setPosition(&pos, position);
        btQuaternion rot = trans.getRotation();
        il_quat rotation;
        rotation.x = rot.getX();
        rotation.y = rot.getY();
        rotation.z = rot.getZ();
        rotation.w = rot.getW();
        il_positionable_setRotation(&pos, rotation);
    }
    btTransform trans = ghostObject->getWorldTransform();
    btVector3 vec = trans.getOrigin();
    il_vec3 position;
    position.x = vec.x();
    position.y = vec.y();
    position.z = vec.z();
    il_positionable_setPosition(&camera->positionable, position);
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

