#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include <getopt.h>
#include <strings.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define ARR_SIZE(x)         sizeof(x)/sizeof(x[0])
#define BACKGROUND          ' '
#define TRESHOLD            0.001f
#define COLOR_EN            0x40
#define HORIZ_OFFSET        0.0f
#define VERT_OFFSET         0.0f
#define INCREMENT_SPEED     1.0f
#define DEG_TO_RAD(x)      0.01745f * x
#define RAD_TO_DEG(x)      57.2957f * x 
#define MILISEC_TO_MICROSEC(x)  x * 1000
#define PI                  3.14

// Rotation info [X Y Z]:
float A, B, C;
// 

struct print_data {
    int window_height;
    int window_width;
    int window_dim;
    int cube_dimension;
    int quality;
    int speed;
    int fov;
    int cam_distance;
    float *ZBuffer;
    char *visible_surface;
};

void print_states(struct print_data *out)
{
    printf("\x1b[1mParameters:\x1b[0m\n"
        "\tdistance: %d\n"
        "\twindow width: %d\n"
        "\tfov: %d\n"
        "\tquality: %d\n"
        "\tspeed: %d\n"
        "\tcube dimensions: %d\n",
        out->cam_distance,
        out->window_width,
        out->fov,
        out->quality,
        out->speed,
        out->cube_dimension);
}

void help_page(int limits[][3])
{
    printf("\n\x1b[1mNAME\x1b[0m\n"
            "\tcube - rotate a cube around its center\n"
            "\x1b[1mSYNOPSIS\x1b[0m\n"
            "\tcube [OPTION] ... [ARG]\n"
            "\x1b[1mDESCRIPTION\x1b[0m\n"
            "\t*Specifying options is not mandatory!\n"
            "\t*Default values will be applied!\n\n"
            "\t\x1b[1m-short, --long - explanation [min, max]\x1b[0m\n\n"
            "\t-d, --distance - specify camera distance from cube [%d, %d]\n"
            "\t-w, --width - specify width of cube sreen [%d, %d]\n"
            "\t-f, --fov - specify field of view [%d, %d]\n"
            "\t-q, --quality - define num of deg. per iteration [%d, %d]\n"
            "\t-s, --speed - define speed of rotation [%d, %d]\n"
            "\t-x, --dimension - define cube dimension [%d, %d]\n"
            "\t-c, --color - pick a color\n\n"
            "\t\t1. b, blue\n"
            "\t\t2. g, green\n"
            "\t\t3. r, red\n"
            "\t\t4. y, yellow\n"
            "\t\t5. m, magenta\n\n",
            limits[0][0], limits[0][2],
            limits[1][0], limits[1][2],
            limits[2][0], limits[2][2],
            limits[3][0], limits[3][2],
            limits[4][0], limits[4][2],
            limits[5][0], limits[5][2]);
}

void check_for_limits(uint8_t flags, int limits[][3],
        int *args, int size)
{
    for (int i = 0; i < size; i++) {
        if (flags & (1 << i)) {
            if (args[i] < limits[i][0]) {
                args[i] = limits[i][0];
            } else if (args[i] > limits[i][2]) {
                args[i] = limits[i][2];
            }
        }
        else {
            args[i] = limits[i][1];
        }
    }
}

struct print_data* setup(int *args)
{
    struct print_data *out;
    A = 0, B = 0, C = 0;

    out = malloc(sizeof(struct print_data));
    if (!out) {
        goto fail;
    }

    out->cam_distance = args[0];
    out->window_width = args[1];
    out->window_height = 0.5 * args[1];
    out->window_dim = args[1] * out->window_height;
    out->fov = args[2];
    out->quality = args[3];
    out->speed = args[4];
    out->cube_dimension = args[5];

    out->visible_surface = malloc(out->window_dim);
    if (!out->visible_surface) {
        free(out);
        goto fail;
    }

