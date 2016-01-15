#ifndef __DALI_WL_TYPES_H__
#define __DALI_WL_TYPES_H__

/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <wayland-client.h>

namespace Dali
{

typedef ::wl_display WlDisplay;
typedef ::wl_surface WlSurface;

} // namespace Dali

#endif /* __DALI_WL_TYPES_H__ */