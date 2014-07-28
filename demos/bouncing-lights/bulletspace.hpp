#ifndef BULLETSPACE_H
#define BULLETSPACE_H

#include <vector>
#include <deque>
#include <memory>
#include <mutex>
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

extern "C" {
#include "math/matrix.h"
#include "graphics/context.h"
}

namespace BouncingLights {

struct BulletSpace {
    std::deque<btRigidBody> bodies;
    std::vector<btTransform> trans;
    std::vector<il_vec3> scale;
    std::vector<unsigned> freelist;

    static void free(void *ptr);
    static void viewmats(void *ptr, il_mat *out, int *types, unsigned num_objects);
    static void objmats(void *ptr, const unsigned *objects, unsigned num_objects, il_mat *out, int type);
    static bool build(void *ptr, unsigned id, ilG_context *context, ilG_coordsys *out);

    il_vec3 pos(unsigned id);
    il_quat rot(unsigned id);

public:
    class BodyId {
        BodyId(unsigned id) : id(id) {}
        unsigned id;
        friend BulletSpace;
    public:
        unsigned getId()
        {
            return id;
        }
    };
    BulletSpace(btPairCachingGhostObject &ghost, btDispatcher *dispatcher, btBroadphaseInterface *cache, btConstraintSolver *solver, btCollisionConfiguration *config);
    void build(ilG_context *context);
    BodyId addBody(const btRigidBody::btRigidBodyConstructionInfo &info);
    void delBody(BodyId id);
    void add(ilG_handle r, BodyId id);
    void del(ilG_handle r, BodyId id);
    int step(float by, int maxsubs = 1, float fixed = 1/60.f);
    btRigidBody &getBody(BodyId id)
    {
        return bodies[id.id];
    }
    void setBodyScale(BodyId id, il_vec3 v)
    {
        scale[id.id] = v;
    }

    btDiscreteDynamicsWorld world;
    btPairCachingGhostObject &ghost;
    il_mat projection;
    unsigned id;
    std::mutex mutex;
};

}

#endif
