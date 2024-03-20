#define MAX_INSTANCED_COUNT 250
#define INSTANCED_LOCATION(x) x * MAX_INSTANCED_COUNT * 4 + 110
#define INSTANCED_PROPERTY(loc, type, name) layout(location = INSTANCED_LOCATION(loc)) uniform type name[MAX_INSTANCED_COUNT]
#define ACCESS_INSTANCED_PROPERTY(name) name[gl_InstanceID]