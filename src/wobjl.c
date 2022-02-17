
/****************************************************************
 *    @author Gustaf Franzén :: https://github.com/BjorneEk;    *
 *                                                              *
 *            wobjl (wavefront object library)                  *
 ****************************************************************/

#include "wobjl.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


#define WHITESPACE(c) ((c==' ') || (c=='\t') || (c=='\n'))

#define VERTEX_DEF    'v'
#define FACE_DEF      'f'
#define OBJNAME_DEF   'o'
#define GROUPNAME_DEF 'g'
#define SHADING_DEF   's'
#define EMPTYLINE     '\n'

#define NORMAL_DEF  'n'
#define TEXTURE_DEF 't'

#define DEBUG

#undef DEBUG

	/* to keep unreleated functions out of the way */
static void log_warning(const char * msg);
static void log_warning_str(const char * msg, char * str);
static void log_error(const char * msg);
static void log_error_str(const char * msg, char * str);
static void log_error_char(const char * msg, char c);
static void log_error_int(const char * msg, int c);



int32_t  index_of( const char * str, const char c )
{
	const char * ptr = strchr(str, c);
	return (ptr) ? (ptr - str) : -1;
}



bool      contains( const char * str, char c )
{
	return (index_of(str, c) != -1);
}

char * str_until(const char * src, char delim)
{
	int len;
	char * res;

	len = index_of(src, delim);

	res = malloc(len+1);
	strncpy(res, src, len);
	return res;
}
 /* number of occurances of c in str */
int nchars(char * str, char c)
{
	int res = 0;
	for(char * s = str; *s != '\0'; s++) if(*s==c)res++;
	return res;
}


 /* split does not allocate new memory so str cannot be freed */
size_t    split(char * str, char ** res, char delimitor)
{
	size_t strl = strlen(str);
	size_t len;

	for (len = 0, *res = str; str < (strl + *res); str++)
	{
		if (*str == delimitor)
		{
			*str = '\0';
			str++;
			len++;
			res[len] = str;
		}
	}
	return len;
}

 /* removes leading and trailing whitespaces from a given string */
void   trim (char ** str)
{
	int i;
	for(i = strlen(*str)-1; WHITESPACE(*(*str+i)); i--);
	*(*str+i+1)='\0';
	while(WHITESPACE(**str))(*str)++;
}

void   trim_objsyntax(char ** str)
{
	/* remove potential comments */
	for(char * s = *str; *s != '\0'; s++) if(*s=='#')*s='\0';

	/* remove leading and trailing whitespaces */
	trim(str);
}

obj_t *    new_objptr()
{
	obj_t * res;
	res = malloc(sizeof(obj_t));
	res->flags  = 0;
	res->nfaces = 0;
	res->nnorms = 0;
	res->ntexts = 0;
	res->vertecies         = NULL;
	res->vertex_normals    = NULL;
	res->texture_vertecies = NULL;
	res->faces             = NULL;
	return res;
}


vert_t parse_vertex(char * vertstr)
{
	vert_t res;
	int    ncords;
	size_t i;

	trim_objsyntax(&vertstr);
	res = (vert_t){1.0,1.0,1.0,1.0};
	ncords = 1 + nchars(vertstr, ' ');

	for (i = 0; i < ncords-1; i++)
	{
		res.a[i] = atof(vertstr);
		vertstr += index_of(vertstr, ' ') + 1;
	}
	res.a[i] = atof(vertstr);
	#ifdef DEBUG
	printf("VERTEX: <x=%f, y=%f, z=%f, w=%f>\n",res.x,res.y,res.z,res.w);
	#endif
	return res;
}

