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

/***
 * \file    sai_lag_callback.h
 *
 * \brief Declaration of SAI LAG related CALLBACKs
*/

#if !defined (__SAILAGCALLBACK_H_)
#define __SAILAGCALLBACK_H_

#include "saitypes.h"
#include "saistatus.h"

/*LAG Operation: List of operations possible on a LAG*/
typedef enum _sai_lag_operation_t {
    /*Create a LAG*/
    SAI_LAG_OPER_CREATE,
    /*Delete a LAG*/
    SAI_LAG_OPER_DELETE,
    /*Add ports to a LAG*/
    SAI_LAG_OPER_ADD_PORTS,
    /*Delete ports from a a LAG*/
    SAI_LAG_OPER_DEL_PORTS,
    /*Set the list of ports on a LAG*/
    SAI_LAG_OPER_SET_PORTS
} sai_lag_operation_t;

/** SAI LAG CALLBACK API - LAG RIF Callback
      \param[in] lag_id LAG Identifier
      \param[in] rif_id Router interface Identifier
      \param[in] port_list List of ports
      \param[in] lag_operation Operation performed on LAG
*/
typedef sai_status_t (*sai_lag_l3_rif_callback) (
    sai_object_id_t lag_id,
    sai_object_id_t rif_id,
    const sai_object_list_t *port_list,
    sai_lag_operation_t lag_operation
);

/*Lag Callback: List of callbacks registered with LAG Module*/
typedef struct _sai_lag_callback_t {
    /*rif_callback: Router interface callback*/
    sai_lag_l3_rif_callback rif_callback;
} sai_lag_callback_t;

/** SAI LAG CALLBACK API - Register Router interface callback
      \param[in]rif_callback Router interface callback
*/
void sai_lag_rif_callback_register(sai_lag_l3_rif_callback rif_callback);

#endif
