#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <vector>
// randpom
#include <cstdlib>

// vars
int screenWidth = 800;
int screenHeight = 600;
const char* title = "mass_sim1";
const double frameDuration = 1.0 / 60.0;

float cameraX = 0.0f, cameraY = 0.0f;
float zoomLevel = 1.0f;
double lastMouseX, lastMouseY; 
bool dragging = false;

const double G = 6.674e-11; // m^3 kg^-1 s^-2
float initMass = 5.0f * pow(10, 20);

std::vector<float> LiveHue = {1.0f, 0.0f, 0.0f, 1.0f};
bool pause=false;

float ballr = 10;

class Object{
    private:
        std::vector<float> position = {400, 300};
        std::vector<float> velocity = {0, 0};
        float framerate = 1/60;
    public:
        bool Initalizing = false;
        bool Launched = false;
        std::vector<float> hue = {1.0f, 0.0f, 0.0f, 1.0f};
        float mass;
        float density = 3344;  // kg / m^3  HYDROGEN
        float radius;
        std::vector<float> LastPos = position;

        Object(std::vector<float> initPosition, std::vector<float> initVelocity, float mass){
            this->position = initPosition;
            this->velocity = initVelocity;
            this->mass = mass;
            this->radius = pow(((3 * this->mass/this->density)/(4 * 3.14159265359)), (1.0f/3.0f)) / 100000;
        }
        void UpdatePos(){
            this->position[0] += this->velocity[0] / 94;
            this->position[1] += this->velocity[1] / 94;
        }
        void UpdateRadius(){
            this->radius = pow(((3 * this->mass/this->density)/(4 * 3.14159265359)), (1.0f/3.0f)) / 100000;
        }
        
        void Velocity(float x, float y){
            this->velocity[0] *=x;
            this->velocity[1] *= y;
        }
        void SetVel(float x, float y){
            this->velocity = {x, y};
        }
        std::vector<float> GetCoord(){
            std::vector<float> xy = {this->position[0], this->position[1]};
            return xy;
        }
        
        void Draw(){
            float radius = this->radius;
            float x = this->position[0];
            float y = this->position[1];
            glColor4f(this->hue[0], this->hue[1], this->hue[2], this->hue[3]);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(x, y);
            int numSegments = 100;
            for (int i = 0; i <= numSegments; ++i) {
                float angle = 2.0f * 3.14159265359f * float(i) / float(numSegments);
                float dx = radius * cosf(angle);
                float dy = radius * sinf(angle);
                glVertex2f(x + dx, y + dy);
            }
             glEnd();
        }; 
        void CheckBoundry(int bottom, int top, int left, int right){
            float radius = this->radius;
            if (this->position[1] < bottom + radius || this->position[1]>top-radius){
                this->position[1] = this->position[1] < bottom+radius ? bottom+radius : top-radius;
                this->velocity[1] *= -0.8f;
            }
            if (this->position[0] < left+radius || this->position[0]>right-radius){
                this->position[0] = this->position[0] < left+radius? left+radius : right-radius;
                this->velocity[0] *= -0.8f;
            }
        }
        void CheckCollision(std::vector<Object>& objs){
            for (auto& obj2 : objs) {
                if (&obj2 == this) continue;
                float dx = obj2.GetCoord()[0] - this->position[0];
                float dy = obj2.GetCoord()[1] - this->position[1];
                float distance = sqrt(dx*dx + dy*dy);
                if (distance < (this->radius + obj2.radius) && distance != 0){
                    std::vector<float> direction = {(obj2.GetCoord()[0] - this->position[0]) / distance, (obj2.GetCoord()[1] - this->position[1]) / distance};
                    this->velocity[0],this->velocity[1] = 0;

                }
            }
        }
        void Translate(int x, int y){
            this->position[0] = x;
            this->position[1] = y;
        }

        void accelerate(float x, float y){
            this->velocity[0] += x / 96;
            this->velocity[1] += y / 96;
        }
};

std::vector<float> initVelocity = {0.0f, 0.0f};
std::vector<Object> objs = {
        //Object({0.0f, 0.0f}, initVelocity, initMass),
        //Object({800.0f, 600.0f}, initVelocity),
    };
