#define MAX_INSTANCED_COUNT 335
#define INSTANCED_LOCATION(x) MAX_INSTANCED_COUNT * 2 * (x+1) - MAX_INSTANCED_COUNT
#define INSTANCED_PROPERTY(loc, type, name) layout(location = INSTANCED_LOCATION(loc)) uniform type name[MAX_INSTANCED_COUNT]
#define ACCESS_INSTANCED_PROPERTY(name) name[gl_InstanceID]