#include "subtitans.h"
#include "surface.h"
#include "openglrenderer.h"

#include "glew/include/GL/glew.h"
#include "glew/include/GL/wglew.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"

OpenGLRenderer::OpenGLRenderer()
{
}

OpenGLRenderer::~OpenGLRenderer()
{

}

bool SetPixelFormat(HDC deviceContext)
{
	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset(&pixelFormatDescriptor, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 32;
	pixelFormatDescriptor.cAlphaBits = 0;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cStencilBits = 8;
	pixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE;

	int32_t pixelFormat = ChoosePixelFormat(deviceContext, &pixelFormatDescriptor);
	if (!pixelFormat)
		return false;

	if (!SetPixelFormat(deviceContext, pixelFormat, &pixelFormatDescriptor))
		return false;

	return true;
}

bool IsOpenGL3Compatible(HDC deviceContext)
{
	if (!SetPixelFormat(deviceContext))
		return false;

	HGLRC openGLContext = wglCreateContext(deviceContext);
	if (!wglMakeCurrent(deviceContext, openGLContext))
	{
		wglDeleteContext(openGLContext);
		return false;
	}

	GetLogger()->Informational("%s %s %s %s\n", __FUNCTION__, "GPU:", glGetString(GL_VENDOR), glGetString(GL_RENDERER));

	bool result = glewInit() == GLEW_OK && GLEW_VERSION_3_3;
	if (!result)
		GetLogger()->Error("%s %s\n", __FUNCTION__, "this system does not support OpenGL 3.3");

	if (!wglewIsSupported("WGL_ARB_create_context"))
	{
		GetLogger()->Error("%s %s\n", __FUNCTION__, "no support for WGL_ARB_create_context");
		result = false;
	}

	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(openGLContext);
	return result;
}

bool OpenGLRenderer::Create()
{
	if (!Global::GameWindow)
	{
		GetLogger()->Error("%s %s\n", __FUNCTION__, "GameWindow hasn't been set");
		return false;
	}

	if (RenderThread)
	{
		GetLogger()->Warning("%s %s\n", __FUNCTION__, "already created");
		return false;
	}

	HDC deviceContext = GetDC(nullptr);
	bool isCompatible = IsOpenGL3Compatible(deviceContext);
	ReleaseDC(nullptr, deviceContext);

	if (isCompatible)
	{
		IsThreadRunning = true;
		RenderThread = new std::thread(&OpenGLRenderer::Run, this);
	}

	return isCompatible;
}

void OpenGLRenderer::OnCreatePrimarySurface(Surface* primarySurface)
{
	Mutex.lock();
	Global::RenderInformation oldInformation;
	memcpy(&oldInformation, &RenderInformation, sizeof(Global::RenderInformation));
	RenderInformation = Global::GetRenderInformation();

	if(memcmp(&oldInformation, &RenderInformation, sizeof(Global::RenderInformation)) != 0)
		RecalculateSurface = true;

	PrimarySurface = primarySurface;
	Mutex.unlock();
}

void OpenGLRenderer::OnDestroyPrimarySurface()
{
	Mutex.lock();
	PrimarySurface = nullptr;
	Mutex.unlock();
}

const char* vertexShader = "#version 330 core\n	\
layout (location = 0) in vec3 pos;				\
layout (location = 1) in vec2 uv;				\
												\
out vec2 surfaceUv;								\
												\
void main()										\
{												\
	gl_Position.xyz = pos;						\
	gl_Position.w = 1.0;						\
												\
	surfaceUv = uv;								\
}												\
";

const char* fragmentShader = "#version 330 core\n		\
uniform usampler2D surface;								\
uniform sampler2D palette;								\
														\
in vec2 surfaceUv;										\
out vec4 renderColor;									\
														\
void main()												\
{														\
	float idx = float(texture(surface, surfaceUv).r);	\
	vec2 uv = (vec2(idx, 0.f) + 0.5f) / 256.f;			\
	renderColor = vec4(texture(palette, uv).rgb, 1.f);	\
}														\
";

const char* fragmentShaderRetro = "#version 330 core\n															\
uniform usampler2D surface;																						\
uniform sampler2D palette;																						\
																												\
in vec2 surfaceUv;																								\
out vec4 renderColor;																							\
																												\
vec3 getDestColor(float x, float y)																				\
{																												\
	float idx = texelFetch(surface, ivec2(x, y), 0).r;															\
	vec2 uv = (vec2(idx, 0.f) + 0.5f) / 256.f;																	\
	return texture(palette, uv).rgb;																			\
}																												\
																												\
vec3 getMostIntense(vec3 left, vec3 right)																		\
{																												\
	if(left.r + left.g + left.b >= right.r + right.g + right.b)													\
	{																											\
		return left;																							\
	}																											\
																												\
	return right;																								\
}																												\
																												\
void main()																										\
{																												\
	float idx = float(texture(surface, surfaceUv).r);															\
	vec2 uv = (vec2(idx, 0.f) + 0.5f) / 256.f;																	\
	renderColor = vec4(texture(palette, uv).rgb, 1.f);															\
																												\
	vec2 size = textureSize(surface, 0);																		\
	float x = surfaceUv.x * size.x;																				\
	float y = surfaceUv.y * size.y;																				\
																												\
	/* Color blending based on intensity (+ shape) */															\
	/* Top */																									\
	float pos = y;																								\
	if(pos - 1 >= 0) { pos -= 1;}																				\
	vec3 topColor = getMostIntense(renderColor.rgb, getDestColor(x, pos));										\
																												\
	/* Left */																									\
	pos = x;																									\
	if(pos - 1 >= 0) { pos -= 1;}																				\
	vec3 leftColor = getMostIntense(renderColor.rgb, getDestColor(pos, y));										\
																												\
	/* Bottom */																								\
	pos = y;																									\
	if(pos + 1 < size.y) { pos += 1; }																			\
	vec3 bottomColor = getMostIntense(renderColor.rgb, getDestColor(x, pos));									\
																												\
	/* Right */																									\
	pos = x;																									\
	if(pos + 1 < size.x) { pos += 1; }																			\
	vec3 rightColor = getMostIntense(renderColor.rgb, getDestColor(pos, y));									\
																												\
	renderColor.rgb = (renderColor.rgb * 2 + topColor + leftColor + bottomColor + rightColor) / 6.f;			\
																												\
	/* Scan line approximation */																				\
	if(mod(floor(gl_FragCoord.y), 2) != 0)																		\
	{																											\
		float multiplier = 0.7f;																				\
																												\
		if(topColor.r + topColor.g + topColor.b > 2.f)															\
		{																										\
			multiplier += (topColor.r + topColor.g + topColor.b - 2.f) * 0.1f;									\
		}																										\
																												\
		if(bottomColor.r + bottomColor.g + bottomColor.b > 2)													\
		{																										\
			multiplier += (bottomColor.r + bottomColor.g + bottomColor.b - 2.f) * 0.1f;							\
		}																										\
																												\
		renderColor.rgb *= multiplier;																			\
	}																											\
}																												\
";

constexpr float _squareVertexData[]{
	// POS
	-1.f, -1.f, 0.f,
	-1.f,  1.f, 0.f,
	 1.f,  1.f, 0.f,
	 1.f, -1.f, 0.f,
	// UV
	0.f, 1.f,
	0.f, 0.f,
	1.f, 0.f,
	1.f, 1.f
};

HGLRC CreateOpenGLContext(HDC deviceContext)
{
	if (!SetPixelFormat(deviceContext))
		return nullptr;

	HGLRC legacyContext = wglCreateContext(deviceContext);
	if (!wglMakeCurrent(deviceContext, legacyContext))
	{
		wglDeleteContext(legacyContext);
		return nullptr;
	}

	if(glewInit() != GLEW_OK)
	{
		wglDeleteContext(legacyContext);
		return nullptr;
	}
	
	// Create modern context
	const int attributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		0,
	};

	HGLRC context = wglCreateContextAttribsARB(deviceContext, nullptr, attributes);
	if (!context)
	{
		wglDeleteContext(legacyContext);
		return nullptr;
	}

	if (!wglMakeCurrent(deviceContext, context))
	{
		wglDeleteContext(context);
		wglDeleteContext(legacyContext);
		return nullptr;
	}

	wglDeleteContext(legacyContext);
	return context;
}


