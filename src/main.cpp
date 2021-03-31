#include <unordered_map>
#include <utility>
#include <vector>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

using namespace std;

#include <stb_easy_font.h>

#define TAU 6.283185307179586

double binomial(double n, double k)
{
    double denom = tgamma(k + 1) * tgamma(n - k + 1);
    return tgamma(n + 1) / denom;
}


namespace gl
{
    void draw_circle(double px, double py, double radius)
    {
        glBegin(GL_TRIANGLE_FAN);
        static const int MAX_STEPS = 16;
        for (int step = 0; step < MAX_STEPS; step++)
        {
            double angle = step * TAU / MAX_STEPS;
            double x = px + cos(angle) * radius;
            double y = py + sin(angle) * radius;
            glVertex2f(x, y);
        }
        glEnd();
    }

    void draw_line(vector<pair<double, double>> points, double radius)
    {
        glLineWidth(radius);
        glDisable(GL_LINE_SMOOTH);
        glBegin(GL_LINE_STRIP);
        for (const auto& [x, y]: points)
        {
            glVertex2f(x, y);
        }
        glEnd();
    }

    void draw_quads(float *vertex_buffer, int quad_num)
    {
        glBegin(GL_QUADS);
        for (int i = 0; i < quad_num*4; i++)
        {
            float x = vertex_buffer[i*4];
            float y = vertex_buffer[i*4 + 1];
            glVertex2f(x, y);
        }
        glEnd();
    }
}

struct Point
{
    double x, y;
    double select_radius;
    bool selected;

    Point(double x, double y): x(x), y(y), select_radius(4), selected(true) {}
};

struct World
{
    unordered_map<int, Point> points;
    int dragging_point;
    double dragging_distance_traveled;

    World(): points(), dragging_point(-1), dragging_distance_traveled(0) {}
};

static World WORLD;

static bool intersect_point(const Point& point, double x, double y)
{
    double dx = x - point.x;
    double dy = y - point.y;
    double d = sqrt(dx*dx + dy*dy);
    bool selected = d <= point.select_radius;
    return selected;
}

static pair<double, double> bezier_step(double t)
{
    int n = WORLD.points.size()-1;
    double bx = 0;
    double by = 0;
    int i = 0;
    for (const auto& [id, point]: WORLD.points)
    {
        double v = binomial(n, i) * pow(1 - t, n - i) * pow(t, i);
        bx += v * point.x;
        by += v * point.y;

        i += 1;
    }
    return {bx, by};
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (WORLD.dragging_point >= 0
            && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        if (WORLD.dragging_distance_traveled < 5)
        {
            WORLD.points.erase(WORLD.points.find(WORLD.dragging_point));
        }

        WORLD.dragging_point = -1;
        WORLD.dragging_distance_traveled = 0;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        for (const auto& [id, point]: WORLD.points)
        {
            if (intersect_point(point, xpos, ypos))
            {
                WORLD.dragging_point = id;
                return;
            }
        }

        static const double RADIUS = 8;
        WORLD.points.try_emplace(rand() % INT_MAX, xpos, ypos);
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (WORLD.dragging_point >= 0)
    {
        Point &p = WORLD.points.at(WORLD.dragging_point);
        {
            double dx = xpos - p.x;
            double dy = ypos - p.y;
            WORLD.dragging_distance_traveled += sqrt(dx*dx + dy*dy);
        }
        p.x = xpos;
        p.y = ypos;
    }

    for (auto& [id, point]: WORLD.points)
    {
        point.selected = intersect_point(point, xpos, ypos);
    }
}

int main(void)
{
    srand(69);
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(640, 480, "BEZIER", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    glfwMakeContextCurrent(window);

    WORLD = World();

    char delete_text_buf[50000];
    string delete_text_text = "Click screen to create point / Points can be dragged / Click a point to delete it";
    int delete_text_quads = stb_easy_font_print(
            6, 4, 
            delete_text_text.c_str(), NULL, 
            delete_text_buf, sizeof(delete_text_buf)
    );
    float *delete_text = (float *)delete_text_buf;

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        vector<pair<double, double>> curve_points;
        for (double t = 0; t < 1.0; t += 0.01)
        {
            auto [x, y] = bezier_step(t);
            curve_points.emplace_back(x, y);
        }
        {
            auto [x, y] = bezier_step(1.0);
            curve_points.emplace_back(x, y);
        }
        glColor3f(0, 0, 0);
        gl::draw_line(curve_points, 1);

        for (const auto& [id, point]: WORLD.points)
        {
            glColor3f(0, 0, 0);
            gl::draw_circle(point.x, point.y, point.select_radius);
            if (point.selected)
            {
                glColor3f(1, 1, 1);
                auto radius = point.select_radius*3/4;
                gl::draw_circle(point.x, point.y, radius);
            }
        }

        glColor3f(0, 0, 0);
        gl::draw_quads(delete_text, delete_text_quads);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
