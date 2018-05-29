/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/**
* @file sai_qos_util.c
*
* @brief This file contains the util functions for SAI Qos component.
*
*************************************************************************/

#include "sai_qos_common.h"
#include "sai_qos_util.h"
#include "std_assert.h"
#include "sai_port_common.h"
#include "sai_port_utils.h"
#include "std_assert.h"

#include "saitypes.h"
#include "saistatus.h"

#include "std_mutex_lock.h"
#include "std_type_defs.h"
#include "std_assert.h"
#include "std_llist.h"
#include "std_struct_utils.h"

#include <string.h>
#include <inttypes.h>
#include <stdio.h>

/** Offset of the policer dll glue in acl rule data structure */
#define SAI_QOS_POLICER_ACL_DLL_GLUE_OFFSET \
    STD_STR_OFFSET_OF(sai_acl_rule_t, \
                       policer_glue)

/** Offset of the policer dll glue in port data structure */
#define SAI_QOS_POLICER_PORT_DLL_GLUE_OFFSET \
    STD_STR_OFFSET_OF(dn_sai_qos_port_t, \
                      policer_dll_glue)
/** Offset of the PORT DLL glue field in Queue data structure */
#define SAI_QOS_QUEUE_PORT_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF (dn_sai_qos_queue_t, port_dll_glue)

/** Offset of the child queue DLL glue field in Queue data structure */
#define SAI_QOS_QUEUE_CHILD_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF (dn_sai_qos_queue_t, child_queue_dll_glue)

/** Offset of the WRED DLL glue field in Queue data structure */
#define SAI_QOS_QUEUE_WRED_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF (dn_sai_qos_queue_t, wred_dll_glue)

/** Offset of the PORT DLL glue field in Scheduler group data structure */
#define SAI_QOS_SCHED_GROUP_PORT_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF (dn_sai_qos_sched_group_t, port_dll_glue)

/** Offset of the Child scheduler group DLL glue field in
 * Scheduler group data structure */
#define SAI_QOS_SCHED_GROUP_CHILD_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF (dn_sai_qos_sched_group_t, \
                                child_sched_group_dll_glue)

/** Offset of the maps dll glue in the qos port data structure */
#define SAI_QOS_MAPS_PORT_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF(dn_sai_qos_port_t, \
                                    maps_dll_glue)

/** Offset of the Scheduler DLL glue field in Queue data structure */
#define SAI_QOS_QUEUE_SCHEDULER_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF (dn_sai_qos_queue_t, scheduler_dll_glue)

/** Offset of the Scheduler DLL glue field in Scheduler group data structure */
#define SAI_QOS_SCHED_GROUP_SCHEDULER_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF (dn_sai_qos_sched_group_t, scheduler_dll_glue)

/** Offset of the Scheduler DLL glue field in port data structure */
#define SAI_QOS_PORT_SCHEDULER_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF (dn_sai_qos_port_t, scheduler_dll_glue)

/** Offset of the queue dll glue in the qos queue data structure */
#define SAI_QOS_WRED_QUEUE_DLL_GLUE_OFFSET \
             STD_STR_OFFSET_OF(dn_sai_qos_queue_t, \
                                    wred_dll_glue)

/* Simple Mutex lock for accessing QOS resources */
static std_mutex_lock_create_static_init_fast (g_sai_qos_lock);

static dn_sai_qos_global_t g_sai_qos_config = {
    is_init_complete: false,
};

static dn_sai_qos_hierarchy_t default_hqos;
static dn_sai_qos_hierarchy_t default_cpu_hqos;
static sai_object_id_t default_sched_id;

dn_sai_qos_global_t *sai_qos_access_global_config (void)
{
    return (&g_sai_qos_config);
}

dn_sai_qos_hierarchy_t *sai_qos_default_hqos_get (void)
{
    return (&default_hqos);
}

dn_sai_qos_hierarchy_t *sai_qos_default_cpu_hqos_get (void)
{
    return (&default_cpu_hqos);
}

sai_object_id_t sai_qos_default_sched_id_get (void)
{
    return (default_sched_id);
}

void sai_qos_default_sched_id_set(sai_object_id_t sched_id)
{
    default_sched_id = sched_id;
}

void sai_qos_lock (void)
{
    std_mutex_lock (&g_sai_qos_lock);
}

void sai_qos_unlock (void)
{
    std_mutex_unlock (&g_sai_qos_lock);
}

dn_sai_qos_port_t* sai_qos_port_node_get (sai_object_id_t port_id)
{
    sai_port_application_info_t  *p_port_node = NULL;

    p_port_node = sai_port_application_info_get (port_id);

    if (NULL == p_port_node) {
        SAI_QOS_LOG_ERR ("Port Info is NULL for port 0x%"PRIx64"", port_id);
        return NULL;
    }

    return ((dn_sai_qos_port_t *) p_port_node->qos_port_db);
}

dn_sai_qos_port_t *sai_qos_port_node_get_first (void)
{
   sai_port_application_info_t *p_port_node = NULL;

   for (p_port_node = sai_port_first_application_node_get();
        p_port_node != NULL;
        p_port_node = sai_port_next_application_node_get(p_port_node)) {

        if (NULL != p_port_node->qos_port_db)
           break;
   }

   if (NULL == p_port_node)
       return NULL;

   return ((dn_sai_qos_port_t *) p_port_node->qos_port_db);
}

