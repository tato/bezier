#include <vector>
#include <math.h>
#include <GLFW/glfw3.h>

using namespace std;

#define TAU 6.283185307179586

struct Point
{
    double x, y;
};

struct World
{
    vector<Point> points;
};

static World WORLD;


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        WORLD.points.push_back({xpos, ypos});
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

    glfwMakeContextCurrent(window);

    WORLD = World();

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glColor3f(1, 1, 1);
        for (auto point: WORLD.points)
        {
            glBegin(GL_TRIANGLE_FAN);
            static const int MAX_STEPS = 32;
            static const double RADIUS = 10;
            for (int step = 0; step < MAX_STEPS; step++)
            {
                double angle = step * TAU / MAX_STEPS;
                double x = point.x + cos(angle) * RADIUS;
                double y = point.y + sin(angle) * RADIUS;
                glVertex2f(2 * x / width - 1, -2 * y / height + 1);
            }
            glEnd();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
