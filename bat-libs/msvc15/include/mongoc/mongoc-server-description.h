/*
 * Copyright 2014 MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MONGOC_SERVER_DESCRIPTION_H
#define MONGOC_SERVER_DESCRIPTION_H

#include <bson.h>

#include "mongoc-read-prefs.h"
#include "mongoc-host-list.h"

typedef struct _mongoc_server_description_t mongoc_server_description_t;

BSON_EXPORT (void)
mongoc_server_description_destroy (mongoc_server_description_t *description);

BSON_EXPORT (mongoc_server_description_t *)
mongoc_server_description_new_copy (
   const mongoc_server_description_t *description);

BSON_EXPORT (uint32_t)
mongoc_server_description_id (const mongoc_server_description_t *description);

BSON_EXPORT (mongoc_host_list_t *)
mongoc_server_description_host (const mongoc_server_description_t *description);

BSON_EXPORT (int64_t)
mongoc_server_description_round_trip_time (
   const mongoc_server_description_t *description);

BSON_EXPORT (const char *)
mongoc_server_description_type (const mongoc_server_description_t *description);

BSON_EXPORT (const bson_t *)
mongoc_server_description_ismaster (
   const mongoc_server_description_t *description);

#endif