dn_sai_qos_port_t *sai_qos_port_node_get_next (dn_sai_qos_port_t *p_qos_port_node)
{
   sai_port_application_info_t *p_prev_port_node = NULL;
   sai_port_application_info_t *p_port_node = NULL;

   if (NULL == p_qos_port_node)
       return NULL;

   p_prev_port_node = sai_port_application_info_get (p_qos_port_node->port_id);
   if (NULL == p_prev_port_node)
       return NULL;

   for (p_port_node = sai_port_next_application_node_get(p_prev_port_node);
        p_port_node != NULL;
        p_port_node = sai_port_next_application_node_get(p_port_node)) {

       if (NULL != p_port_node->qos_port_db)
           break;
   }

    if (NULL == p_port_node)
       return NULL;

   return ((dn_sai_qos_port_t *) p_port_node->qos_port_db);
}

dn_sai_qos_queue_t *sai_qos_port_get_first_queue (
                                    dn_sai_qos_port_t *p_qos_port_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
                   sai_qos_dll_get_first (&p_qos_port_node->queue_dll_head)))) {
        return ((dn_sai_qos_queue_t *) (p_temp -
                                        SAI_QOS_QUEUE_PORT_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

dn_sai_qos_queue_t *sai_qos_port_get_next_queue (
                                            dn_sai_qos_port_t *p_qos_port_node,
                                            dn_sai_qos_queue_t *p_queue_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next (&p_qos_port_node->queue_dll_head,
                                                     &p_queue_node->port_dll_glue)))) {
        return ((dn_sai_qos_queue_t *) (p_temp -
                                        SAI_QOS_QUEUE_PORT_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

dn_sai_qos_queue_t *sai_qos_queue_node_get (sai_object_id_t queue_id)
{
    rbtree_handle           queue_tree;
    dn_sai_qos_queue_t      queue_node;

    memset (&queue_node, 0, sizeof (dn_sai_qos_queue_t));

    queue_node.key.queue_id = queue_id;

    queue_tree = sai_qos_access_global_config()->queue_tree;

    if (NULL == queue_tree)
        return NULL;

    return ((dn_sai_qos_queue_t *) std_rbtree_getexact (queue_tree, &queue_node));
}

sai_queue_type_t sai_qos_get_queue_type (sai_object_id_t queue_id)
{
    dn_sai_qos_queue_t      *queue_node = sai_qos_queue_node_get(queue_id);
    STD_ASSERT( queue_node != NULL);
    return queue_node->queue_type;
}

dn_sai_qos_sched_group_t *sai_qos_sched_group_node_get (
                                               sai_object_id_t sched_group_id)
{
    rbtree_handle             sched_group_tree;
    dn_sai_qos_sched_group_t  sched_group_node;

    memset (&sched_group_node, 0, sizeof (dn_sai_qos_sched_group_t));

    sched_group_node.key.sched_group_id = sched_group_id;

    sched_group_tree = sai_qos_access_global_config()->scheduler_group_tree;

    if (NULL == sched_group_tree)
        return NULL;

    return ((dn_sai_qos_sched_group_t *)
            std_rbtree_getexact (sched_group_tree, &sched_group_node));
}

t_std_error sai_qos_policer_node_insert(dn_sai_qos_policer_t *p_policer_node)
{
    rbtree_handle policer_tree;
    policer_tree = sai_qos_access_global_config()->policer_tree;

    STD_ASSERT(policer_tree != NULL);

    return std_rbtree_insert(policer_tree, p_policer_node);
}

dn_sai_qos_policer_t *sai_qos_policer_node_get(sai_object_id_t policer_id)
{
    rbtree_handle  policer_tree;
    dn_sai_qos_policer_t policer_entry;

    policer_tree = sai_qos_access_global_config()->policer_tree;

    STD_ASSERT(policer_tree != NULL);

    memset (&policer_entry, 0, sizeof (dn_sai_qos_policer_t));
    policer_entry.key.policer_id = policer_id;

    return ((dn_sai_qos_policer_t *) std_rbtree_getexact (policer_tree,
                                                      &policer_entry));
}

void sai_qos_policer_node_remove(sai_object_id_t policer_id)
{
    rbtree_handle  policer_tree;
    dn_sai_qos_policer_t policer_entry;

    policer_tree = sai_qos_access_global_config()->policer_tree;

    STD_ASSERT(policer_tree != NULL);

    memset (&policer_entry, 0, sizeof (dn_sai_qos_policer_t));
    policer_entry.key.policer_id = policer_id;

    std_rbtree_remove(policer_tree, &policer_entry);
}

sai_acl_rule_t *sai_qos_first_acl_rule_node_from_policer_get(dn_sai_qos_policer_t *p_policer)
{
    uint8_t *p_temp = NULL;
    STD_ASSERT(p_policer != NULL);

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_first(&p_policer->acl_dll_head))))
    {
        return ((sai_acl_rule_t *) (p_temp - SAI_QOS_POLICER_ACL_DLL_GLUE_OFFSET));

    }
    return NULL;
}
sai_acl_rule_t *sai_qos_next_acl_rule_node_from_policer_get(dn_sai_qos_policer_t *p_policer,
                                                            sai_acl_rule_t *p_acl_node)
{
    uint8_t *p_temp = NULL;
    STD_ASSERT(p_policer != NULL);
    STD_ASSERT(p_acl_node != NULL);

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next(&p_policer->acl_dll_head,
                                                    &p_acl_node->policer_glue))))
    {
        return ((sai_acl_rule_t *) (p_temp - SAI_QOS_POLICER_ACL_DLL_GLUE_OFFSET));

    }
    return NULL;
}
dn_sai_qos_port_t *sai_qos_port_node_from_policer_get(dn_sai_qos_policer_t *p_policer,
                                                      uint_t type)
{
    uint8_t *p_temp = NULL;
    STD_ASSERT(p_policer != NULL);

    /*
     * Policer_dll_glue[type] is a pointer in the port node. When policer is applied on port
     * the glue is inserted into the policer_dll_head. On any modification to the policer
     * the policer dll is walked. This returns the pointer to the glue in port node. Offset
     * of (glue base offset + glue[type]) is subtracted to get the base pointer of port node.
     */
    if ((p_temp = ((uint8_t *) sai_qos_dll_get_first(&p_policer->port_dll_head[type]))))
    {
        return ((dn_sai_qos_port_t *) (p_temp -
                                       (SAI_QOS_POLICER_PORT_DLL_GLUE_OFFSET +
                                        (sizeof(std_dll) * type))));
    }
    return NULL;
}

