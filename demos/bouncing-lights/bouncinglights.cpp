#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

extern "C" {
#include "asset/image.h"
#include "common/positionable.h"
#include "common/world.h"
}

static btBroadphaseInterface* broadphase;
static btDefaultCollisionConfiguration* collisionConfiguration;
static btCollisionDispatcher* dispatcher;
static btSequentialImpulseConstraintSolver* solver;
static btDiscreteDynamicsWorld* dynamicsWorld;
static btCollisionShape *ball_shape;
static il_world *world;

extern "C" void set_world(il_world *w)
{
    world = w;
}

extern "C" void add_heightmap(ilA_img *hm, float height)
{
    unsigned char *mem = new unsigned char[hm->width * hm->height];
    memcpy(mem, hm->data, hm->width*hm->height);
    btCollisionShape *heightmap_shape = new btHeightfieldTerrainShape(hm->width+1, hm->height+1, mem, height/255.f, 0, height, 1, PHY_UCHAR, false);
    btTransform trans = btTransform(btQuaternion(0,0,0,1), btVector3(hm->width/2, height/2, hm->height/2));
    btVector3 min, max, scaling;
    heightmap_shape->getAabb(trans, min, max);
    scaling = heightmap_shape->getLocalScaling();
    printf("Heightmap AABB: (%f %f %f) (%f %f %f)\nScaling: (%f %f %f)\n", min.x(), min.y(), min.z(), max.x(), max.y(), max.z(), scaling.x(), scaling.y(), scaling.z());
    btDefaultMotionState *heightmap_state = new btDefaultMotionState(trans);
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, heightmap_state, heightmap_shape, btVector3(0,0,0));
    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
    groundRigidBody->setRestitution(1.0);
    dynamicsWorld->addRigidBody(groundRigidBody);
}

extern "C" void add_ball(il_positionable *pos)
{
    btQuaternion rot = btQuaternion(pos->rotation[0], pos->rotation[1], pos->rotation[2], pos->rotation[3]);
    btVector3 vec = btVector3(pos->position[0], pos->position[1], pos->position[2]);
    btDefaultMotionState *state = new btDefaultMotionState(btTransform(rot, vec));
    float mass = 1.f;
    btVector3 inertia(0,0,0);
    ball_shape->calculateLocalInertia(mass, inertia);
    btRigidBody::btRigidBodyConstructionInfo ballRigidBodyCI(mass, state, ball_shape, inertia);
    btRigidBody* ballRigidBody = new btRigidBody(ballRigidBodyCI);
    ballRigidBody->setRestitution(1.0);
    dynamicsWorld->addRigidBody(ballRigidBody);
    il_base_set(pos, "rigidbody", ballRigidBody, sizeof(btRigidBody), IL_VOID);
}

extern "C" void update()
{
    //printf("physics step\n");
    dynamicsWorld->stepSimulation(1/20.f, 10, 1/200.f);
    il_worldIterator *it = NULL;
    il_positionable *pos;
    for (pos = il_world_iterate(world, &it); pos; pos = il_world_iterate(world, &it)) {
        btRigidBody *body = (btRigidBody*)il_base_get(pos, "rigidbody", NULL, NULL);
        if (!body) {
            continue;
        }
        //printf("update %p\n", body);
        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);
        btVector3 vec = trans.getOrigin();
        pos->position[0] = vec.getX();
        pos->position[1] = vec.getY();
        pos->position[2] = vec.getZ();
        btQuaternion rot = trans.getRotation();
        pos->rotation[0] = rot.getX();
        pos->rotation[1] = rot.getY();
        pos->rotation[2] = rot.getZ();
        pos->rotation[3] = rot.getW();
    }
}

extern "C" int il_bootstrap(int argc, char **argv)
{
    (void)argc; (void)argv;
    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver;
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0,-10,0));
    ball_shape = new btSphereShape(.25);

    btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0),1);
    btDefaultMotionState *groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,-50,0)));
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0,groundMotionState,groundShape,btVector3(0,0,0));
    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
    groundRigidBody->setRestitution(1.0);
    dynamicsWorld->addRigidBody(groundRigidBody);

    return 0;
}

