#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <GLES3/gl3.h>

const char* vertexShaderSource =
    "#version 300 es\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 vColor;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "    vColor = aColor;\n"
    "}\0";

const char* fragmentShaderSource =
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec3 vColor;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vec4(vColor, 1.0);\n"
    "}\0";

struct AppState {
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    GLuint shaderProgram = 0;
    GLuint VAO = 0;
    GLuint VBO = 0;
};

static GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        SDL_Log("Shader compilation error: %s", infoLog);
    }
    return shader;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Configure OpenGL ES 3.0 / WebGL 2.0 context attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    auto* state = new AppState();
    *appstate = state;

    state->window = SDL_CreateWindow(
        "SDL3 WebGL 2.0 (WebAssembly)",
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!state->window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->glContext = SDL_GL_CreateContext(state->window);
    if (!state->glContext) {
        SDL_Log("Failed to create OpenGL context: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GL_SetSwapInterval(1);

    // Build and compile shader program
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    state->shaderProgram = glCreateProgram();
    glAttachShader(state->shaderProgram, vertexShader);
    glAttachShader(state->shaderProgram, fragmentShader);
    glLinkProgram(state->shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {
        // positions    // colors
         0.0f,  0.5f,   1.0f, 0.0f, 0.0f, // top (red)
        -0.5f, -0.5f,   0.0f, 1.0f, 0.0f, // bottom left (green)
         0.5f, -0.5f,   0.0f, 0.0f, 1.0f  // bottom right (blue)
    };

    glGenVertexArrays(1, &state->VAO);
    glGenBuffers(1, &state->VBO);

    glBindVertexArray(state->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, state->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute (layout location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute (layout location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    auto* state = static_cast<AppState*>(appstate);

    int w, h;
    SDL_GetWindowSizeInPixels(state->window, &w, &h);
    glViewport(0, 0, w, h);

    glClearColor(0.12f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(state->shaderProgram);
    glBindVertexArray(state->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(state->window);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    if (appstate) {
        auto* state = static_cast<AppState*>(appstate);

        glDeleteVertexArrays(1, &state->VAO);
        glDeleteBuffers(1, &state->VBO);
        glDeleteProgram(state->shaderProgram);

        if (state->glContext) {
            SDL_GL_DestroyContext(state->glContext);
        }
        if (state->window) {
            SDL_DestroyWindow(state->window);
        }
        delete state;
    }
    SDL_Quit();
}