    out->ZBuffer = malloc(out->window_dim * sizeof(float));
    if (!out ->ZBuffer) {
        free(out->visible_surface);
        free(out);
        goto fail;
    }

    return out;

    fail:
        printf("ERR: failed to allocate memory!'\n");
        exit(1);
}

void release(struct print_data *out)
{
    free(out->visible_surface);
    free(out->ZBuffer);
    free(out);
}

void setup_color(char *color)
{
    if (!strcasecmp(color, "b") || !strcasecmp(color, "blue")) {
        printf("\x1b[38;2;51;153;255m");
    } else if (!strcasecmp(color, "g") || !strcasecmp(color, "green")) {
        printf("\x1b[38;2;102;255;102m");
    } else if (!strcasecmp(color, "r") || !strcasecmp(color, "red")) {
        printf("\x1b[38;2;255;51;51m");
    } else if (!strcasecmp(color, "y") || !strcasecmp(color, "yellow")) {
        printf("\x1b[38;2;255;255;110m");
    } else if (!strcasecmp(color, "m") || !strcasecmp(color, "magenta")) {
        printf("\x1b[38;2;255;102;255m");
    };
    return;
}

int cube_cli(int argc, char **argv, struct print_data **out)
{
    int c;
    uint8_t flags = 0;
    int opt_idx = 0;
    char *color;
    char opts[] = "d:w:f:q:s:x:c:h";

    static struct option long_options[] = {
        {"distance", required_argument, 0, 'd'},
        {"width", required_argument, 0, 'w'},
        {"fov", required_argument, 0, 'f'},
        {"quality", required_argument, 0,'q'},
        {"speed", required_argument, 0, 's'},
        {"dimension", required_argument, 0, 'x'},
        {"color", required_argument, 0, 'c'},
        {"help", no_argument, 0, 'h'}
    };

    int args[ARR_SIZE(long_options) - 2] = {0};

    // col[0] = min, col[1] = default, col[3] = max:
    int limits [ARR_SIZE(args)] [3] = {
        {10, 100, 120}, // distance
        {50, 100, 120}, // width
        {10, 40, 80}, // fov
        {1, 2, 30}, // quality
        {5, 30, 150}, //speed
        {10, 20, 40},// dimensions
    };

    while ((c = getopt_long(argc, argv, opts, long_options, &opt_idx)) != -1) {
        switch(c) {
        case 'd':
            args[0] = atoi(optarg);
            flags |= 1 << 0;
            break;
        case 'w':
            args[1] = atoi(optarg);
            flags |= 1 << 1;
            break;
        case 'f':
            args[2] = atoi(optarg);
            flags |= 1 << 2;
            break;
        case 'q':
            args[3] = atoi(optarg);
            flags |= 1 << 3;
            break;
        case 's':
            args[4] = atoi(optarg);
            flags |= 1 << 4;
            break;
        case 'x':
            args[5] = atoi(optarg);
            flags |= 1 << 5;
            break;
        case 'c':
            color = optarg;
            flags |= 1 << 6;
            break;
        case 'h':
            help_page(limits);
            return 1;
        default:
            printf("** Specified option is not available <%c> **\n", c);
            return -1;
        }
        opt_idx = 0;
    }

    check_for_limits(flags, limits, args, ARR_SIZE(args));
    *out = setup(args);
    print_states(*out);
    if (flags & COLOR_EN) {
        setup_color(color);
    }

    return 0;
}

int prompt()
{
    char input[10];

    printf("\n\nDo you want to run with these parameters (y/n)?\n");
    scanf("%s", input);
    if (!strcasecmp(input, "y") || !strcasecmp(input, "yes")) {
        return 0;
    }
    return -1;
}

int get_center_of_terminal(int *cent_row, int *cent_col, struct print_data *out)
{
    struct winsize win;
    int ret = 0;

    ret = ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
    if (ret) {
        printf("ERR: Could not get terminal dimmensions!\n");
        exit(1);
    }

    *cent_row = (win.ws_row - out->window_height)/2 - 1;
    *cent_col = (win.ws_col - out->window_width)/2 - 1;

    return ret;
}

