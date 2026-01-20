#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <sys/types.h>
#include <dirent.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include "nuklear.h"

#define NK_SDL_GL2_IMPLEMENTATION

#include "nuklear_sdl_gl2.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

struct Animation
{
    int animId;
    char name[255];
    int fileIndex;
};

struct Animation *parseAIndex(char *path, int *out_count)
{
    printf("Opening: %s\n", path);

    FILE *f = fopen(path, "rb");
    if (f == 0)
    {
        perror("Failed to open .aindex file!");
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    rewind(f);

    int count = fileSize / 259;
    *out_count = count;

    struct Animation *list = malloc(sizeof(struct Animation) * count);

    for (int i = 0; i < count; i++)
    {
        char buffer[255];
        fread(buffer, 255, 1, f);
        uint32_t id;
        fread(&id, 4, 1, f);

        strcpy(list[i].name, buffer);
        list[i].animId = id;
        list[i].fileIndex = i;
    }

    fclose(f);
    return list;
}

void saveAIndex(char *path, struct Animation* list, int count2) {
    
}

void remove_animation(struct Animation **list, int *count, int index) {
    if (*count <= 0 || index < 0 || index >= *count) return;

    if (index < *count - 1) {
        memmove(&(*list)[index], &(*list)[index + 1], sizeof(struct Animation) * (*count - index - 1));
    }
    
    (*count)--;

    *list = realloc(*list, sizeof(struct Animation) * (*count));
}

struct Animation *anim_list_trip = NULL;
struct Animation *anim_list_grace = NULL;

int anim_list_trip_count = 0;
int anim_list_grace_count = 0;

int main(int argc, char *argv[])
{
    SDL_Window *win;
    SDL_GLContext glContext;

    char facade_path[512] = "/home/noia/.wine/drive_c/Program Files (x86)/Facade/util/sources/facade";

    int path_len = strlen(facade_path);

    int running = 1;

    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    win = SDL_CreateWindow("Facade Animation Tool", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    glContext = SDL_GL_CreateContext(win);

    struct nk_context *ctx;
    ctx = nk_sdl_init(win);

    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();

    char selected_file[1024] = {0};

    while (running)
    {
        SDL_Event evt;

        nk_input_begin(ctx);

        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_QUIT)
            {
                running = 0;
            }

            nk_sdl_handle_event(&evt);
        }

        nk_input_end(ctx);

        if (nk_begin(ctx, "Animation Editor", nk_rect(50, 50, 300, 250), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
        {

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "Facade Base Path:", NK_TEXT_LEFT);
            nk_edit_string(ctx, NK_EDIT_FIELD, facade_path, &path_len, 512, nk_filter_default);

            if (nk_button_label(ctx, "Load Animations"))
            {
                if (anim_list_grace != NULL)
                {
                    free(anim_list_grace);
                }

                if (anim_list_trip != NULL)
                {
                    free(anim_list_trip);
                }

                char full_path_trip[1024];
                char full_path_grace[1024];

                snprintf(full_path_grace, sizeof(full_path_grace), "%s/animation/grace/grace.aindex", facade_path);
                snprintf(full_path_trip, sizeof(full_path_trip), "%s/animation/trip/trip.aindex", facade_path);

                anim_list_trip = parseAIndex(full_path_trip, &anim_list_trip_count);
                anim_list_grace = parseAIndex(full_path_grace, &anim_list_grace_count);
            }

            if (anim_list_grace_count > 0 && anim_list_trip_count > 0)
            {
                char buf[64], buf2[64];
                snprintf(buf, sizeof(buf), "Trip animations: %d", anim_list_trip_count);
                nk_label(ctx, buf, NK_TEXT_LEFT);
                snprintf(buf2, sizeof(buf2), "Grace animations: %d", anim_list_grace_count);
                nk_label(ctx, buf2, NK_TEXT_ALIGN_LEFT);
            }
        }

        nk_end(ctx);

        if (anim_list_grace_count > 0 && anim_list_trip_count > 0)
        {
            if (nk_begin(ctx, "trip.aindex", nk_rect(400, 50, 500, 600), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE)) {
                nk_layout_row_dynamic(ctx, 30, 1);

                if (nk_button_label(ctx, "SAVE CHANGES TO TRIP.AINDEX")) {
                    char full_path[1024];
                    snprintf(full_path, sizeof(full_path), "%s/animation/trip/trip.aindex", facade_path);
                    saveAIndex(full_path, anim_list_trip, anim_list_trip_count);
                }

                nk_layout_row_template_begin(ctx, 20);
                nk_layout_row_template_push_static(ctx, 40);  // INDX
                nk_layout_row_template_push_static(ctx, 60);  // ID
                nk_layout_row_template_push_dynamic(ctx);     // NAME
                nk_layout_row_template_push_static(ctx, 80);  // ACTION
                nk_layout_row_template_end(ctx);
                nk_label(ctx, "INDX", NK_TEXT_LEFT);
                nk_label(ctx, "ID", NK_TEXT_LEFT);
                nk_label(ctx, "NAME", NK_TEXT_LEFT);
                nk_label(ctx, "", NK_TEXT_LEFT);

                for (int j = 0; j < anim_list_trip_count; j++) {
                    nk_layout_row_template_begin(ctx, 30);
                    nk_layout_row_template_push_static(ctx, 40);
                    nk_layout_row_template_push_static(ctx, 60);
                    nk_layout_row_template_push_dynamic(ctx);
                    nk_layout_row_template_push_static(ctx, 80);
                    nk_layout_row_template_end(ctx);

                    nk_labelf(ctx, NK_TEXT_LEFT, "%d", j);
                    nk_labelf(ctx, NK_TEXT_LEFT, "[%d]", anim_list_trip[j].animId);
                    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, anim_list_trip[j].name, 255, nk_filter_default);

                    if (nk_button_label(ctx, "Remove")) {
                        remove_animation(&anim_list_trip, &anim_list_trip_count, j);
                        break; 
                    }
                }
            }
            nk_end(ctx);

            if (nk_begin(ctx, "grace.aindex", nk_rect(1000, 50, 500, 600), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE)) {
                nk_layout_row_dynamic(ctx, 30, 1);
                if (nk_button_label(ctx, "SAVE CHANGES TO GRACE.AINDEX")) {
                    char full_path[1024];
                    snprintf(full_path, sizeof(full_path), "%s/animation/grace/grace.aindex", facade_path);
                    saveAIndex(full_path, anim_list_grace, anim_list_grace_count);
                }

                for (int j = 0; j < anim_list_grace_count; j++) {
                    nk_layout_row_template_begin(ctx, 30);
                    nk_layout_row_template_push_static(ctx, 40);
                    nk_layout_row_template_push_static(ctx, 60);
                    nk_layout_row_template_push_dynamic(ctx);
                    nk_layout_row_template_push_static(ctx, 80);
                    nk_layout_row_template_end(ctx);

                    nk_labelf(ctx, NK_TEXT_LEFT, "%d", j);
                    nk_labelf(ctx, NK_TEXT_LEFT, "[%d]", anim_list_grace[j].animId);
                    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, anim_list_grace[j].name, 255, nk_filter_default);

                    if (nk_button_label(ctx, "Remove")) {
                        remove_animation(&anim_list_grace, &anim_list_grace_count, j);
                        break;
                    }
                }
            }
            nk_end(ctx);
        }

        SDL_GetWindowSize(win, NULL, NULL);

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_GL_SwapWindow(win);
    }

    nk_sdl_shutdown();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}

// gcc main.c -o facade_atool.exe `pkg-config --cflags --libs sdl2` -lGL -lm && ./facade_atool.exe