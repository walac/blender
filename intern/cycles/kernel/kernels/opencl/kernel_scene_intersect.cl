/*
 * Copyright 2011-2015 Blender Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "split/kernel_scene_intersect.h"

__kernel void kernel_ocl_path_trace_scene_intersect(
        ccl_global char *globals,
        ccl_constant KernelData *data,
        ccl_global uint *rng_coop,
        ccl_global Ray *Ray_coop,              /* Required for scene_intersect */
        ccl_global PathState *PathState_coop,  /* Required for scene_intersect */
        Intersection *Intersection_coop,       /* Required for scene_intersect */
        ccl_global char *ray_state,            /* Denotes the state of each ray */
        int sw, int sh,
        ccl_global int *Queue_data,            /* Memory for queues */
        ccl_global int *Queue_index,           /* Tracks the number of elements in queues */
        int queuesize,                         /* Size (capacity) of queues */
        ccl_global char *use_queues_flag,      /* used to decide if this kernel should use queues to fetch ray index */
#ifdef __KERNEL_DEBUG__
        DebugData *debugdata_coop,
#endif
        int parallel_samples)                  /* Number of samples to be processed in parallel */
{
	kernel_scene_intersect(globals,
	                       data,
	                       rng_coop,
	                       Ray_coop,
	                       PathState_coop,
	                       Intersection_coop,
	                       ray_state,
	                       sw, sh,
	                       Queue_data,
	                       Queue_index,
	                       queuesize,
	                       use_queues_flag,
#ifdef __KERNEL_DEBUG__
	                       debugdata_coop,
#endif
	                       parallel_samples);
}
