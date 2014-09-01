#ifndef ILG_CONTEXT_INTERNAL_H
#define ILG_CONTEXT_INTERNAL_H

#include "graphics/transform.h"
#include "math/matrix.h"

// TODO: Generalize this somehow
typedef struct ilG_context_msg {
    struct ilG_context_msg *next;
    enum ilG_context_msgtype {
        ILG_UPLOAD,
        ILG_RESIZE,
        ILG_STOP,
        ILG_END,
        ILG_MESSAGE,
        ILG_ADD_COORDS,
        ILG_DEL_COORDS,
        ILG_VIEW_COORDS,
        ILG_ADD_RENDERER,
        ILG_DEL_RENDERER,
        ILG_ADD_LIGHT,
        ILG_DEL_LIGHT
    } type;
    union {
        struct {
            void (*cb)(void*);
            void *ptr;
        } upload;
        int resize[2];
        struct {
            ilG_rendid node;
            int type;
            il_value data;
        } message;
        struct {
            ilG_rendid parent, child;
        } renderer;
        struct {
            ilG_rendid parent;
            unsigned cosys, codata;
        } coords;
        struct {
            ilG_rendid parent;
            ilG_light child;
        } light;
    } value;
} ilG_context_msg;

typedef struct ilG_context_queue {
    struct ilG_context_msg *first;
    volatile struct ilG_context_msg *head, *tail;
} ilG_context_queue;

void ilG_context_queue_init(ilG_context_queue *queue);
void ilG_context_queue_free(ilG_context_queue *queue);
void ilG_context_queue_produce(ilG_context_queue *queue, ilG_context_msg *msg);
ilG_context_msg *ilG_context_queue_consume(ilG_context_queue *queue);
void ilG_context_queue_doneconsume(ilG_context_queue *queue);

void ilG_default_update(void *, ilG_rendid);
void ilG_default_multiupdate(void *, ilG_rendid, il_mat *);
void ilG_default_draw(void *, ilG_rendid, il_mat **, const unsigned*, unsigned);
void ilG_default_viewmats(void*, il_mat*, int*, unsigned);
void ilG_default_objmats(void*, const unsigned*, unsigned, il_mat*, int);
void ilG_default_free(void*);

#endif
