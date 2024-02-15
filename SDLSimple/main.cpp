//    Author: Nabira Ahmad
//    Assignment: Simple 2D Scene
//    Date due: 2024-02-17, 11:59pm
//    I pledge that I have completed this assignment without
//    collaborating with anyone else, in conformance with the
//    NYU School of Engineering Policies and Procedures on
//    Academic Misconduct.
 
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

const int WINDOW_WIDTH  = 640*1.5,
          WINDOW_HEIGHT = 480*1.5;

const float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X      = 0,
          VIEWPORT_Y      = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// add sprite images
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl",
           SOPHIE_PATH[]   = "witch.png",
           CAT_PATH[]   = "cat2.png";

const int TRIANGLE_RED     = 1.0,
          TRIANGLE_BLUE    = 0.4,
          TRIANGLE_GREEN   = 0.4,
          TRIANGLE_OPACITY = 1.0;

const float MILLISECONDS_IN_SECOND = 2000.0;
const float DEGREES_PER_SECOND = 90.0f;

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL  = 0; // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER   = 0; // this value MUST be zero

SDL_Window* g_display_window;

bool g_game_is_running = true;

ShaderProgram g_program;

// create two model matrices, 1 and 2
glm::mat4 g_view_matrix,
          g_model_matrix_witch,
          g_model_matrix_cat,
          g_projection_matrix,
          g_trans_matrix;

float g_triangle_rotate = 0.0f;
float g_previous_ticks  = 0.0f;

// create texture id's
GLuint g_witch_texture_id;
GLuint g_cat_texture_id;

// load_texture function used from lecture content
GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        LOG(filepath);
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint texture_id;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    
        // changed texture filter to linear for high resolution pictures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return texture_id;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    // changed the window name
    g_display_window = SDL_CreateWindow("2D SIMPLE SCENE- WITCH AND CAT :-)",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_witch_texture_id = load_texture(SOPHIE_PATH);
    g_cat_texture_id = load_texture(CAT_PATH);
    
    g_view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.
    g_model_matrix_witch = glm::mat4(1.0f);
    g_model_matrix_cat = glm::mat4(1.0f);
    
    
    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);
    
    g_program.set_colour(TRIANGLE_RED, TRIANGLE_BLUE, TRIANGLE_GREEN, TRIANGLE_OPACITY);
    
    glUseProgram(g_program.get_program_id());
    
    // call blend in case of texture overlap
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_game_is_running = false;
        }
    }
}


void update()
    {
        // delta time formula
        float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
        float delta_time = ticks - g_previous_ticks;
        g_previous_ticks = ticks;

        // speed for rotations
        float angular_speed = DEGREES_PER_SECOND * delta_time;
        g_triangle_rotate += angular_speed; // 90 degrees per second

        // angles for circular translation
        // the two sprites are 180 deg. from each other
        float position1 = glm::radians(g_triangle_rotate);
        float position2 = glm::radians(g_triangle_rotate + 180.0f);
        float radius = 1.5f;

        // circle coordinates using trigonometric functions
        float x_coordinate_1 = radius * cos(position1);
        float y_coordinate_1 = radius * sin(position1);

        float x_coordinate_2 = radius * cos(position2);
        float y_coordinate_2 = radius * sin(position2);

        // reset the model matrices to the identity matrix EVERY FRAME
        g_model_matrix_witch = glm::mat4(1.0f);
        g_model_matrix_cat = glm::mat4(1.0f);

        // update scale factor
        static float scale_factor = 1.0f;
        static bool growing = true;

        // use delta time in scaling
        if (growing) {
            scale_factor += delta_time;
            if (scale_factor >= 2.5f) {
                growing = false;
            }
        } else {
            scale_factor -= delta_time;
            if (scale_factor <= 1.5f) {
                growing = true;
            }
        }
    
        // scaling the cat, which in turn will scale the witch since the witch's translations are relative to the cat
        g_model_matrix_cat = glm::scale(g_model_matrix_cat, glm::vec3(scale_factor, scale_factor, 0.0f));

        // translate -> Rotate g_model_matrix_witch
        g_model_matrix_witch = glm::translate(g_model_matrix_cat, glm::vec3(x_coordinate_1, y_coordinate_1, 0.0f));
        g_model_matrix_witch = glm::rotate(g_model_matrix_witch, glm::radians(g_triangle_rotate), glm::vec3(0.0f, 0.0f, 1.0f));

        // translate -> Rotate g_model_matrix_cat
        g_model_matrix_cat = glm::translate(g_model_matrix_cat, glm::vec3(x_coordinate_2, y_coordinate_2, 0.0f));
    
        // wanted to rotate the cat in a different direction from the witch
        g_model_matrix_cat = glm::rotate(g_model_matrix_cat, glm::radians(-g_triangle_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
}

// used lecture suggestion to have a draw_object function and call it in render
void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    // keep track of time for color pattern
    float time = SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    
    // color array to alternate in a pattern
    float colors[][3] = {
        {0.4f, 0.0f, 0.9f},  // purplish
        {0.5f, 0.0f, 0.5f},  // magenta
        {0.0f, 0.5f, 0.0f},  // dark Green
        {0.0f, 0.0f, 0.0f}   // black
    };

    // calculate color index based on time
    // alternate between four colors every two seconds
    int color_pick = (int(time) / 2) % 4;

    // pick the color from the array
    glClearColor(colors[color_pick][0], colors[color_pick][1], colors[color_pick][2], 1.0f);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
    
    // vertices
    float vertices[] =
    {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };
    
    glVertexAttribPointer(g_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_program.get_position_attribute());

    // texture coordinates
    float texture_coordinates[] =
    {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(g_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_program.get_tex_coordinate_attribute());
    
    // call draw_object twice for two objects
    draw_object(g_model_matrix_witch, g_witch_texture_id);
    draw_object(g_model_matrix_cat, g_cat_texture_id);
    
    
    glDisableVertexAttribArray(g_program.get_position_attribute());
    glDisableVertexAttribArray(g_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


    // Start here—we can see the general structure of a game loop without worrying too much about the details yet.

int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
