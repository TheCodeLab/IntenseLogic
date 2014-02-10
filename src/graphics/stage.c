#include "stage.h"

ilG_stage ilG_stage_new(void *obj, const ilG_stagable *stagable)
{
    return (ilG_stage){
        .obj = obj,
        .stagable = stagable
    };
}

int ilG_stage_track(ilG_stage self, ilG_renderer renderer)
{
    return self.stagable->track(self.obj, renderer);
}

const char *ilG_stage_getName(ilG_stage self)
{
    return self.stagable->name;
}

