/*
 * repos.c: mod_dav_svn repository provider functions for Subversion
 *
 * ================================================================
 * Copyright (c) 2000 CollabNet.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. The end-user documentation included with the redistribution, if
 * any, must include the following acknowlegement: "This product includes
 * software developed by CollabNet (http://www.Collab.Net)."
 * Alternately, this acknowlegement may appear in the software itself, if
 * and wherever such third-party acknowlegements normally appear.
 * 
 * 4. The hosted project names must not be used to endorse or promote
 * products derived from this software without prior written
 * permission. For written permission, please contact info@collab.net.
 * 
 * 5. Products derived from this software may not use the "Tigris" name
 * nor may "Tigris" appear in their names without prior written
 * permission of CollabNet.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL COLLABNET OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 * 
 * This software consists of voluntary contributions made by many
 * individuals on behalf of CollabNet.
 *
 * Portions of this software were originally written by Greg Stein as a
 * sourceXchange project sponsored by SapphireCreek.
 */



#include <httpd.h>
#include <http_protocol.h>
#include <http_log.h>
#include <mod_dav.h>

#include <apr_strings.h>

#include "svn_types.h"
#include "dav_svn.h"


struct dav_resource_private {
  apr_pool_t *pool;

  /* Path from the SVN repository root to this resource. */
  const char *path;
};

struct dav_stream {
  const dav_resource *res;
  int pos;
};

typedef struct {
  dav_resource res;
  dav_resource_private priv;
} dav_resource_combined;


static dav_resource * dav_svn_get_resource(request_rec *r,
                                           const char *root_dir,
                                           const char *workspace,
                                           const char *target,
                                           int is_label)
{
  dav_resource_combined *comb;
  apr_size_t len;
  char *uri;
  const char *relative;

  comb = apr_pcalloc(r->pool, sizeof(*comb));
  comb->res.info = &comb->priv;
  comb->res.hooks = &dav_svn_hooks_repos;
  comb->priv.pool = r->pool;

  comb->res.type = DAV_RESOURCE_TYPE_REGULAR;
  comb->res.exists = TRUE;

  /* make a copy so that we can do some work on it */
  uri = apr_pstrdup(r->pool, r->uri);

  /* remove duplicate slashes */
  ap_no2slash(uri);

  /* make sure the URI does not have a trailing "/" */
  len = strlen(uri);
  if (len > 1 && uri[len - 1] == '/')
    uri[len - 1] = '\0';

  comb->res.uri = uri;

  /* The URL space defined by the SVN provider is always a virtual
     space. Construct the path relative to the configured Location
     (root_dir). So... the relative location is simply the URL used,
     skipping the root_dir. */
  relative = ap_stripprefix(uri, root_dir);

  /* It is possible that some yin-yang used a trailing slash in their
     Location directive (which was then removed as part of the
     "prefix".  Back up a step if we don't have a leading slash. */
  if (*relative != '/')
      --relative;

  /* "relative" is part of the "uri" string, so it has the proper
     lifetime to store here. */
  comb->priv.path = relative;

#if 0
  DBG1("uri: %s", uri);
  DBG2("root_dir=\"%s\"  path=\"%s\"", root_dir, relative);
#endif

  return &comb->res;
}

static dav_resource * dav_svn_get_parent_resource(const dav_resource *resource)
{
  /* ### fill this in */
  return NULL;
}

static int dav_svn_is_same_resource(const dav_resource *res1,
                                    const dav_resource *res2)
{
  /* ### fill this in */
  return 1;
}

static int dav_svn_is_parent_resource(const dav_resource *res1,
                                      const dav_resource *res2)
{
  /* ### fill this in */
  return 1;
}

static dav_error * dav_svn_open_stream(const dav_resource *resource,
                                       dav_stream_mode mode,
                                       dav_stream **stream)
{
  *stream = apr_pcalloc(resource->info->pool, sizeof(*stream));

  (*stream)->res = resource;

  return NULL;
}

static dav_error * dav_svn_close_stream(dav_stream *stream, int commit)
{
  /* ### fill this in */
  return NULL;
}

static dav_error * dav_svn_read_stream(dav_stream *stream, void *buf,
                                       apr_size_t *bufsize)
{
  if (stream->pos) {
    /* EOF */
    *bufsize = 0;
    return NULL;
  }

  if (*bufsize > 10)
    *bufsize = 10;
  memcpy(buf, "123456789\n", *bufsize);
  stream->pos = 1;

  return NULL;
}

static dav_error * dav_svn_write_stream(dav_stream *stream, const void *buf,
                                        apr_size_t bufsize)
{
  /* ### fill this in */
  return NULL;
}

static dav_error * dav_svn_seek_stream(dav_stream *stream,
                                       apr_off_t abs_position)
{
  /* ### fill this in */
  return NULL;
}

static const char * dav_svn_getetag(const dav_resource *resource)
{
  /* ### fix this */
  return "svn-etag";
}

static dav_error * dav_svn_set_headers(request_rec *r,
                                       const dav_resource *resource)
{
  if (!resource->exists)
    return NULL;

  /* make sure the proper mtime is in the request record */
#if 0
  ap_update_mtime(r, resource->info->finfo.mtime);
#endif

  /* ### note that these use r->filename rather than <resource> */
#if 0
  ap_set_last_modified(r);
#endif

  /* generate our etag and place it into the output */
  apr_table_set(r->headers_out, "ETag", dav_svn_getetag(resource));

  /* we accept byte-ranges */
  apr_table_setn(r->headers_out, "Accept-Ranges", "bytes");

  /* set up the Content-Length header */
#if 0
  ap_set_content_length(r, resource->info->finfo.size);
#endif

  /* ### how to set the content type? */
  /* ### until this is resolved, the Content-Type header is busted */

  return NULL;
}

static dav_error * dav_svn_create_collection(dav_resource *resource)
{
  /* ### fill this in */
  return NULL;
}

static dav_error * dav_svn_copy_resource(const dav_resource *src,
                                         dav_resource *dst,
                                         int depth,
                                         dav_response **response)
{
  /* ### fill this in */
  return NULL;
}

static dav_error * dav_svn_move_resource(dav_resource *src,
                                         dav_resource *dst,
                                         dav_response **response)
{
  /* ### fill this in */
  return NULL;
}

static dav_error * dav_svn_remove_resource(dav_resource *resource,
                                           dav_response **response)
{
  /* ### fill this in */
  return NULL;
}

static dav_error * dav_svn_walk(dav_walker_ctx *wctx, int depth)
{
  /* ### fill this in */
  return NULL;
}


const dav_hooks_repository dav_svn_hooks_repos =
{
  1,                            /* special GET handling */
  dav_svn_get_resource,
  dav_svn_get_parent_resource,
  dav_svn_is_same_resource,
  dav_svn_is_parent_resource,
  dav_svn_open_stream,
  dav_svn_close_stream,
  dav_svn_read_stream,
  dav_svn_write_stream,
  dav_svn_seek_stream,
  dav_svn_set_headers,
  NULL,                         /* get_pathname */
  NULL,                         /* free_file */
  dav_svn_create_collection,
  dav_svn_copy_resource,
  dav_svn_move_resource,
  dav_svn_remove_resource,
  dav_svn_walk,
  dav_svn_getetag,
};


/* 
 * local variables:
 * eval: (load-file "../svn-dev.el")
 * end:
 */