vertn_t parse_normal(char * normstr)
{
	vertn_t res;
	size_t  i;

	trim_objsyntax(&normstr);
	res = (vertn_t){1.0,1.0,1.0};

	for (i = 0; i < 2; i++)
	{
		res.a[i] = atof(normstr);
		normstr += index_of(normstr, ' ') + 1;
	}
	res.a[i] = atof(normstr);
	#ifdef DEBUG
	printf("normal: <x=%f, y=%f, z=%f>\n",res.x,res.y,res.z);
	#endif
	return res;
}
vertt_t parse_texture(char * textstr)
{
	vertt_t res;
	int     ncords;
	size_t  i;

	trim_objsyntax(&textstr);
	ncords = 1 + nchars(textstr, ' ');
	res = (vertt_t){0.0,0.0,0.0};

	for (i = 0; i < ncords-1; i++)
	{
		res.a[i] = atof(textstr);
		textstr += index_of(textstr, ' ') + 1;
	}
	res.a[i] = atof(textstr);
	#ifdef DEBUG
	printf("TEXTURE: <u=%f, v=%f, w=%f>\n",res.u,res.v,res.w);
	#endif
	return res;
}

bool has_normals(char * str)
{
	char * tmp = str_until(str, ' ');
	int i = nchars(tmp, '/');
	free(tmp);
	return (i==2);
}

bool has_textures(char * str)
{
	if (!contains(str, '/')) return false;
	return (*(str + index_of(str, '/') + 1) != '/');
}

void print_face(face_t face)
{
	printf("FACE: <");
	for (size_t i = 0; i < face.nverts; i++) {
		printf("%i", face.vertecies[i]);
		if(face.has_textures)
		{
			printf("/%i", face.textures[i]);
			if(face.has_normals)
				printf("/%i", face.normals[i]);
		}
		else if (face.has_normals)
			printf("//%i", face.normals[i]);
		if(i != face.nverts-1)
			printf(" ");
	}
	printf(">\n");
}

int get_index(char * str, int nverts) {
	int res = atoi(str);
	if (res < 0) res = nverts - res;
	res --;
	return res;
}

face_t parse_face(char * facestr)
{
	face_t res;

	trim_objsyntax(&facestr);
	res = (face_t){0,0,0,NULL,NULL,NULL};
	res.has_normals = has_normals(facestr);
	res.has_textures = has_textures(facestr);
	res.nverts = 1 + nchars(facestr, ' ');

	res.vertecies = malloc(res.nverts * sizeof(i32_t));
	if (res.has_normals) res.normals   = malloc(res.nverts * sizeof(i32_t));
	if (res.has_textures) res.textures = malloc(res.nverts * sizeof(i32_t));

	for (int i = 0; i < res.nverts; i++)
	{
		res.vertecies[i] = get_index(facestr, res.nverts);

		if(res.has_textures)
		{
			facestr += index_of(facestr, '/') + 1;
			res.textures[i] = get_index(facestr, res.nverts);
			if(res.has_normals)
			{
				facestr += index_of(facestr, '/') + 1;
				res.normals[i] = get_index(facestr, res.nverts);
			}
		}
		else if(res.has_normals)
		{
			facestr += index_of(facestr, '/') + 1;
			facestr += index_of(facestr, '/') + 1;
			res.normals[i] = get_index(facestr, res.nverts);
		}
		if(i < (res.nverts-1))
			facestr += index_of(facestr, ' ') + 1;
	}
	#ifdef DEBUG
	print_face(res);
	#endif
	return res;
}