GLFWwindow* StartGLU(int screenWidth, int screenHeight, const char* title);
void UpdateGLU();
void LiveTitle(GLFWwindow* window, double& mouseX, double& mouseY);
void glfwGetCursorPos(GLFWwindow* window, double* xpos, double* ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

int main(void)
{
    double mouseX, mouseY;
    GLFWwindow* window = StartGLU(screenWidth, screenHeight, title);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        //UpdateGLU();

        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetKeyCallback(window, keyCallback);

        LiveTitle(window, mouseX, mouseY);

        for(auto& obj : objs){
            obj.Draw();
            //obj.GetData();
            if(!pause){
                obj.UpdatePos();
            }
            // obj gravity
            for(auto& obj2 : objs){
                if(!obj2.Initalizing && !obj.Initalizing && &obj2 != &obj){
                    float dx = obj2.GetCoord()[0] - obj.GetCoord()[0];
                    float dy = obj2.GetCoord()[1] - obj.GetCoord()[1];
                    float distance = std::sqrt(dx * dx + dy * dy);

                    if (distance > 1e-6) { // Avoid very small distances
                        std::vector<float> direction = {dx / distance, dy / distance};
                        const double G = 6.674e-11; // Gravitational constant
                        distance *= 1000;
                        double Gforce = (G * obj.mass * obj2.mass) / (distance * distance);

                        float acc1 = Gforce / obj.mass;
                        float acc2 = Gforce / obj2.mass;
                        std::vector<float> acc = {direction[0] * acc1, direction[1]*acc1};
                        obj.accelerate(acc[0], acc[1]);
                    }
                }
            }
            // gravity
            //obj.accelerate(0, -9.6);
            if(obj.Initalizing){
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                    double xpos, ypos;
                    glfwGetCursorPos(window, &xpos, &ypos);
                    ypos = screenHeight - ypos;
                    obj.Velocity(0, 0);
                    obj.hue = {LiveHue[0], LiveHue[1], LiveHue[2], LiveHue[3]};
                    std::cout<<obj.hue[0]<<", "<<obj.hue[1]<<", "<<obj.hue[2]<<", "<<obj.hue[2]<<std::endl;
                    // target line
                    glColor4f(1.0f, 0.4f, 0.4f, 0.2f);
                    glLineWidth(2.0f);
                    glBegin(GL_LINES);

                    glVertex2d(xpos, ypos);
                    glVertex2d(obj.GetCoord()[0], obj.GetCoord()[1]);

                    // init mass 
                    if (xpos < obj.GetCoord()[0] + obj.radius && xpos > obj.GetCoord()[0] - obj.radius && ypos < obj.GetCoord()[1] + obj.radius && ypos > obj.GetCoord()[1] - obj.radius){
                        obj.mass *= 1.05;
                        obj.UpdateRadius();
                    }
                    glEnd();
            }
            if(obj.Launched){
                obj.Launched = false;
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                ypos = screenHeight - ypos;
                float dx = (xpos - obj.GetCoord()[0]);
                float dy = (ypos - obj.GetCoord()[1]);
                float distance = std::sqrt(dx*dx+dy*dy);
                if (distance != 0){
                    std::vector<float> direction = {(float(xpos) - obj.GetCoord()[0]) * distance * 0.01f, (float(ypos) - obj.GetCoord()[1]) * distance * 0.01f};
                    std::cout<<direction[0]<< ' '<< direction[1]<<std::endl;
                    obj.SetVel(-direction[0], -direction[1]);
                }

            }
            if(dragging){
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                ypos = screenHeight - ypos;

                float dx = xpos - lastMouseX;
                float dy = ypos - lastMouseY;
                
                for (auto& obj : objs) {
                    obj.Translate(obj.GetCoord()[0] + dx, obj.GetCoord()[1] + dy);
                }

                lastMouseX = xpos;
                lastMouseY = ypos;
            }
            //obj.CheckBoundry(0, screenHeight, 0, screenWidth);
            obj.CheckCollision(objs);

        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT){
        if (action == GLFW_PRESS){
            //draw initial circle
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            ypos = screenHeight - ypos;
            objs.emplace_back(std::vector<float>{float(xpos), float(ypos)}, std::vector<float>{0.0f, 0.0f}, initMass);
            objs[objs.size()-1].Initalizing = true;
        }
        if (action == GLFW_RELEASE){
            objs[objs.size()-1].Initalizing = false;
            objs[objs.size()-1].Launched = true;
        }

    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        lastMouseY = screenHeight - lastMouseY; // Invert Y-axis to match OpenGL coordinates
        dragging = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        dragging = false;
    }
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Check for specific key actions
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        pause = true;
    } else if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE){
        pause = false;
    } else if (key == GLFW_KEY_R && action == GLFW_REPEAT) {
         if (mods & GLFW_MOD_SHIFT) {
            LiveHue[0] -= 0.1;
        } else {
            LiveHue[0] += 0.1;
        }
    } else if (key == GLFW_KEY_G && action == GLFW_REPEAT) {
        if (mods & GLFW_MOD_SHIFT) {
            LiveHue[1] -= 0.1;
        } else {
            LiveHue[1] += 0.1;
        }
    } else if (key == GLFW_KEY_B && action == GLFW_REPEAT) {
         if (mods & GLFW_MOD_SHIFT) {
            LiveHue[2] -= 0.1;
        } else {
            LiveHue[2] += 0.1;
        }
    } else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        objs = {};
    }

    // Example: Close the window when the Escape key is pressed
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    float zoomFactor = 0.1f;
    if (yoffset > 0) {
        zoomLevel *= (1.0f - zoomFactor);
    } else if (yoffset < 0) {
        zoomLevel *= (1.0f + zoomFactor);
    }

    // Clamp zoom level to prevent inverting or excessive zoom
    zoomLevel = std::max(0.1f, std::min(zoomLevel, 10.0f));
}
void LiveTitle(GLFWwindow* window, double& mouseX, double& mouseY){
    glfwGetCursorPos(window, &mouseX, &mouseY);
    mouseY = screenHeight - mouseY; // Invert Y-axis to match OpenGL coordinates
    std::string windowTitle = std::string(title) + " | Mouse X: " + std::to_string(mouseX) + " Y: " + std::to_string(mouseY);
    glfwSetWindowTitle(window, windowTitle.c_str());
};

GLFWwindow* StartGLU(int screenWidth, int screenHeight, const char* title){
    if (!glfwInit()){
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return nullptr;
    }

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, title, NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return nullptr;
    }
    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Set up an orthographic projection
    glOrtho(0.0f, screenWidth, 0.0f, screenHeight, -1.0f, 1.0f);

    return window;
}
void UpdateGLU(){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(cameraX - screenWidth / 2 * zoomLevel, 
            cameraX + screenWidth / 2 * zoomLevel, 
            cameraY - screenHeight / 2 * zoomLevel, 
            cameraY + screenHeight / 2 * zoomLevel, 
            -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
