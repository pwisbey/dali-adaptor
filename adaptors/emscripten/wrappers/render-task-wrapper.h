#ifndef __DALI_RENDER_TASK_WRAPPER_H__
#define __DALI_RENDER_TASK_WRAPPER_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
 *
 */

// EXTERNAL INCLUDES
#include <dali/public-api/dali-core.h>
#include "emscripten/emscripten.h"
#include "emscripten/bind.h"

// INTERNAL INCLUDES
#include "actor-wrapper.h"

namespace Dali
{
namespace Internal
{
namespace Emscripten
{

/**
 * Gets the local coordinates from the screen coordinates
 *
 * @param[in] self The render task
 * @param[in] actor The Dali actor
 * @param[in] screenX The screen X position
 * @param[in] screenY The screen Y position
 *
 * @returns The Local coordinates
 *
 */
Dali::Vector2 ScreenToLocal(Dali::RenderTask self, Dali::Actor actor, float screenX, float screenY);

/**
 * Gets the screen coordinates from a position
 *
 * @param[in] self The RenderTask
 * @param[in] position The position
 *
 * @returns The screen coordinates of the actor
 *
 */
Dali::Vector2 WorldToScreen(Dali::RenderTask self, const Dali::Vector3 &position);

}; // namespace Emscripten
}; // namespace Internal
}; // namespace Dali

#endif // header