obj_t *     read_obj(char * filename)
{
	FILE  *  obj_file;
	obj_t *  object;
	char  *  buffer;
	char  ** lines;
	size_t   buffer_len;
	size_t   nlines;

		/* allocate result object */
	object = new_objptr();

		/* open the file */
	obj_file = fopen(filename, "rb");
	if (obj_file == NULL)
	{
		log_error_str("could not open file", filename);
		return NULL;
	}

		/* get size of file */
	fseek(obj_file, 0, SEEK_END);
	buffer_len = ftell(obj_file);
	fseek(obj_file, 0, SEEK_SET);

		/* read file into a buffer and close it*/
	buffer = malloc(buffer_len + 1 * sizeof(char));
	fread(buffer, buffer_len, 1, obj_file);
	fclose(obj_file);

		/* split buffer on each newline */
	lines  = malloc(buffer_len);
	nlines = split(buffer, lines, '\n');
	//free(buffer);

	/**
	 *   remove all unnecesary chars,
	 *   count vertecies, faces, normals and
	 *   textures in order to allocate arrays
	 **/
	size_t vcnt = 0;
	size_t fcnt = 0;
	size_t ncnt = 0;
	size_t tcnt = 0;
	for (size_t i = 0; i < nlines; i++)
	{
		trim_objsyntax(&lines[i]);
		switch (*lines[i])
		{
			case VERTEX_DEF: switch (*(*(lines+i)+1)) {
			case NORMAL_DEF:  ncnt++; break;
			case TEXTURE_DEF: tcnt++; break;
			default:          vcnt++; break;}break;
			case FACE_DEF:    fcnt++; break;
			default: break;
		}
		#ifdef DEBUG
		printf("line: %s\n", lines[i]); //break;
		#endif
	}
	#ifdef DEBUG
	printf("vector count: %zu\nface count: %zu\n", vcnt, fcnt);
	#endif
		/* allocate all object data arrays in object */
	object->vertecies         = malloc(vcnt * sizeof(vert_t));
	object->vertex_normals    = malloc(vcnt * sizeof(vertn_t));
	object->texture_vertecies = malloc(vcnt * sizeof(vertt_t));
	object->faces             = malloc(vcnt * sizeof(face_t));

		/* set flags for easy resetting on errors */
	object->is_complete = 1;
	object->triangulated = 1;

	for (size_t i = 0; i < nlines; i++)
	{
		if(strcmp(lines[i], ""))
		{
			switch (*lines[i])
			{
				case VERTEX_DEF: switch (*(*(lines+i)+1)) {
				case NORMAL_DEF:
					object->vertex_normals[object->nnorms]    = parse_normal((*(lines+i)+3));
					object->nnorms++;
					break;
				case TEXTURE_DEF:
					object->texture_vertecies[object->ntexts] = parse_texture((*(lines+i)+3));
					object->ntexts++;
					break;
				default:
					object->vertecies[object->nverts]         = parse_vertex((*(lines+i)+2));
					object->nverts++;
					break;
				}break;
				case FACE_DEF:
					object->faces[object->nfaces]             = parse_face((*(lines+i)+2));
					if (object->faces[object->nfaces].nverts != 3) object->triangulated = 0;
					if (object->faces[object->nfaces].nverts < 3)
					{
						log_warning_str("a face with less than 3 vertecies was detected", lines[i]);
						object->is_complete = 0;
					}
					object->nfaces++;
					break;
				case OBJNAME_DEF:
					/**
					 * TODO: implement this
					 * unimplemented feature
					 **/
					break;
				case GROUPNAME_DEF:
					/**
					 * TODO: implement this
					 * unimplemented feature
					 **/
					break;
				case SHADING_DEF:
					/**
					 * TODO: implement this
					 * unimplemented feature
					 **/
					break;
				case EMPTYLINE:break;
				default:
					if(strstr(lines[i], "usemtl") || strstr(lines[i], "mtllib"))
					{
						log_warning_str("MTL features not suported", lines[i]);
					}
					else log_warning_str("unrecognized line, result may be faulty", lines[i]);
					object->is_complete = 0;
					break;
			}
		}
	}

	/* a general rule for .obj files is that if one face have it all faces have it */
	object->has_normals = object->faces[0].has_normals;
	object->has_textures = object->faces[0].has_textures;
	return object;
}





static void log_warning(const char * msg)
{
	printf("[\033[33;1;4mWarning\033[0m]: %s\n", msg);
}
static void log_warning_str(const char * msg, char * str)
{
	printf("[\033[33;1;4mWarning\033[0m]: %s: '%s'\n", msg, str);
}
static void log_error(const char * msg)
{
	fprintf(stderr, "[\033[31;1;4mError\033[0m]: %s | %s\n",msg, strerror( errno ));
}
static void log_error_str(const char * msg, char * str)
{
	fprintf(stderr, "[\033[31;1;4mError\033[0m]: %s: '%s' | %s\n",msg, str, strerror( errno ));
}
static void log_error_char(const char * msg, char c)
{
	fprintf(stderr, "[\033[31;1;4mError\033[0m]: %s: %c\n",msg, c);
}
static void log_error_int(const char * msg, int i)
{
	fprintf(stderr, "[\033[31;1;4mError\033[0m]: %s: %i\n",msg, i);
}
