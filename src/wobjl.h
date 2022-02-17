#ifndef _WOBJL_H_
#define _WOBJL_H_

#include <stdbool.h>
/****************************************************************
 *    @author Gustaf Franzén :: https://github.com/BjorneEk;    *
 *                                                              *
 *            wobjl (wavefront object library)                  *
 ****************************************************************/

typedef signed char        i8_t;
typedef unsigned char      u8_t;
typedef signed short       i16_t;
typedef unsigned short     u16_t;
typedef signed int         i32_t;
typedef unsigned int       u32_t;
typedef signed long long   i64_t;
typedef unsigned long long u64_t;


/**
 *   represents a .obj file vertex.
 **/
typedef union vertex
{
	double a[4];
	struct
	{
		double x, y, z;
		/**
		 * optional coordinate, default value is 1
		 **/
		double w;
	};
} vert_t;

/**
 *   represents a .obj file texture vertex.
 **/
typedef union texture_vertex
{
	double a[3];
	struct
	{
		double u;

		/**
		 * optional values, default value is 0
		 **/
		double v;
		double w;
	};

} vertt_t;

/**
 *   represents a .obj file normal vertex,
 *   does not have to be a unit vector.
 **/
typedef union normal_vertex
{
	double a[3];
	struct
	{
		double x, y, z;
	};

} vertn_t;

/**
 *   represents a .obj file face/polygon
 **/
typedef struct face
{

	bool has_normals;
	bool has_textures;

	/* number of vertecies in face, minimum 3 */
	u32_t nverts;

	/* index of vertecies defining the polygon */
	i32_t * vertecies;

	/* index of texture vertecies for each vertex, if they exists */
	i32_t * textures;

	/* index of normal vertecies for each vertex, if they exists */
	i32_t * normals;

} face_t;

/**
 *   represents a buffered .obj file
 **/
typedef struct buffered_obj
{
	 /* union FLAGS information about the buffered object */
	union
	{
		 /* entire flags byte */
		u8_t flags;

		/* bitfield for easy setting and checking */
		struct
		{
			u8_t has_normals  : 1;
			u8_t has_textures : 1;
			u8_t is_complete  : 1;
			u8_t triangulated : 1;
		};
	};

	 /* total face count */
	i32_t nfaces;

	 /* total vertex count */
	i32_t nverts;

	/**
	 *   total normal vertex and texture vertex count
	 *   these should be equal to nverts .ie vertex count
	 **/
	i32_t nnorms;
	i32_t ntexts;

	 /* arrays of all vertecies in object */
	vert_t  * vertecies;
	vertn_t * vertex_normals;
	vertt_t * texture_vertecies;

	 /* array of all faces in object */
	face_t * faces;

} obj_t;

obj_t * new_objptr();

obj_t * read_obj(char * filename);



#endif /* _WOBJL_H_ */