dn_sai_qos_port_t *sai_qos_next_port_node_from_policer_get(dn_sai_qos_policer_t *p_policer,
                                                            dn_sai_qos_port_t *p_port_node,
                                                            uint_t type)
{
    uint8_t *p_temp = NULL;
    STD_ASSERT(p_port_node != NULL);
    STD_ASSERT(p_policer != NULL);

    /*
     * Policer_dll_glue[type] is a pointer in the port node. When policer is applied on port
     * the glue is inserted into the policer_dll_head. On any modification to the policer
     * the policer dll is walked. This returns the pointer to the glue in port node. Offset
     * of (glue base offset + glue[type]) is subtracted to get the base pointer of port node.
     */
    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next(&p_policer->port_dll_head[type],
                                                    &p_port_node->policer_dll_glue[type]))))
    {
        return ((dn_sai_qos_port_t *) (p_temp -
                                       (SAI_QOS_POLICER_PORT_DLL_GLUE_OFFSET +
                                        (sizeof(std_dll) * type))));
    }
    return NULL;
}

static sai_status_t sai_qos_sched_group_child_index_bitmap_get (sai_object_id_t sg_id,
                                                    void **p_bitmap,
                                                    uint_t *max_childs)
{
    dn_sai_qos_sched_group_t    *p_sg_node = NULL;

    STD_ASSERT (max_childs != NULL);

    if (! sai_is_obj_id_scheduler_group (sg_id)) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    p_sg_node = sai_qos_sched_group_node_get (sg_id);

    if (NULL == p_sg_node) {
        SAI_SCHED_GRP_LOG_ERR ("Scheduler group 0x%"PRIx64" "
                               "does not exist in tree.", sg_id);

        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *max_childs = p_sg_node->hqos_info.max_childs;
    *p_bitmap = p_sg_node->hqos_info.child_index_bitmap;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_sched_group_first_free_child_index_get (
                                                 sai_object_id_t sg_id,
                                                 uint_t *child_index)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    void            *p_bitmap = NULL;
    uint_t          max_childs = 0;
    int             free_idx = 0;

    *child_index = 0;

    sai_rc = sai_qos_sched_group_child_index_bitmap_get (sg_id, &p_bitmap,
                                                         &max_childs);
    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Failed to get child index bitmap.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    STD_ASSERT (p_bitmap != NULL);

    free_idx = std_find_first_bit (p_bitmap, max_childs, 0);

    if (free_idx < 0) {
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }
    *child_index = free_idx;
    STD_BIT_ARRAY_CLR (p_bitmap, *child_index);

    return sai_rc;
}

sai_status_t sai_qos_sched_group_child_index_free (sai_object_id_t sg_id,
                                                   uint_t child_index)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    void            *p_bitmap = NULL;
    uint_t          max_childs = 0;

    sai_rc = sai_qos_sched_group_child_index_bitmap_get (sg_id, &p_bitmap,
                                                         &max_childs);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Failed to get child index bitmap.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    STD_ASSERT (p_bitmap != NULL);

    STD_ASSERT (child_index < max_childs);

    STD_BIT_ARRAY_SET (p_bitmap, child_index);

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_child_parent_id_get (sai_object_id_t child_id,
                                          sai_object_id_t *child_parent_id)
{
    sai_status_t               sai_rc = SAI_STATUS_SUCCESS;
    dn_sai_qos_sched_group_t  *p_child_sg_node = NULL;
    dn_sai_qos_queue_t        *p_child_queue_node = NULL;
    sai_object_type_t          child_obj_type = 0;

    STD_ASSERT (child_parent_id != NULL);

    child_obj_type = sai_uoid_obj_type_get (child_id);

    switch (child_obj_type)
    {
        case SAI_OBJECT_TYPE_QUEUE:
            p_child_queue_node = sai_qos_queue_node_get (child_id);

            if (NULL == p_child_queue_node) {
                SAI_SCHED_GRP_LOG_ERR ("Queue 0x%"PRIx64" does not exist in tree.",
                                       child_id);

                return SAI_STATUS_ITEM_NOT_FOUND;
            }

            *child_parent_id =  p_child_queue_node->parent_sched_group_id;

            break;

        case SAI_OBJECT_TYPE_SCHEDULER_GROUP:
            p_child_sg_node = sai_qos_sched_group_node_get (child_id);

            if (NULL == p_child_sg_node) {
                SAI_SCHED_GRP_LOG_ERR ("Scheduler group 0x%"PRIx64" does "
                                       "not exist in tree.",
                                       child_id);

                return SAI_STATUS_ITEM_NOT_FOUND;
            }

            *child_parent_id = p_child_sg_node->hqos_info.parent_sched_group_id;

            break;

        default:
            return SAI_STATUS_INVALID_PARAMETER;
    }

    return sai_rc;
}


static sai_status_t sai_qos_child_index_ptr_get (sai_object_id_t child_id,
                                                 uint_t **p_child_index)
{
    dn_sai_qos_sched_group_t    *p_sg_node = NULL;
    dn_sai_qos_queue_t          *p_queue_node = NULL;

    if (sai_is_obj_id_queue (child_id)) {

        p_queue_node = sai_qos_queue_node_get (child_id);

        if (NULL == p_queue_node) {
            SAI_SCHED_GRP_LOG_ERR ("Queue 0x%"PRIx64" does not exist in tree.",
                               child_id);

            return SAI_STATUS_ITEM_NOT_FOUND;
        }

        *p_child_index = &p_queue_node->child_offset;

    } else if (sai_is_obj_id_scheduler_group (child_id)) {

        p_sg_node = sai_qos_sched_group_node_get (child_id);

        if (NULL == p_sg_node) {
            SAI_SCHED_GRP_LOG_ERR ("Scheduler group 0x%"PRIx64" "
                                   "does not exist in tree.",
                                   child_id);

            return SAI_STATUS_ITEM_NOT_FOUND;
        }

        *p_child_index = &p_sg_node->child_offset;

    } else {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_child_index_update (sai_object_id_t child_id, uint_t child_index)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    uint_t          *p_child_index = NULL;

    sai_rc = sai_qos_child_index_ptr_get (child_id, &p_child_index);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Failed to get child index pointer.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *p_child_index = child_index;

    return sai_rc;
}

sai_status_t sai_qos_child_index_get (sai_object_id_t child_id, uint_t *child_index)
{
    sai_status_t    sai_rc = SAI_STATUS_SUCCESS;
    uint_t          *p_child_index = NULL;

    sai_rc = sai_qos_child_index_ptr_get (child_id, &p_child_index);

    if (sai_rc != SAI_STATUS_SUCCESS) {
        SAI_SCHED_GRP_LOG_ERR ("Failed to get child index pointer.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *child_index = *p_child_index;

    return SAI_STATUS_SUCCESS;
}

dn_sai_qos_sched_group_t *sai_qos_port_get_first_sched_group (
                                  dn_sai_qos_port_t *p_qos_port_node,
                                  uint_t level)

{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
        sai_qos_dll_get_first (&p_qos_port_node->sched_group_dll_head[level])))) {
        return ((dn_sai_qos_sched_group_t *) (p_temp - SAI_QOS_SCHED_GROUP_PORT_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

dn_sai_qos_sched_group_t *sai_qos_port_get_next_sched_group (
                                   dn_sai_qos_port_t *p_qos_port_node,
                                   dn_sai_qos_sched_group_t *p_sg_node)
{
    uint8_t *p_temp = NULL;
    uint_t   level = p_sg_node->hierarchy_level;

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next (
                               &p_qos_port_node->sched_group_dll_head[level],
                               &p_sg_node->port_dll_glue)))) {
        return ((dn_sai_qos_sched_group_t *) (p_temp - SAI_QOS_SCHED_GROUP_PORT_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

dn_sai_qos_sched_group_t *sai_qos_sched_group_get_first_child_sched_group (
                                     dn_sai_qos_sched_group_t *p_sg_node)

{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
        sai_qos_dll_get_first (&p_sg_node->hqos_info.child_sched_group_dll_head)))) {
        return ((dn_sai_qos_sched_group_t *) (p_temp -
                                     SAI_QOS_SCHED_GROUP_CHILD_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

dn_sai_qos_sched_group_t *sai_qos_sched_group_get_next_child_sched_group (
                                   dn_sai_qos_sched_group_t *p_sg_node,
                                   dn_sai_qos_sched_group_t *p_child_sg_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next (
                               &p_sg_node->hqos_info.child_sched_group_dll_head,
                               &p_child_sg_node->child_sched_group_dll_glue)))) {
        return ((dn_sai_qos_sched_group_t *) (p_temp -
                                      SAI_QOS_SCHED_GROUP_CHILD_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

dn_sai_qos_queue_t *sai_qos_sched_group_get_first_child_queue (
                                  dn_sai_qos_sched_group_t *p_sg_node)

{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
        sai_qos_dll_get_first (&p_sg_node->hqos_info.child_queue_dll_head)))) {
        return ((dn_sai_qos_queue_t *) (p_temp -
                                     SAI_QOS_QUEUE_CHILD_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

dn_sai_qos_queue_t *sai_qos_sched_group_get_next_child_queue (
                                   dn_sai_qos_sched_group_t *p_sg_node,
                                   dn_sai_qos_queue_t *p_queue_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next (
                               &p_sg_node->hqos_info.child_queue_dll_head,
                               &p_queue_node->child_queue_dll_glue)))) {
        return ((dn_sai_qos_queue_t *) (p_temp -
                                      SAI_QOS_QUEUE_CHILD_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

sai_status_t sai_qos_sched_group_child_id_list_get (dn_sai_qos_sched_group_t *p_sg_node,
                                              sai_object_list_t *p_child_objlist)
{
    size_t                    child_count = 0;
    dn_sai_qos_sched_group_t *p_child_sg_node = NULL;
    dn_sai_qos_queue_t       *p_child_queue_node = NULL;

    STD_ASSERT (p_sg_node != NULL);
    STD_ASSERT (p_child_objlist != NULL);

    if(p_child_objlist->count == 0) {
        return SAI_STATUS_FAILURE;
    }

    if(p_child_objlist->count < p_sg_node->hqos_info.child_count) {
        SAI_SCHED_GRP_LOG_ERR ("Get child list count %d is less than "
                               "actual child list count %d",
                               p_child_objlist->count, p_sg_node->hqos_info.child_count);

        p_child_objlist->count = p_sg_node->hqos_info.child_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    for (p_child_sg_node = sai_qos_sched_group_get_first_child_sched_group (p_sg_node);
         (p_child_sg_node != NULL); p_child_sg_node =
         sai_qos_sched_group_get_next_child_sched_group (p_sg_node, p_child_sg_node)) {

        p_child_objlist->list[child_count] = p_child_sg_node->key.sched_group_id;
        child_count++;
    }

    for (p_child_queue_node = sai_qos_sched_group_get_first_child_queue (p_sg_node);
         (p_child_queue_node != NULL); p_child_queue_node =
         sai_qos_sched_group_get_next_child_queue (p_sg_node, p_child_queue_node)) {

        p_child_objlist->list[child_count] = p_child_queue_node->key.queue_id;
        child_count++;
    }

    p_child_objlist->count = child_count;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_port_queue_id_list_get (sai_object_id_t port_id,
                                             sai_object_list_t *p_queue_objlist)
{
    dn_sai_qos_port_t   *p_qos_port_node = NULL;
    dn_sai_qos_queue_t  *p_queue_node = NULL;
    size_t               queue_count = 0;
    uint_t               max_queue_count = 0;
    sai_status_t         ret_val;

    STD_ASSERT (p_queue_objlist != NULL);

    if (! sai_is_port_valid(port_id)) {
        SAI_QUEUE_LOG_ERR ("Port 0x%"PRIx64" is not valid", port_id);
        return SAI_STATUS_INVALID_PORT_NUMBER;
    }

    if (p_queue_objlist->count == 0) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    ret_val = sai_qos_port_queue_count_get(port_id, &max_queue_count);
    if(ret_val != SAI_STATUS_SUCCESS) {
        SAI_QUEUE_LOG_ERR ("Get queue id list count failed for port 0x%"PRIx64"",port_id);
        return ret_val;
    }

    if (p_queue_objlist->count < max_queue_count) {
        SAI_QUEUE_LOG_ERR ("Get queue id list count %d is less than "
                           "actual queue id list count %d",
                           p_queue_objlist->count,
                           max_queue_count);

        p_queue_objlist->count = max_queue_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {
        SAI_QUEUE_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                           port_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for (p_queue_node = sai_qos_port_get_first_queue (p_qos_port_node);
         (p_queue_node != NULL) ; p_queue_node =
         sai_qos_port_get_next_queue (p_qos_port_node, p_queue_node)) {

        p_queue_objlist->list[queue_count] = p_queue_node->key.queue_id;
        queue_count++;
    }

    p_queue_objlist->count = queue_count;

    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_port_queue_count_get (sai_object_id_t port_id, uint32_t *count)
{
    dn_sai_qos_port_t   *p_qos_port_node = NULL;
    dn_sai_qos_queue_t  *p_queue_node = NULL;
    uint_t               queue_count = 0;


    if(count == NULL) {
        SAI_QUEUE_LOG_ERR ("Invalid parameter. Count is NULL");
        STD_ASSERT(0);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {
        SAI_QUEUE_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                           port_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for (p_queue_node = sai_qos_port_get_first_queue (p_qos_port_node);
         (p_queue_node != NULL) ; p_queue_node =
         sai_qos_port_get_next_queue (p_qos_port_node, p_queue_node)) {

        queue_count++;
    }
    *count = queue_count;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_port_sched_group_count_get (sai_object_id_t port_id, uint32_t *count)
{
    dn_sai_qos_port_t         *p_qos_port_node = NULL;
    dn_sai_qos_sched_group_t  *p_sg_node = NULL;
    uint_t                     sg_count = 0;
    uint_t                     level = 0;

    if(count == NULL) {
        SAI_QUEUE_LOG_ERR ("Invalid parameter. Count is NULL");
        STD_ASSERT(0);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {
        SAI_QUEUE_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                           port_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for (level = 0; level < sai_switch_max_hierarchy_levels_get (); level++)
    {
        for (p_sg_node = sai_qos_port_get_first_sched_group (p_qos_port_node, level);
             (p_sg_node != NULL);
             p_sg_node = sai_qos_port_get_next_sched_group (p_qos_port_node, p_sg_node)) {
             sg_count++;
        }
    }
    *count = sg_count;
    return SAI_STATUS_SUCCESS;
}

sai_status_t sai_qos_port_sched_group_id_list_get (sai_object_id_t port_id,
                                           sai_object_list_t *p_sg_objlist)
{
    dn_sai_qos_port_t         *p_qos_port_node = NULL;
    dn_sai_qos_sched_group_t  *p_sg_node = NULL;
    size_t                     max_sg_count = 0;
    size_t                     sg_count = 0;
    uint_t                     level = 0;

    STD_ASSERT (p_sg_objlist != NULL);

    if (! sai_is_port_valid(port_id)) {
        SAI_QUEUE_LOG_ERR ("Port 0x%"PRIx64" is not valid", port_id);
    }

    if (p_sg_objlist->count == 0) {
        return SAI_STATUS_FAILURE;
    }

    p_qos_port_node = sai_qos_port_node_get (port_id);

    if (NULL == p_qos_port_node) {
        SAI_QUEUE_LOG_ERR ("Qos Port 0x%"PRIx64" does not exist in tree.",
                           port_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for (level = 0; level < sai_switch_max_hierarchy_levels_get (); level++)
    {
        for (p_sg_node = sai_qos_port_get_first_sched_group (p_qos_port_node, level);
             (p_sg_node != NULL);
             p_sg_node = sai_qos_port_get_next_sched_group (p_qos_port_node, p_sg_node)) {
            max_sg_count++;
        }
    }

    if (p_sg_objlist->count < max_sg_count) {
        SAI_QUEUE_LOG_ERR ("Get scheduler group id list count %d is less than "
                           "actual sg id list count %d", p_sg_objlist->count,
                           max_sg_count);

        p_sg_objlist->count = max_sg_count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    for (level = 0; level < sai_switch_max_hierarchy_levels_get (); level++)
    {
        for (p_sg_node = sai_qos_port_get_first_sched_group (p_qos_port_node, level);
             p_sg_node != NULL;
             p_sg_node = sai_qos_port_get_next_sched_group (p_qos_port_node, p_sg_node)) {
            p_sg_objlist->list[sg_count] = p_sg_node->key.sched_group_id;
            sg_count++;
        }
    }

    p_sg_objlist->count = sg_count;

    return SAI_STATUS_SUCCESS;
}

dn_sai_qos_map_t *sai_qos_map_node_get(const sai_object_id_t map_id)
{
    rbtree_handle  map_tree;
    dn_sai_qos_map_t map_entry;

    map_tree = sai_qos_access_global_config()->map_tree;

    STD_ASSERT(map_tree != NULL);

    memset (&map_entry, 0, sizeof (dn_sai_qos_map_t));
    map_entry.key.map_id = map_id;

    return ((dn_sai_qos_map_t *) std_rbtree_getexact (map_tree,
                                                       &map_entry));
}

t_std_error sai_qos_map_node_insert(const dn_sai_qos_map_t *p_map_node)
{
    rbtree_handle map_tree;

    STD_ASSERT(p_map_node != NULL);
    map_tree = sai_qos_access_global_config()->map_tree;

    STD_ASSERT(map_tree != NULL);

    return std_rbtree_insert(map_tree, (void *)p_map_node);
}

void sai_qos_map_node_remove(sai_object_id_t map_id)
{
    rbtree_handle  map_tree;
    dn_sai_qos_map_t map_entry;

    map_tree = sai_qos_access_global_config()->map_tree;

    STD_ASSERT(map_tree != NULL);

    memset (&map_entry, 0, sizeof (dn_sai_qos_map_t));
    map_entry.key.map_id = map_id;

    std_rbtree_remove(map_tree, &map_entry);
}

dn_sai_qos_port_t *sai_qos_maps_get_port_node_from_map(dn_sai_qos_map_t *p_map_node)
{
    uint8_t *p_temp = NULL;
    STD_ASSERT(p_map_node != NULL);

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_first(&p_map_node->port_dll_head))))
    {
        return ((dn_sai_qos_port_t *) (p_temp -
                                       (SAI_QOS_MAPS_PORT_DLL_GLUE_OFFSET +
                                        (sizeof(std_dll) * p_map_node->map_type))));
    }
    return NULL;
}

dn_sai_qos_port_t *sai_qos_maps_next_port_node_from_map_get(dn_sai_qos_map_t *p_map_node,
                                                            dn_sai_qos_port_t *p_port_node)
{
    uint8_t *p_temp = NULL;

    STD_ASSERT(p_port_node != NULL);
    STD_ASSERT(p_map_node != NULL);

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next(&p_map_node->port_dll_head,
                                                    &p_port_node->maps_dll_glue
                                                    [p_map_node->map_type]))))
    {
        return ((dn_sai_qos_port_t *) (p_temp -
                                       (SAI_QOS_MAPS_PORT_DLL_GLUE_OFFSET +
                                        (sizeof(std_dll) * p_map_node->map_type))));
    }
    return NULL;
}

dn_sai_qos_queue_t *sai_qos_port_get_first_mcast_queue(
                        dn_sai_qos_port_t *p_qos_port_node)
{
    dn_sai_qos_queue_t *p_queue_node = NULL;

    p_queue_node = sai_qos_port_get_first_queue(p_qos_port_node);

    while(p_queue_node != NULL)
    {
        if(p_queue_node->queue_type == SAI_QUEUE_TYPE_MULTICAST)
        {
            return p_queue_node;
        }
        p_queue_node = sai_qos_port_get_next_queue(p_qos_port_node,
                                                   p_queue_node);
    }

    return NULL;
}

sai_npu_object_id_t sai_add_type_to_object(sai_npu_object_id_t id,
                                           uint_t type)
{
    sai_object_id_t embed_id = 0;

    embed_id = (((((sai_object_id_t)type) << SAI_UOID_TYPE_BITPOS) & SAI_UOID_TYPE_MASK) |
              (((id ) <<  SAI_UOID_OBJ_ID_BITPOS) & SAI_UOID_OBJ_ID_MASK));

    return embed_id;
}

uint_t sai_get_type_from_npu_object(sai_npu_object_id_t id)
{
    return ((id & SAI_UOID_TYPE_MASK) >> SAI_UOID_TYPE_BITPOS);
}

sai_npu_object_id_t sai_get_id_from_npu_object(sai_npu_object_id_t id)
{
    return ((id & SAI_UOID_OBJ_ID_MASK) >> SAI_UOID_OBJ_ID_BITPOS);
}

dn_sai_qos_queue_t *sai_qos_port_get_indexed_uc_queue_object(dn_sai_qos_port_t *p_qos_port_node,
                                                          uint_t queue_index)
{
    dn_sai_qos_queue_t *p_queue_node = NULL;
    uint_t count = 0;
    STD_ASSERT(p_qos_port_node != NULL);

    if(queue_index >= sai_switch_max_queues_per_port_get(p_qos_port_node->port_id)){
        return NULL;
    }
    p_queue_node = sai_qos_port_get_first_queue(p_qos_port_node);

    while(p_queue_node != NULL)
    {
        if(count == queue_index){
            return p_queue_node;
        }
        count ++;
        p_queue_node = sai_qos_port_get_next_queue(p_qos_port_node,
                                                   p_queue_node);
    }
    return NULL;
}

dn_sai_qos_queue_t *sai_qos_port_get_indexed_mc_queue_object(dn_sai_qos_port_t *p_qos_port_node,
                                                          uint_t queue_index)
{
    dn_sai_qos_queue_t *p_queue_node = NULL;
    uint_t count = 0;
    STD_ASSERT(p_qos_port_node != NULL);

    if(queue_index >= sai_switch_max_queues_per_port_get(p_qos_port_node->port_id)){
        return NULL;
    }

    p_queue_node = sai_qos_port_get_first_mcast_queue(p_qos_port_node);

    while(p_queue_node != NULL)
    {
        if(count == queue_index){
            return p_queue_node;
        }
        count ++;
        p_queue_node = sai_qos_port_get_next_queue(p_qos_port_node,
                                                   p_queue_node);
    }
    return NULL;
}

dn_sai_qos_scheduler_t *sai_qos_scheduler_node_get (sai_object_id_t sched_id)
{
    rbtree_handle           scheduler_tree;
    dn_sai_qos_scheduler_t  scheduler_node;

    memset (&scheduler_node, 0, sizeof (dn_sai_qos_scheduler_t));

    scheduler_node.key.scheduler_id = sched_id;

    scheduler_tree = sai_qos_access_global_config()->scheduler_tree;

    if (NULL == scheduler_tree)
        return NULL;

    return ((dn_sai_qos_scheduler_t *) std_rbtree_getexact (scheduler_tree,
                                                            &scheduler_node));
}

dn_sai_qos_queue_t *sai_qos_scheduler_get_first_queue (
                                  dn_sai_qos_scheduler_t *p_sched_node)

{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
        sai_qos_dll_get_first (&p_sched_node->queue_dll_head)))) {
        return ((dn_sai_qos_queue_t *) (p_temp -
                                     SAI_QOS_QUEUE_SCHEDULER_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

dn_sai_qos_queue_t *sai_qos_scheduler_get_next_queue (
                                   dn_sai_qos_scheduler_t *p_sched_node,
                                   dn_sai_qos_queue_t *p_queue_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next (
                               &p_sched_node->queue_dll_head,
                               &p_queue_node->scheduler_dll_glue)))) {
        return ((dn_sai_qos_queue_t *) (p_temp -
                                      SAI_QOS_QUEUE_SCHEDULER_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

dn_sai_qos_sched_group_t *sai_qos_scheduler_get_first_sched_group (
                                  dn_sai_qos_scheduler_t *p_sched_node)

{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
        sai_qos_dll_get_first (&p_sched_node->sched_group_dll_head)))) {
        return ((dn_sai_qos_sched_group_t *) (p_temp -
                                     SAI_QOS_SCHED_GROUP_SCHEDULER_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

dn_sai_qos_sched_group_t *sai_qos_scheduler_get_next_sched_group (
                                   dn_sai_qos_scheduler_t *p_sched_node,
                                   dn_sai_qos_sched_group_t *p_sg_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next (
                               &p_sched_node->sched_group_dll_head,
                               &p_sg_node->scheduler_dll_glue)))) {
        return ((dn_sai_qos_sched_group_t *) (p_temp -
                                      SAI_QOS_SCHED_GROUP_SCHEDULER_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

dn_sai_qos_port_t *sai_qos_scheduler_get_first_port (
                                  dn_sai_qos_scheduler_t *p_sched_node)

{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *)
        sai_qos_dll_get_first (&p_sched_node->port_dll_head)))) {
        return ((dn_sai_qos_port_t *) (p_temp -
                                       SAI_QOS_PORT_SCHEDULER_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

dn_sai_qos_port_t *sai_qos_scheduler_get_next_port (
                                   dn_sai_qos_scheduler_t *p_sched_node,
                                   dn_sai_qos_port_t *p_qos_port_node)
{
    uint8_t *p_temp = NULL;

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next (
                               &p_sched_node->port_dll_head,
                               &p_qos_port_node->scheduler_dll_glue)))) {
        return ((dn_sai_qos_port_t *) (p_temp -
                                      SAI_QOS_PORT_SCHEDULER_DLL_GLUE_OFFSET));
    } else {
        return NULL;
    }
}

dn_sai_qos_wred_t *sai_qos_wred_node_get(const sai_object_id_t wred_id)
{
    rbtree_handle  wred_tree;
    dn_sai_qos_wred_t wred_entry;

    wred_tree = sai_qos_access_global_config()->wred_tree;

    STD_ASSERT(wred_tree != NULL);

    memset (&wred_entry, 0, sizeof (dn_sai_qos_wred_t));
    wred_entry.key.wred_id = wred_id;

    return ((dn_sai_qos_wred_t *) std_rbtree_getexact (wred_tree,
                                                       &wred_entry));
}

t_std_error sai_qos_wred_node_insert(const dn_sai_qos_wred_t *p_wred_node)
{
    rbtree_handle wred_tree;

    STD_ASSERT(p_wred_node != NULL);
    wred_tree = sai_qos_access_global_config()->wred_tree;

    STD_ASSERT(wred_tree != NULL);

    return std_rbtree_insert(wred_tree, (void *)p_wred_node);
}

void sai_qos_wred_node_remove(sai_object_id_t wred_id)
{
    rbtree_handle  wred_tree;
    dn_sai_qos_wred_t wred_entry;

    wred_tree = sai_qos_access_global_config()->wred_tree;

    STD_ASSERT(wred_tree != NULL);

    memset (&wred_entry, 0, sizeof (dn_sai_qos_wred_t));
    wred_entry.key.wred_id = wred_id;

    std_rbtree_remove(wred_tree, &wred_entry);
}

dn_sai_qos_queue_t *sai_qos_queue_node_from_wred_get(dn_sai_qos_wred_t *p_wred_node)
{
    uint8_t *p_temp = NULL;
    STD_ASSERT(p_wred_node != NULL);

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_first(&p_wred_node->queue_dll_head))))
    {
        return ((dn_sai_qos_queue_t *) (p_temp - SAI_QOS_WRED_QUEUE_DLL_GLUE_OFFSET));
    }
    return NULL;
}

dn_sai_qos_queue_t *sai_qos_next_queue_node_from_wred_get(dn_sai_qos_wred_t *p_wred_node,
                                                            dn_sai_qos_queue_t *p_queue_node)
{
    uint8_t *p_temp = NULL;

    STD_ASSERT(p_wred_node != NULL);
    STD_ASSERT(p_queue_node != NULL);

    if ((p_temp = ((uint8_t *) sai_qos_dll_get_next(&p_wred_node->queue_dll_head,
                                                    &p_queue_node->wred_dll_glue))))
    {
        return ((dn_sai_qos_queue_t *) (p_temp - SAI_QOS_WRED_QUEUE_DLL_GLUE_OFFSET));
    }
    return NULL;
}
sai_qos_map_type_t sai_get_map_type_from_port_attr(sai_attr_id_t port_attr)
{
    if(port_attr == SAI_PORT_ATTR_QOS_DOT1P_TO_TC_MAP){
        return SAI_QOS_MAP_DOT1P_TO_TC;
    }else if(port_attr == SAI_PORT_ATTR_QOS_DOT1P_TO_COLOR_MAP){
        return SAI_QOS_MAP_DOT1P_TO_COLOR;
    }else if(port_attr == SAI_PORT_ATTR_QOS_DOT1P_TO_TC_AND_COLOR_MAP){
        return SAI_QOS_MAP_DOT1P_TO_TC_AND_COLOR;
    }else if(port_attr == SAI_PORT_ATTR_QOS_DSCP_TO_TC_MAP){
        return SAI_QOS_MAP_DSCP_TO_TC;
    }else if(port_attr == SAI_PORT_ATTR_QOS_DSCP_TO_COLOR_MAP){
        return SAI_QOS_MAP_DSCP_TO_COLOR;
    }else if(port_attr == SAI_PORT_ATTR_QOS_DSCP_TO_TC_AND_COLOR_MAP){
        return SAI_QOS_MAP_DSCP_TO_TC_AND_COLOR;
    }else if(port_attr == SAI_PORT_ATTR_QOS_TC_TO_QUEUE_MAP){
        return SAI_QOS_MAP_TC_TO_QUEUE;
    }else if(port_attr == SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DOT1P_MAP){
        return SAI_QOS_MAP_TC_AND_COLOR_TO_DOT1P;
    }else if(port_attr == SAI_PORT_ATTR_QOS_TC_AND_COLOR_TO_DSCP_MAP){
        return SAI_QOS_MAP_TC_AND_COLOR_TO_DSCP;
    }else if(port_attr == SAI_PORT_ATTR_QOS_TC_TO_DSCP_MAP){
        return SAI_QOS_MAP_TC_TO_DSCP;
    }else if(port_attr == SAI_PORT_ATTR_QOS_TC_TO_DOT1P_MAP){
        return SAI_QOS_MAP_TC_TO_DOT1P;
    }else if(port_attr == SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP){
        return SAI_QOS_MAP_TC_TO_PRIORITY_GROUP;
    }else if(port_attr == SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP){
        return SAI_QOS_MAP_PFC_PRIORITY_TO_QUEUE;
    }

    return SAI_QOS_MAP_INVALID_TYPE;
}
dn_sai_qos_queue_t *sai_qos_port_get_indexed_queue_object(dn_sai_qos_port_t *p_qos_port_node,
                                                          uint_t queue_index)
{
    dn_sai_qos_queue_t *p_queue_node = NULL;
    uint_t count = 0;
    STD_ASSERT(p_qos_port_node != NULL);

    if(queue_index >= sai_switch_max_queues_per_port_get(p_qos_port_node->port_id)){
        return NULL;
    }
    p_queue_node = sai_qos_port_get_first_queue(p_qos_port_node);

    while(p_queue_node != NULL)
    {
        if(count == queue_index){
            return p_queue_node;
        }
        count ++;
        p_queue_node = sai_qos_port_get_next_queue(p_qos_port_node,
                                                   p_queue_node);
    }
    return NULL;
}

sai_status_t sai_qos_get_tc_from_pg (sai_object_id_t port_id, uint_t pg_id, uint_t *tc)
{
    dn_sai_qos_port_t *port_node;
    dn_sai_qos_map_t *map_node;
    uint_t index = 0;

    port_node = sai_qos_port_node_get (port_id);

    if(port_node == NULL ) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if(port_node->maps_id[SAI_QOS_MAP_TC_TO_PRIORITY_GROUP] == SAI_NULL_OBJECT_ID) {
        *tc = SAI_QOS_DEFAULT_TC;
         return SAI_STATUS_SUCCESS;
    }

    map_node = sai_qos_map_node_get(port_node->maps_id[SAI_QOS_MAP_TC_TO_PRIORITY_GROUP]);

    if(map_node == NULL) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for(index = 0; index < map_node->map_to_value.count; index++) {
        if(map_node->map_to_value.list[index].value.pg == pg_id) {
            *tc = map_node->map_to_value.list[index].key.tc;
            return SAI_STATUS_SUCCESS;
        }
    }
    return SAI_STATUS_FAILURE;
}

