#ifndef ILG_CONTEXT_INTERNAL_H
#define ILG_CONTEXT_INTERNAL_H

#include <pthread.h>

#include "graphics/transform.h"
#include "math/matrix.h"
#include "util/storage.h"

// TODO: Generalize this somehow
typedef struct ilG_context_msg {
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

typedef struct ilG_client_msg {
    enum ilG_client_msgtype {
        ILG_READY,
        ILG_FAILURE,
    } type;
    union {
        ilG_rendid ready;
        struct {
            ilG_rendid id;
            const char *msg;
        } failure;
    } v;
} ilG_client_msg;

typedef IL_ARRAY(ilG_context_msg, ilG_context_msgarray) ilG_context_msgarray;

typedef struct ilG_context_queue {
    ilG_context_msgarray read, write;
    pthread_mutex_t mutex;
} ilG_context_queue;

typedef IL_ARRAY(ilG_client_msg, ilG_client_msgarray) ilG_client_msgarray;

typedef struct ilG_client_queue {
    ilG_client_msgarray read, write;
    pthread_mutex_t mutex;
    void (*notify)(il_value *);
    il_value user;
} ilG_client_queue;

void ilG_context_queue_init(ilG_context_queue *queue);
void ilG_context_queue_free(ilG_context_queue *queue);
void ilG_context_queue_produce(ilG_context_queue *queue, ilG_context_msg msg);
void ilG_context_queue_read(ilG_context_queue *queue);

void ilG_client_queue_init(ilG_client_queue *queue);
void ilG_client_queue_free(ilG_client_queue *queue);
void ilG_client_queue_produce(ilG_client_queue *queue, ilG_client_msg msg);
void ilG_client_queue_read(ilG_client_queue *queue);

void ilG_default_update(void *, ilG_rendid);
void ilG_default_multiupdate(void *, ilG_rendid, il_mat *);
void ilG_default_draw(void *, ilG_rendid, il_mat **, const unsigned*, unsigned);
void ilG_default_viewmats(void*, il_mat*, int*, unsigned);
void ilG_default_objmats(void*, const unsigned*, unsigned, il_mat*, int);
void ilG_default_free(void*);

#endif