void setup_cursor(int row, int col)
{
    printf("\x1b[%d;%dH", row, col);
}

float calc_x(float x, float y, float z)
{
    return z * (sin(A) * sin(C) + cos(A) * cos(C) * sin(B))
            - y * (cos(A) * sin(C) - cos(C) * sin(A) * sin(B))
            + x * cos(B) * cos(C);
}

float calc_y(float x, float y, float z)
{
    return y * (cos(A) * cos(C) + sin(A) * sin(B) * sin(C))
            - z * (cos(C) * sin(A) - cos(A) * sin(B) * sin(C))
            + x * cos(B) * sin(C);
}

float calc_z(float x, float y, float z)
{
    return z * cos(A) * cos(B) - x * sin(B) + y * cos(B) * sin(A);
}

void rotate_point(float x, float y, float z, int ch, struct print_data *out)
{
    float dx, dy, dz, ooz;
    int xp, yp, idx;

    dx = calc_x(x, y, z);
    dy = calc_y(x, y, z);
    dz = calc_z(x, y, z) + out->cam_distance;

    ooz = 1.0f/dz;
    xp = (int)((float)out->window_width/2.0f + HORIZ_OFFSET + (float)out->fov * dx * ooz * 2.0f);
    yp = (int)((float)out->window_height/2.0f + VERT_OFFSET + (float)out->fov * dy * ooz);
    idx = xp + yp * out->window_width;

    if (idx >= 0 && idx < out->window_dim) {
        if ((ooz - out->ZBuffer[idx]) > TRESHOLD) {
            out->ZBuffer[idx] = ooz;
            out->visible_surface[idx] = ch;
        }
    }
}

void rotation_quality(float A_deg_incr, float B_deg_incr, float C_deg_incr)
{
        A += DEG_TO_RAD(A_deg_incr);
        B += DEG_TO_RAD(B_deg_incr);
        C += DEG_TO_RAD(C_deg_incr);

        if (A > 2*PI) {
            A -= 2*PI;
        }
        if (B > 2*PI) {
            B -= 2*PI;
        }
        if (C > 2*PI) {
            C -= 2*PI;
        }
}

int main(int argc, char **argv)
{
    struct print_data *out;
    int cent_row, cent_col;
    int cube_d;
    int ret = 0;

    printf("\x1b[2J");
    printf("\x1b[H");

    ret = cube_cli(argc, argv, &out);
    if (ret) {
        exit(1);
    }
    ret = prompt();
    if (ret) {
        exit(0);
    }

    cube_d = out->cube_dimension;
    printf("\x1b[2J");
    printf("\x1b[H");
    get_center_of_terminal(&cent_row, &cent_col, out);

    for (;;) {
        printf("\x1b[%d;%dH", cent_row, cent_col);
        memset(out->visible_surface, BACKGROUND, out->window_dim);
        memset(out->ZBuffer, 0, out->window_dim * sizeof(float));

        for (float x = -cube_d; x < cube_d; x += INCREMENT_SPEED) {
            for (float y = -cube_d; y < cube_d; y += INCREMENT_SPEED) {
                rotate_point(x, y, cube_d, ';', out);
                rotate_point(x, y, -cube_d, '.', out);
                rotate_point(cube_d, y, x, '+', out);
                rotate_point(-cube_d, y, x, '~', out);
                rotate_point(x, cube_d, y, '#', out);
                rotate_point(x, -cube_d, y, '$', out);
            }
        }
        
        for (int i = 0; i < out->window_dim; i++) {
            putchar(i % out->window_width ? out->visible_surface[i] : '\n');
        }

        rotation_quality(out->quality, out->quality, out->quality);
        usleep(MILISEC_TO_MICROSEC(30));
    }

    printf("\x1b[0m");
    release(out);
    return 0;
}