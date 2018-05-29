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

/*
 *  * filename:sai_infra_api.h
 *  */


#ifndef __SAI_INFRA_API_H
#define __SAI_INFRA_API_H
#include "std_type_defs.h"
#include "saiswitch.h"

/** \defgroup SAISWITCHAPIS SAI - Switch and infra implementation
 *   Switch and Infra APIs for SAI. APIS are to be used by SAI
 *   only and not the upper layers.
 *
 *  \{
 */

/**
 * @brief  Retrieve switch functionality method table.
 *
 * @return Address of the structure containing the switch functionality
 *         method table.
 **/

sai_switch_api_t* sai_switch_api_query(void);


/**
 * @brief Initialize the switch and the SDK using the hardware id.
 *
 * @param[in] profile_id - Profile id
 * @param[in] switch_hardware_id - Switch hardware id which identifies
 *        the NPU
 * @param[in] microcode_module_name - Microcode name to be loaded
 * @param[in] switch_notifications - Pointer containing functions for
 *        callback
 *
 * @return SAI_STATUS_SUCCESS if operation is successful else a
 *         different error code is returned
 */

sai_status_t sai_switch_initialize(sai_switch_profile_id_t profile_id,
                               char* switch_hardware_id,
                               char* microcode_module_name,
                               sai_switch_notification_t* switch_notifications);

/**
 *  @brief Release all resources allocated to this switch.
 *
 *  @param warm_restart_hint - Hint for warm restart.
 *
 */

void sai_switch_shutdown(bool warm_restart_hint);


/**
 *  @brief Set the switch attribute value.
 *
 *  @param[in] attr - switch attribute structure with the attribute
 *                    and value.
 *
 *  @return SAI_STATUS_SUCCESS - on success or a failure status code on
 *          error
 */

sai_status_t sai_switch_set_attribute(const sai_attribute_t *attr);


/**
 * @brief Get the switch attribute value.
 *
 * @param[in] attr_count - number of switch attributes to get
 * @param[out] attr_list - Array which holds the attributes and values
 *
 * @return SAI_STATUS_SUCCESS - on success or a failure status code on
 *         error
 */

sai_status_t sai_switch_get_attribute(unsigned int attr_count,
                                      sai_attribute_t *attr_list);



/**
 * @brief This API connects library to the initialized SDK.
 *        Multiple agents can call this to access SDK APIs.
 *
 * @param[in] profile_id - Profile id
 * @param[in] switch_hardware_id - Switch hardware id which identifies
 *        the NPU
 * @param[in] switch_notifications - Pointer containing functions for
 *        callback
 *
 * @return SAI_STATUS_SUCCESS if operation is successful else a
 *         different error code is returned
 */

sai_status_t sai_connect_switch(sai_switch_profile_id_t profile_id,
                               char* switch_hardware_id,
                               sai_switch_notification_t* switch_notifications);


/**
 * @brief This API disconnects library from the initialized SDK.
 *
 */

void  sai_disconnect_switch(void);


/**
 * \}
 */

#endif
