
////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include "wobjl.h"

int main(int argc, char const *argv[]) {

	obj_t * obj;
	obj = read_obj(argv[1]);
	if(obj == NULL) return -1;

	printf("┌────────────────────────────────────────────────────────\n");
	printf("│ Read '%s'\n│\n", argv[1]);
	printf("│ has normals: "); (obj->has_normals) ? printf("true\n") : printf("false\n");
	printf("│ has textures: "); (obj->has_textures) ? printf("true\n") : printf("false\n");
	printf("│ is only triangles: "); (obj->triangulated) ? printf("true\n") : printf("false\n");
	printf("│\n│ object has %i faces\n", obj->nfaces);
	printf("│\n│ object has %i vertecies\n", obj->nverts);
	printf("└────────────────────────────────────────────────────────\n");


	return 0;
}