uint32_t CreateShader()
{
	uint32_t vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderId, 1, &vertexShader, NULL);
	glCompileShader(vertexShaderId);
	int32_t compileSucceeded = 0;
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &compileSucceeded);
	if (!compileSucceeded)
	{
		GetLogger()->Error("%s %s\n", __FUNCTION__, "failed to compile vertex shader");

		char errorBuffer[1024 - 16];
		glGetShaderInfoLog(vertexShaderId, sizeof(errorBuffer), NULL, errorBuffer);
		GetLogger()->Error(errorBuffer);

		return 0;
	}

	uint32_t fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderId, 1, Global::RetroShader ? &fragmentShaderRetro : &fragmentShader, NULL);
	glCompileShader(fragmentShaderId);
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &compileSucceeded);
	if (!compileSucceeded)
	{
		GetLogger()->Error("%s %s\n", __FUNCTION__, "failed to compile fragment shader");

		char errorBuffer[1024 - 16];
		glGetShaderInfoLog(fragmentShaderId, sizeof(errorBuffer), NULL, errorBuffer);
		GetLogger()->Error(errorBuffer);

		return 0;
	}

	uint32_t shaderProgramId = glCreateProgram();
	glAttachShader(shaderProgramId, vertexShaderId);
	glAttachShader(shaderProgramId, fragmentShaderId);
	glLinkProgram(shaderProgramId);
	glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &compileSucceeded);
	if (!compileSucceeded)
	{
		GetLogger()->Error("%s %s\n", __FUNCTION__, "failed to link shader program");

		char errorBuffer[1024 - 16];
		glGetProgramInfoLog(shaderProgramId, sizeof(errorBuffer), NULL, errorBuffer);
		GetLogger()->Error(errorBuffer);

		return 0;
	}
	
	glDetachShader(shaderProgramId, vertexShaderId);
	glDetachShader(shaderProgramId, fragmentShaderId);

	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

	GetLogger()->Informational("%s %s\n", __FUNCTION__, "shader has been compiled");

	return shaderProgramId;
}

void InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplOpenGL3_Init("#version 330 core");
	
	auto io = &ImGui::GetIO();
	io->DisplaySize.x = static_cast<float>(Global::MonitorWidth);
	io->DisplaySize.y = static_cast<float>(Global::MonitorHeight);
}

void OpenGLRenderer::Run()
{
	HDC deviceContext = GetDC(nullptr);
	HGLRC openGLContext = CreateOpenGLContext(deviceContext);
	if (openGLContext == nullptr)
	{
		GetLogger()->Error("%s %s\n", __FUNCTION__, "failed to create OpenGL context");

		ReleaseDC(nullptr, deviceContext);
		return;
	}

	uint32_t shaderProgramId = CreateShader();
	if (shaderProgramId == 0)
	{
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(openGLContext);
		ReleaseDC(nullptr, deviceContext);
		return;
	}

	glUseProgram(shaderProgramId);

	/* Create images */
	uint32_t surfaceTextureId = 0;
	uint32_t paletteTextureId = 0;

	glUniform1i(glGetUniformLocation(shaderProgramId, "surface"), 0);
	glUniform1i(glGetUniformLocation(shaderProgramId, "palette"), 1);

	glGenTextures(1, &surfaceTextureId);
	glGenTextures(1, &paletteTextureId);

	glViewport(0, 0, Global::MonitorWidth, Global::MonitorHeight);

	uint32_t vboId = 0;
	uint32_t vertexBufferId = 0;
	glGenVertexArrays(1, &vboId);
	glBindVertexArray(vboId);

	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_squareVertexData), _squareVertexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GetLogger()->Informational("%s %s\n", __FUNCTION__, "is running");

	InitImGui();

	Mutex.lock();
	while (IsThreadRunning)
	{
		Mutex.unlock();

		// Render if event has been dispatched OR if the rendering will drop under 25fps as some screens like the loading screen do not dispatch the event
		WaitForSingleObject(Global::RenderEvent, 40);

		Mutex.lock();
		
		// Not initialized
		if (PrimarySurface == nullptr)
			continue;
		
		// Stop rendering if the game isn't being focussed on
		if (GetForegroundWindow() != Global::GameWindow)
			continue;

		// Prevent flickering caused by palette changes
		if (PrimarySurface->PrimaryInvalid)
			continue;

		// Can't render without palette
		if (!PrimarySurface->AttachedPalette)
			continue; 

		// Clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set images
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, surfaceTextureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		PrimarySurface->PrimaryDrawMutex.lock();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, PrimarySurface->Stride, PrimarySurface->Height, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, PrimarySurface->SurfaceBuffer);
		PrimarySurface->PrimaryDrawMutex.unlock();

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, paletteTextureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		PrimarySurface->AttachedPalette->Mutex.lock();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, PrimarySurface->AttachedPalette->RawPalette);
		PrimarySurface->AttachedPalette->Mutex.unlock();

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(12 * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
		if (RecalculateSurface)
		{
			float* recalculatedSurface = new float[sizeof(_squareVertexData)/sizeof(float)];
			memcpy(recalculatedSurface, _squareVertexData, sizeof(_squareVertexData));
		
			if (RenderInformation.Padding != 0)
			{
				recalculatedSurface[0] += (float)RenderInformation.Padding * 2 / RenderInformation.MonitorWidth;
				recalculatedSurface[3] += (float)RenderInformation.Padding * 2 / RenderInformation.MonitorWidth;
				recalculatedSurface[6] -= (float)RenderInformation.Padding * 2 / RenderInformation.MonitorWidth;
				recalculatedSurface[9] -= (float)RenderInformation.Padding * 2 / RenderInformation.MonitorWidth;
			}

			glBufferData(GL_ARRAY_BUFFER, sizeof(_squareVertexData), recalculatedSurface, GL_STATIC_DRAW);
			delete[] recalculatedSurface;
			RecalculateSurface = false;
		}
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		if (Global::ImGuiEnabled)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();

			ImGui::ShowDemoWindow();
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
		
		// Swap
		SwapBuffers(deviceContext);

		SetEvent(Global::VerticalBlankEvent);
	}
	Mutex.unlock();

	// Cleanup
	GetLogger()->Informational("%s %s\n", __FUNCTION__, "is cleaning up");

	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
	
	glDeleteBuffers(1, &vertexBufferId);
	glDeleteVertexArrays(1, &vboId);

	glDeleteProgram(shaderProgramId);
	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(openGLContext);
	ReleaseDC(nullptr, deviceContext);

	GetLogger()->Informational("%s %s\n", __FUNCTION__, "has been cleaned up");
}