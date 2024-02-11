#pragma once

#define SNOW_LEOPARD_NO_COPY_CONSTRUCTOR(className) className(const className&) = delete;
#define SNOW_LEOPARD_NO_MOVE_CONSTRUCTOR(className) className(className &&) = delete;
#define SNOW_LEOPARD_DEFAULT_CONSTRUCTOR(className) className() = default;
#define SNOW_LEOPARD_NO_COPY_MOVE_CONSTRUCTOR(className) SNOW_LEOPARD_NO_COPY_CONSTRUCTOR(className) SNOW_LEOPARD_NO_MOVE_CONSTRUCTOR(className)
#define SNOW_LEOPARD_DEFAULT_NO_COPY_MOVE_CONSTRUCTOR(className) SNOW_LEOPARD_DEFAULT_CONSTRUCTOR(className) SNOW_LEOPARD_NO_COPY_MOVE_CONSTRUCTOR(className) 