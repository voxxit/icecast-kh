/* Icecast
 *
 * This program is distributed under the GNU General Public License, version 2.
 * A copy of this license is included with this source.
 *
 * Copyright 2000-2004, Jack Moffitt <jack@xiph.org, 
 *                      Michael Smith <msmith@xiph.org>,
 *                      oddsock <oddsock@xiph.org>,
 *                      Karl Heyes <karl@xiph.org>
 *                      and others (see AUTHORS for details).
 */

/* refbuf.c
**
** reference counting buffer implementation
**
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "refbuf.h"

#define CATMODULE "refbuf"

#include "logging.h"
#include "global.h"


void refbuf_initialize(void)
{
}

void refbuf_shutdown(void)
{
}

#ifdef MY_ALLOC
refbuf_t *refbuf_new_s (unsigned int size, const char *file, const int line)
{
    refbuf_t *refbuf;

    refbuf = (refbuf_t *)my_calloc (file, line, 1, sizeof(refbuf_t));
    if (refbuf == NULL)
        abort();
    refbuf->data = NULL;
    if (size)
    {
        refbuf->data = my_calloc (file, line, 1, size);
        if (refbuf->data == NULL)
            abort();
    }
    refbuf->len = size;
    refbuf->_count = 1;
    refbuf->next = NULL;
    refbuf->associated = NULL;

    return refbuf;
}
#else
refbuf_t *refbuf_new (unsigned int size)
{
    refbuf_t *refbuf;

    refbuf = (refbuf_t *)calloc(1, sizeof(refbuf_t));
    if (refbuf == NULL)
        abort();
    refbuf->data = NULL;
    if (size)
    {
        refbuf->data = malloc (size);
        if (refbuf->data == NULL)
            abort();
    }
    refbuf->len = size;
    refbuf->_count = 1;
    refbuf->next = NULL;
    refbuf->associated = NULL;

    return refbuf;
}
#endif


void refbuf_addref(refbuf_t *self)
{
    if (self == NULL)
        return;
    self->_count++;
}


refbuf_t *refbuf_copy (refbuf_t *orig)
{
    if (orig == NULL) return NULL;
    refbuf_t *ret = refbuf_new (orig->len), *ref = ret;
    memcpy (ref->data, orig->data, orig->len);
    return ret;
}

refbuf_t *refbuf_copy_default (refbuf_t *orig)
{
    refbuf_t *ret = refbuf_copy (orig), *ref = ret;
    // assume the associated links are also refbuf
    orig = orig->associated;
    while (orig)
    {
        ref->associated = refbuf_copy (orig->associated);
        ref = ref->associated;
        orig = orig->associated;
    }
    return ret;
}


static void refbuf_release_associated (refbuf_t *ref)
{
    if (ref == NULL)
        return;
    while (ref)
    {
        refbuf_t *to_go = ref;
        ref = to_go->next;
        if (to_go->_count == 1)
            to_go->next = NULL;
        refbuf_release (to_go);
    }
}

void refbuf_release(refbuf_t *self)
{
    if (self == NULL)
        return;
    self->_count--;
    if (self->_count == 0)
    {
        refbuf_release_associated (self->associated);
        if (self->next)
            DEBUG0 ("next not null");
        free(self->data);
        free(self);
    }
}

