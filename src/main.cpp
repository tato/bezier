#include <vector>
#include <stdio.h>
#include <GLFW/glfw3.h>

using namespace std;

#include <moremath.h>

namespace render
{
    void draw_circle(double px, double py, double radius)
    {
        union {
            struct { int x, y, width, height; };
            int v[4];
        } viewport;
        glGetIntegerv(GL_VIEWPORT, viewport.v);

        glBegin(GL_TRIANGLE_FAN);
        static const int MAX_STEPS = 16;
        for (int step = 0; step < MAX_STEPS; step++)
        {
            double angle = step * TAU / MAX_STEPS;
            double x = px + cos(angle) * radius;
            double y = py + sin(angle) * radius;
            glVertex2f(2 * x / viewport.width - 1, -2 * y / viewport.height + 1);
        }
        glEnd();
    }
}

struct Point
{
    double x, y;
    double select_radius;
    bool selected;

    Point(double x, double y): x(x), y(y), select_radius(8), selected(true) {}
};

struct World
{
    vector<Point> points;
    int dragging_point;

    World(): points(), dragging_point(-1) {}
};

static World WORLD;

static bool intersect_point(Point& point, double x, double y)
{
    double dx = x - point.x;
    double dy = y - point.y;
    double d = sqrt(dx*dx + dy*dy);
    bool selected = d <= point.select_radius;
    return selected;
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (WORLD.dragging_point >= 0
            && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        WORLD.dragging_point = -1;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int i = 0;
        for (auto& point: WORLD.points)
        {
            if (intersect_point(point, xpos, ypos))
            {
                WORLD.dragging_point = i;
                return;
            }
            i++;
        }

        static const double RADIUS = 8;
        WORLD.points.push_back(Point(xpos, ypos));
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (WORLD.dragging_point >= 0)
    {
        printf("%d\n", WORLD.dragging_point);
        WORLD.points[WORLD.dragging_point].x = xpos;
        WORLD.points[WORLD.dragging_point].y = ypos;
    }

    for (auto& point: WORLD.points)
    {
        point.selected = intersect_point(point, xpos, ypos);
    }
}

int main(void)
{
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

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        for (auto& point: WORLD.points)
        {
            glColor3f(0.1f, 0.1f, 0.1f);
            render::draw_circle(point.x, point.y, point.select_radius);
            if (point.selected)
            {
                glColor3f(1, 1, 1);
                render::draw_circle(point.x, point.y, point.select_radius*3/4);
            }
        }

        int n = WORLD.points.size()-1;
        for (double t = 0; t < 1.0; t += 0.01)
        {
            double bx = 0;
            double by = 0;
            for (int i = 0; i <= n; i++)
            {
                auto& point = WORLD.points[i];
                double v = binomial(n, i) * pow(1 - t, n - i) * pow(t, i);
                bx += v * point.x;
                by += v * point.y;
            }
            render::draw_circle(bx, by, 2);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
