//todor licence

#ifndef DALI_INTERNAL_ADAPTOR_TIZEN_VR_PLATFORM_HEADER_H
#define DALI_INTERNAL_ADAPTOR_TIZEN_VR_PLATFORM_HEADER_H

// CLASS HEADER
//todor
#define TIZENVR_USE_DYNAMIC_LIBRARY

//#include "vr-engine-impl-tizen.h"
#define GL_DEPTH_COMPONENT24              0x81A6

#ifdef TIZENVR_USE_DYNAMIC_LIBRARY

// These are types from the TizenVR Engine.
// This does not adhere to DALi coding standards as is a copy/paste from the TizenVR engine for maintainability.

#ifndef __TZVR_TYPES_H__
#define __TZVR_TYPES_H__
//todor #include <tizen.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef void *EGLDisplay;
typedef void *EGLSurface;
typedef void *EGLContext;

//todor #define RAW_QUAT 1
typedef struct _tzvr_vec3f
{
  float x, y, z;
}tzvr_vector3f;

typedef struct tzvr_quat
{
  float w, x, y, z;
}tzvr_quaternionF;

typedef enum {
  EYE_INDEX_LEFT = 0,
  EYE_INDEX_RIGHT = 1,
  EYE_INDEX_MAX = 2,
}tzvr_eye_index_e;

typedef enum {
  TEXTURE_TYPE_2D = 0x0DE1,     /* GL_TEXUTRE_2D*/
  //TEXTURE_TYPE_2D_EXTERNAL = ?,
  TEXTURE_TYPE_2D_ARRAY = 0x8C1A,   /* GL_TEXUTRE_2D_ARRAY*/
  TEXTURE_TYPE_CUBE = 0x8513,     /* GL_TEXUTRE_CUBE_MAP*/
}tzvr_texture_type_e; /* TODO: add more supportable type*/

typedef enum
{
    /* To disable chromatic aberration */
    DISABLE_CHROMATIC_ABERRATION = 0x0,
    /* To enable chromatic aberration */
    ENABLE_CHROMATIC_ABERRATION
}tzvr_chromatic_aberration_e;

typedef struct eyePose {
  float x, y, z;
  float w;  /* For quaternion */
  double timestamp;
}eyePose_s;

typedef struct tzvr_submit_params {
  eyePose_s render_pose[EYE_INDEX_MAX];
  long long frame_index;
  float *view;
  unsigned int min_vsync_wait;
  tzvr_chromatic_aberration_e chromatic_value;
}tzvr_submit_params_s;

/* Tizen VR Error enumaration */
typedef enum
{
    /* Successful */
    TZ_VR_SUCCESS = 0,
    /* Out of memory */
    TZ_VR_ERROR_OUT_OF_MEMORY = 1,
    /* Invalid parameter */
    TZ_VR_ERROR_INVALID_PARAMETER = 2,
    /* Invalid operation */
    TZ_VR_ERROR_INVALID_OPERATION = 3,

    /* General error */
    TZ_VR_ERROR_GENERAL = 4 + 0x1
}_tzvr_engine_error_e;

typedef _tzvr_engine_error_e tzvr_ret_type;
typedef void *frame_buffer_handle;

typedef size_t _tzvr_context_ptr; // Handle to hold pointer/address
typedef _tzvr_context_ptr tzvr_context;
#ifdef __cplusplus
}
#endif
#endif /* __TZVR_TYPES_H__ */


#include <dlfcn.h>
#else
#include <vr-engine/inc/Core/tzvr.h>
#endif // TIZENVR_USE_DYNAMIC_LIBRARY

//todor
#if 0
#include <adaptors/common/gl/egl-implementation.h>
#include <adaptors/common/gl/egl-factory.h>
#include <adaptors/common/gl/egl-implementation.h>
#include <adaptors/common/gl/gl-implementation.h>

#include <dali/integration-api/debug.h>

#include <dali/public-api/math/matrix.h>
#endif

#endif // DALI_INTERNAL_ADAPTOR_TIZEN_VR_PLATFORM_HEADER_H
