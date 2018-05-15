// Must defined this before including Viewer.h
#define IGL_VR_VIEWER_VIEWER_CPP
#include "VR_Viewer.h"

#include <algorithm>
#include <iostream>


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

//#define GLFW_INCLUDE_GLU
#if defined(__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#else
#define GL_GLEXT_PROTOTYPES
#endif

#include <GLFW/glfw3.h>


#ifdef _WIN32
#  include <windows.h>
#  undef max
#  undef min
#endif

#define FAIL(X) throw std::runtime_error(X)


#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#include <Extras/OVR_CAPI_Util.h>
#include <Extras/OVR_StereoProjection.h>
#include "Extras/OVR_Math.h"

#include <igl/PI.h>


namespace helper {
	// Convenience method for looping over each eye with a lambda
	template <typename Function>
	inline void for_each_eye(Function function) {
		for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
			eye < ovrEyeType::ovrEye_Count;
			eye = static_cast<ovrEyeType>(eye + 1)) {
			function(eye);
		}
	}
}

using namespace Eigen;
using namespace OVR;


namespace igl
{
	namespace viewer
	{
		bool isVisible;
		GLFWwindow * window;
		ovrSession session;
		ovrHmdDesc hmdDesc;
		ovrGraphicsLuid luid;
		ovrEyeRenderDesc eyeRenderDesc[2];
		ovrSizei bufferSize;
		GLuint _mirrorFbo{ 0 };
		ovrPosef pose;
		unsigned int frame{ 0 };
		ovrViewScaleDesc _viewScaleDesc;
		ovrLayerEyeFov _sceneLayer;
		ovrTextureSwapChain textureSwapChain;
		GLuint _fbo{ 0 };
		GLuint _depthBuffer{ 0 };
		ovrPosef hmdToEyeViewPose[2];
		ovrLayerEyeFov layer;
		GLuint VertexArrayID;
		GLuint vertexbuffer;
		GLuint colorbuffer;
		ovrMirrorTexture _mirrorTexture;
		GLuint programID;

		static void ErrorCallback(int error, const char* description) {
			FAIL(description);
		}



		IGL_INLINE VR_Viewer::VR_Viewer()
		{
		}

		IGL_INLINE GLuint LoadShaders(std::string VertexShaderCode, std::string FragmentShaderCode) {

			// Create the shaders
			GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
			GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

			// Read the Vertex Shader code from the file
			GLint Result = GL_FALSE;
			int InfoLogLength;

			// Compile Vertex Shader
			char const * VertexSourcePointer = VertexShaderCode.c_str();
			glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
			glCompileShader(VertexShaderID);

			// Check Vertex Shader
			glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
			glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

			// Compile Fragment Shader
			char const * FragmentSourcePointer = FragmentShaderCode.c_str();
			glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
			glCompileShader(FragmentShaderID);

			// Check Fragment Shader
			glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
			glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

			// Link the program
			printf("Linking program\n");
			GLuint ProgramID = glCreateProgram();
			glAttachShader(ProgramID, VertexShaderID);
			glAttachShader(ProgramID, FragmentShaderID);
			glLinkProgram(ProgramID);

			// Check the program
			glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
			glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

			glDetachShader(ProgramID, VertexShaderID);
			glDetachShader(ProgramID, FragmentShaderID);

			glDeleteShader(VertexShaderID);
			glDeleteShader(FragmentShaderID);


			if (Result != GL_TRUE)
			{
				char* buffer = new char[InfoLogLength];
				glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, buffer);
				std::cout << "Linker error: " << std::endl << buffer << std::endl;
				ProgramID = 0;
				delete buffer;
				return false;
			}

			return ProgramID;
		}

