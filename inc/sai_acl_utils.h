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
* @file sai_acl_utils.h
*
* @brief This file contains utility functions for  SAI ACL component.
*
*************************************************************************/

/* OPENSOURCELICENSE */

#ifndef __SAI_ACL_UTILS_H__
#define __SAI_ACL_UTILS_H__

#include "saitypes.h"
#include "sai_event_log.h"

/** \defgroup SAIACLUTILS SAI - ACL Utility functions
 *  Util functions in the SAI ACL component
 *
 *  \{
 */

/**
 * @brief Enumeration to specifiy ACL Atribute data types
 */
typedef enum _sai_acl_rule_attr_type {
    SAI_ACL_ENTRY_ATTR_BOOL,
    SAI_ACL_ENTRY_ATTR_ONE_BYTE,
    SAI_ACL_ENTRY_ATTR_TWO_BYTES,
    SAI_ACL_ENTRY_ATTR_FOUR_BYTES,
    SAI_ACL_ENTRY_ATTR_ENUM,
    SAI_ACL_ENTRY_ATTR_MAC,
    SAI_ACL_ENTRY_ATTR_IPv4,
    SAI_ACL_ENTRY_ATTR_IPv6,
    SAI_ACL_ENTRY_ATTR_OBJECT_ID,
    SAI_ACL_ENTRY_ATTR_OBJECT_LIST,
    SAI_ACL_ENTRY_ATTR_ONE_BYTE_LIST,
    SAI_ACL_ENTRY_ATTR_INVALID
} sai_acl_rule_attr_type;

/** Logging utility for SAI ACL API */
#define SAI_ACL_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_ACL, level)) { \
            SAI_LOG_UTIL(ev_log_t_ACL, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI ACL API */
#define SAI_ACL_LOG_TRACE(msg, ...) \
        SAI_ACL_LOG (SAI_LOG_DEBUG, msg, ##__VA_ARGS__)

#define SAI_ACL_LOG_CRIT(msg, ...) \
        SAI_ACL_LOG (SAI_LOG_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_ACL_LOG_ERR(msg, ...) \
        SAI_ACL_LOG (SAI_LOG_ERROR, msg, ##__VA_ARGS__)

#define SAI_ACL_LOG_INFO(msg, ...) \
        SAI_ACL_LOG (SAI_LOG_INFO, msg, ##__VA_ARGS__)

#define SAI_ACL_LOG_WARN(msg, ...) \
        SAI_ACL_LOG (SAI_LOG_WARN, msg, ##__VA_ARGS__)

#define SAI_ACL_LOG_NTC(msg, ...) \
        SAI_ACL_LOG (SAI_LOG_NOTICE, msg, ##__VA_ARGS__)

/** Custom ACL Field macro */
#define SAI_ACL_ENTRY_ATTR_FIELD_DST_PORT (SAI_ACL_TABLE_ATTR_CUSTOM_RANGE_BASE + 1)

/**
 * @brief Accessor function for fetching the data type of attribute passed
 *
 * @param[in] attribute_id  Attribute Id
 * @return Enum value of the attribute data type
 */
sai_acl_rule_attr_type sai_acl_rule_get_attr_type (
                                        sai_attr_id_t attribute_id);

/**
 * @brief To determine the ACL Table field belongs to the UDF range
 *
 * @param[in] attribute_id  Attribute Id
 * @return Bool value: True if ACL table field belongs to UDF range, else false.
 */
static inline bool sai_acl_table_udf_field_attr_range(
                                        sai_attr_id_t attribute_id)
{
    if ((attribute_id >= SAI_ACL_TABLE_ATTR_USER_DEFINED_FIELD_GROUP_MIN) &&
        (attribute_id <= SAI_ACL_TABLE_ATTR_USER_DEFINED_FIELD_GROUP_MAX)) {
        return true;
    }

    return false;
}

/**
 * @brief To determine the ACL Rule field belongs to the UDF range
 *
 * @param[in] attribute_id  Attribute Id
 * @return Bool value: True if ACL rule field belongs to UDF range, else false.
 */
static inline bool sai_acl_rule_udf_field_attr_range(
                                        sai_attr_id_t attribute_id)
{
    if ((attribute_id >= SAI_ACL_ENTRY_ATTR_USER_DEFINED_FIELD_MIN) &&
        (attribute_id <= SAI_ACL_ENTRY_ATTR_USER_DEFINED_FIELD_MAX)) {
        return true;
    }

    return false;
}

/**
 * \}
 */

#endif /* __SAI_ACL_UTILS_H__ */

