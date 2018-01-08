#ifndef IGL_VR_VIEWER_VIEWER_H
#define IGL_VR_VIEWER_VIEWER_H
#endif


#ifndef __APPLE__
#  define GLEW_STATIC
#  include <GL/glew.h>
#endif

#ifdef __APPLE__
#   include <OpenGL/gl3.h>
#   define __gl_h_ /* Prevent inclusion of the old gl.h */
#else
#   include <GL/gl.h>
#endif

#include <igl/igl_inline.h>
#include <Eigen/Geometry>
#include "ViewerData.h"



namespace igl
{
	namespace viewer
	{
		class VR_Viewer
		{
		public:

			ViewerData data;

			/*std::string mesh_vertex_shader_string =
				"#version 150\n"
				"uniform mat4 model;"
				"uniform mat4 view;"
				"uniform mat4 proj;"
				"in vec3 position;"
				"in vec3 normal;"
				"out vec3 position_eye;"
				"out vec3 normal_eye;"
				"in vec4 Ka;"
				"in vec4 Kd;"
				"in vec4 Ks;"
				"in vec2 texcoord;"
				"out vec2 texcoordi;"
				"out vec4 Kai;"
				"out vec4 Kdi;"
				"out vec4 Ksi;"

				"void main()"
				"{"
				"  position_eye = vec3 (view * model * vec4 (position, 1.0));"
				"  normal_eye = vec3 (view * model * vec4 (normal, 0.0));"
				"  normal_eye = normalize(normal_eye);"
				"  gl_Position = proj * vec4 (position_eye, 1.0);" //proj * view * model * vec4(position, 1.0);"
				"  Kai = Ka;"
				"  Kdi = Kd;"
				"  Ksi = Ks;"
				"  texcoordi = texcoord;"
				"}";


			std::string mesh_fragment_shader_string =
				"#version 150\n"
				"uniform mat4 model;"
				"uniform mat4 view;"
				"uniform mat4 proj;"
				"uniform vec4 fixed_color;"
				"in vec3 position_eye;"
				"in vec3 normal_eye;"
				"uniform vec3 light_position_world;"
				"vec3 Ls = vec3 (1, 1, 1);"
				"vec3 Ld = vec3 (1, 1, 1);"
				"vec3 La = vec3 (1, 1, 1);"
				"in vec4 Ksi;"
				"in vec4 Kdi;"
				"in vec4 Kai;"
				"in vec2 texcoordi;"
				"uniform sampler2D tex;"
				"uniform float specular_exponent;"
				"uniform float lighting_factor;"
				"uniform float texture_factor;"
				"out vec4 outColor;"
				"void main()"
				"{"
				"vec3 Ia = La * vec3(Kai);"    // ambient intensity

				"vec3 light_position_eye = vec3 (view * vec4 (light_position_world, 1.0));"
				"vec3 vector_to_light_eye = light_position_eye - position_eye;"
				"vec3 direction_to_light_eye = normalize (vector_to_light_eye);"
				"float dot_prod = dot (direction_to_light_eye, normal_eye);"
				"float clamped_dot_prod = max (dot_prod, 0.0);"
				"vec3 Id = Ld * vec3(Kdi) * clamped_dot_prod;"    // Diffuse intensity

				"vec3 reflection_eye = reflect (-direction_to_light_eye, normal_eye);"
				"vec3 surface_to_viewer_eye = normalize (-position_eye);"
				"float dot_prod_specular = dot (reflection_eye, surface_to_viewer_eye);"
				"dot_prod_specular = float(abs(dot_prod)==dot_prod) * max (dot_prod_specular, 0.0);"
				"float specular_factor = pow (dot_prod_specular, specular_exponent);"
				"vec3 Is = Ls * vec3(Ksi) * specular_factor;"    // specular intensity
				"vec4 color = vec4(lighting_factor * (Is + Id) + Ia + (1.0-lighting_factor) * vec3(Kdi),(Kai.a+Ksi.a+Kdi.a)/3);"
				"outColor = mix(vec4(1,1,1,1), texture(tex, texcoordi), texture_factor) * color;"
				"if (fixed_color != vec4(0.0)) outColor = fixed_color;"
				"}";
				*/
			std::string mesh_vertex_shader_string =
				"#version 330 core\n"
				"layout(location = 0) in vec4 Position;"
				"layout(location = 1) in vec3 vertexColor;"
				"uniform mat4 mvp;"
				"out vec3 fragmentColor;"
				"void main(){"
				"  gl_Position = (mvp * Position); "
				"  fragmentColor = vertexColor;"
				"}";

			std::string mesh_fragment_shader_string =

				"#version 330 core\n"
				"in vec3 fragmentColor;"
				"out vec3 color;"
				"void main() {"
				"color = fragmentColor;"
				//"color = vec3(1,0,0);"
				"}";

		

			IGL_INLINE void init_ovr();
			IGL_INLINE void init_opengl();
			IGL_INLINE void init_opengl_connection();
			IGL_INLINE void init_data();
			IGL_INLINE void init_shaders();
			IGL_INLINE void init();
			IGL_INLINE void draw();
			IGL_INLINE void set_data(const igl::viewer::ViewerData & data, bool invert_normals);
			IGL_INLINE GLint bindVertexAttribArray(const std::string & name, GLuint bufferID, const Eigen::MatrixXf & M, bool refresh) const;
			IGL_INLINE void bind_mesh();
			IGL_INLINE void launch();
			VR_Viewer();
			

		};
	}
}