		IGL_INLINE void VR_Viewer::init_ovr() {
			// Create OVR Session Object
			if (!OVR_SUCCESS(ovr_Initialize(nullptr))) {
				FAIL("Failed to initialize the Oculus SDK");
			}

			//create OVR session & HMD-description
			if (!OVR_SUCCESS(ovr_Create(&session, &luid))) {
				FAIL("Unable to create HMD session");
			}

			hmdDesc = ovr_GetHmdDesc(session);
			_viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

			memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
			_sceneLayer.Header.Type = ovrLayerType_EyeFov;
			_sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

			helper::for_each_eye([&](ovrEyeType eye) {
				ovrEyeRenderDesc& erd = eyeRenderDesc[eye] = ovr_GetRenderDesc(session, eye, hmdDesc.DefaultEyeFov[eye]);
			});



			// step 2 > Size of the buffer is the width added / the maximum height
			ovrSizei recommenedTex0Size = ovr_GetFovTextureSize(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0], 1.0f);
			ovrSizei recommenedTex1Size = ovr_GetFovTextureSize(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1], 1.0f);
			bufferSize.w = recommenedTex0Size.w + recommenedTex1Size.w;
			bufferSize.h = std::max(recommenedTex0Size.h, recommenedTex1Size.h);



			// texture swap chain description 
			ovrTextureSwapChainDesc texDesc = {};
			texDesc.Type = ovrTexture_2D;
			texDesc.ArraySize = 1;
			texDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
			texDesc.Width = bufferSize.w;
			texDesc.Height = bufferSize.h;
			texDesc.MipLevels = 1;
			texDesc.SampleCount = 1;
			texDesc.StaticImage = ovrFalse;

			// create the texture swap chain
			ovrResult r = ovr_CreateTextureSwapChainGL(session, &texDesc, &textureSwapChain);
			if (!OVR_SUCCESS(r))
			{
				FAIL("creating the texture swap chain failed.");
			}

			int length = 0;
			if (!OVR_SUCCESS(ovr_GetTextureSwapChainLength(session, textureSwapChain, &length)) || !length) {
				FAIL("creating the texture swap chain failed, as it has no length or length is 0.");
			}


			ovrMirrorTextureDesc desc;
			memset(&desc, 0, sizeof(desc));
			desc.Width = 1200;
			desc.Height = 800;
			desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;

			// Create mirror texture and an FBO used to copy mirror texture to back buffer
			ovrResult result = ovr_CreateMirrorTextureGL(session, &desc, &_mirrorTexture);
			if (!OVR_SUCCESS(result))
			{
				ERROR("Failed to create mirror texture.");
			}

			hmdToEyeViewPose[0] = eyeRenderDesc[0].HmdToEyePose;
			hmdToEyeViewPose[1] = eyeRenderDesc[1].HmdToEyePose;

			// Initialize our single full screen Fov layer.
			layer.Header.Type = ovrLayerType_EyeFov;
			layer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;
			layer.ColorTexture[0] = textureSwapChain;
			layer.ColorTexture[1] = textureSwapChain;
			layer.Fov[0] = eyeRenderDesc[0].Fov;
			layer.Fov[1] = eyeRenderDesc[1].Fov;

			layer.Viewport[0] = Recti(0, 0, bufferSize.w / 2, bufferSize.h);
			layer.Viewport[1] = Recti(bufferSize.w / 2, 0, bufferSize.w / 2, bufferSize.h);

		}

		IGL_INLINE void VR_Viewer::init_opengl() {
			// pre-create: some GL-settings
			glfwWindowHint(GLFW_DEPTH_BITS, 16);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

			//Init GLFW
			if (!glfwInit()) {
				FAIL("Failed to initialize GLFW");
			}
			glfwSetErrorCallback(ErrorCallback);

			// create a GL-window for the mirror
			window = glfwCreateWindow(1200, 800, "glfw", nullptr, nullptr);
			if (!window) {
				FAIL("Unable to create rendering window");
			}
			glfwSetWindowPos(window, 100, 100);

			glfwSetWindowUserPointer(window, this);
			glfwMakeContextCurrent(window);

			glewExperimental = GL_TRUE;
			if (0 != glewInit()) {
				FAIL("Failed to initialize GLEW");
			}
			glGetError();


			glClearColor(0.2f, 0.2f, 0.6f, 0.0f);
			glEnable(GL_DEPTH_TEST);

			glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

		}


		IGL_INLINE void VR_Viewer::init_opengl_connection() {
			// Set up the framebuffer object
			glGenFramebuffers(1, &_fbo);
			glGenRenderbuffers(1, &_depthBuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
			glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, bufferSize.w, bufferSize.h);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glGenFramebuffers(1, &_mirrorFbo);
		}

		IGL_INLINE void VR_Viewer::init_data() {

			

			glGenVertexArrays(1, &VertexArrayID);
			glBindVertexArray(VertexArrayID);

			GLfloat g_vertex_buffer_data[] = {
				-1.0f,-1.0f,-1.0f, // triangle 1 : begin
				-1.0f,-1.0f, 1.0f,
				-1.0f, 1.0f, 1.0f, // triangle 1 : end
				1.0f, 1.0f,-1.0f, // triangle 2 : begin
				-1.0f,-1.0f,-1.0f,
				-1.0f, 1.0f,-1.0f, // triangle 2 : end
				1.0f,-1.0f, 1.0f,
				-1.0f,-1.0f,-1.0f,
				1.0f,-1.0f,-1.0f,
				1.0f, 1.0f,-1.0f,
				1.0f,-1.0f,-1.0f,
				-1.0f,-1.0f,-1.0f,
				-1.0f,-1.0f,-1.0f,
				-1.0f, 1.0f, 1.0f,
				-1.0f, 1.0f,-1.0f,
				1.0f,-1.0f, 1.0f,
				-1.0f,-1.0f, 1.0f,
				-1.0f,-1.0f,-1.0f,
				-1.0f, 1.0f, 1.0f,
				-1.0f,-1.0f, 1.0f,
				1.0f,-1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				1.0f,-1.0f,-1.0f,
				1.0f, 1.0f,-1.0f,
				1.0f,-1.0f,-1.0f,
				1.0f, 1.0f, 1.0f,
				1.0f,-1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				1.0f, 1.0f,-1.0f,
				-1.0f, 1.0f,-1.0f,
				1.0f, 1.0f, 1.0f,
				-1.0f, 1.0f,-1.0f,
				-1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				-1.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 1.0f,
				10.0f, -2.0f, 10.0f,
				-10.0f, -2.0f, 10.0f,
				10.0f, -2.0f, -10.0f,
				-10.0f, -2.0f, 10.0f,
				10.0f, -2.0f, -10.0f,
				-10.0f, -2.0f, -10.0f,
			};


			// Generate 1 buffer, put the resulting identifier in vertexbuffer
			glGenBuffers(1, &vertexbuffer);
			// The following commands will talk about our 'vertexbuffer' buffer
			glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);


			// Give our vertices to OpenGL.
			//glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

			MatrixXd V_T = data.V.transpose();
			glBufferData(GL_ARRAY_BUFFER, sizeof(double)*data.V.size(), V_T.data(), GL_DYNAMIC_DRAW);
			/*std::cout << data.V.size() << std::endl;
			std::cout << data.V << std::endl << std::endl;
			std::cout << data.F.cols() << std::endl;
			std::cout << data.F << std::endl;*/

			GLfloat g_color_buffer_data[] = {
				0.0f,  0.0f, 1.0f,
				0.0f,  1.0f,  0.0f,
				1.0f,  0.0f,  0.0f,
				0.0f,  0.0f, 1.0f,
				0.0f,  1.0f,  0.0f,
				1.0f,  0.0f,  0.0f,
				0.0f,  0.0f, 1.0f,
				0.0f,  1.0f,  0.0f,
				1.0f,  0.0f,  0.0f,
				0.0f,  0.0f, 1.0f,
				0.0f,  1.0f,  0.0f,
				1.0f,  0.0f,  0.0f,
				0.0f,  0.0f, 1.0f,
				0.0f,  1.0f,  0.0f,
				1.0f,  0.0f,  0.0f,
				0.0f,  0.0f, 1.0f,
				0.0f,  1.0f,  0.0f,
				1.0f,  0.0f,  0.0f,
				0.0f,  0.0f, 1.0f,
				0.0f,  1.0f,  0.0f,
				1.0f,  0.0f,  0.0f,
				0.0f,  0.0f, 1.0f,
				0.0f,  1.0f,  0.0f,
				1.0f,  0.0f,  0.0f,
				0.0f,  0.0f, 1.0f,
				0.0f,  1.0f,  0.0f,
				1.0f,  0.0f,  0.0f,
				0.0f,  0.0f, 1.0f,
				0.0f,  1.0f,  0.0f,
				1.0f,  0.0f,  0.0f,
				0.0f,  0.0f, 1.0f,
				0.0f,  1.0f,  0.0f,
				1.0f,  0.0f,  0.0f,
				0.0f,  0.0f, 1.0f,
				0.0f,  1.0f,  0.0f,
				1.0f,  0.0f,  0.0f,
				1.0f,  1.0f,  1.0f,
				1.0f,  1.0f,  1.0f,
				1.0f,  1.0f,  1.0f,
				1.0f,  1.0f,  1.0f,
				1.0f,  1.0f,  1.0f,
				1.0f,  1.0f,  1.0f

			};

			glGenBuffers(1, &colorbuffer);
			glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
			//glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW); 
			glBufferData(GL_ARRAY_BUFFER, sizeof(double)*data.V_material_diffuse.size(), data.V_material_ambient.data(), GL_DYNAMIC_DRAW);
			
			std::cout << data.V_material_ambient.size() << std::endl;
		}


		IGL_INLINE void VR_Viewer::init_shaders() {

			programID = LoadShaders(mesh_vertex_shader_string, mesh_fragment_shader_string);
			glUseProgram(programID);
		}



		/*IGL_INLINE void VR_Viewer::set_data() {

		}*/




		IGL_INLINE void VR_Viewer::init() {

			init_opengl();
			init_ovr();
			init_opengl_connection();
			init_data();
			init_shaders();

		}

		IGL_INLINE void VR_Viewer::draw() {

			// Get both eye poses simultaneously, with IPD offset already included.
			double displayMidpointSeconds = ovr_GetPredictedDisplayTime(session, 0);
			ovrTrackingState hmdState = ovr_GetTrackingState(session, displayMidpointSeconds, ovrTrue);
			ovr_CalcEyePoses(hmdState.HeadPose.ThePose, hmdToEyeViewPose, layer.RenderPose);

			// Get next available index of the texture swap chain
			int currentIndex;
			ovr_GetTextureSwapChainCurrentIndex(session, textureSwapChain, &currentIndex);

			GLuint curTexId;
			ovr_GetTextureSwapChainBufferGL(session, textureSwapChain, currentIndex, &curTexId);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDepthRange(0.01, 1000.0);


			for (int eye = 0; eye < 2; eye++)
			{

				auto& vp = layer.Viewport[eye];
				glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
				OVR::Matrix4f model = OVR::Matrix4f::Translation(-OVR::Vector3f(layer.RenderPose[eye].Position) + OVR::Vector3f(0.0f, 0, -2.0f));
				OVR::Matrix4f view = OVR::Matrix4f(OVR::Quatf(layer.RenderPose[eye].Orientation).Inverted());
				OVR::Matrix4f proj = ovrMatrix4f_Projection(eyeRenderDesc[eye].Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
				OVR::Matrix4f mvp_ovr = proj * view * model;
				GLfloat mvps[16];
				memcpy(&mvps[0], &mvp_ovr.Transposed().M[0][0], sizeof(GLfloat) * 16);

				GLint mvpi = glGetUniformLocation(programID, "mvp");
				glUniformMatrix4fv(mvpi, 1, GL_FALSE, mvps);
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
				glVertexAttribPointer(
					0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
					3,                  // size
					GL_DOUBLE,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
				);

				
				

				MatrixXi F_T = data.F.transpose();

				glDrawElements(GL_TRIANGLES, data.F.size(), GL_UNSIGNED_INT, F_T.data());
				glDisableVertexAttribArray(0);
				// 2nd attribute buffer : colors
				
				glEnableVertexAttribArray(1);
				glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
				glVertexAttribPointer(
					1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
					4,                                // size
					GL_DOUBLE,                         // type
					GL_FALSE,                         // normalized?
					0,                                // stride
					(void*)0                          // array buffer offset
				);
			}

			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

			// Commit the changes to the texture swap chain
			ovr_CommitTextureSwapChain(session, textureSwapChain);

			// Submit frame with one layer we have.
			ovrLayerHeader* layers = &layer.Header;
			ovr_SubmitFrame(session, frame, &_viewScaleDesc, &layers, 1);

			// Configure the mirror read buffer
			GLuint mirrorTextureId;
			ovr_GetMirrorTextureBufferGL(session, _mirrorTexture, &mirrorTextureId);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
			glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
			glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
			glBlitFramebuffer(0, 0, 1200, 800, 0, 800, 1200, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

			// Swap buffers
			glfwSwapBuffers(window);
			glfwPollEvents();

		}

		IGL_INLINE void VR_Viewer::launch() {
			while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0) {
				draw();
			}
		}
	}
}