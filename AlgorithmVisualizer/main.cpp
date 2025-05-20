#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include "shader.h"
#include <cstdlib> 
#include <array>
#include <vector>
#include <chrono>
#include <thread>
#include <memory>
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const unsigned int max_arr_size = 300;
const unsigned int max_vertex_count = max_arr_size * 4;
const unsigned int max_index_count = max_arr_size * 6;

float defaultColor[4] = { 1.0f, 0.5f, 0.2f, 1.0f };
float highlightColor[4] = { 1.0f, 0.2f, 0.2f, 1.0f };
bool bSorting = false;

struct Pos3
{
    float x, y, z;
};
struct Color4
{
    float r, g, b, a;
};


struct Vertex
{
    Pos3 position;
    Color4 color;
};

//functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
GLFWwindow* initWindow();
void randArray(std::array<float, max_arr_size>& arr, int size);
std::array<float, max_arr_size> scaleArray(std::array<float, max_arr_size> arr, int size, float new_min = -0.5, float new_max = 0.5);
//void bubbleSort(float* arr, int size, float baseSize, float offset, Shader ourShader, unsigned int VBO, unsigned int VAO);
void printArray(float* array, int size);
void renderRectangles(std::unique_ptr<Vertex[]>& vertices, int size, Shader ourShader, unsigned int VBO, unsigned int VAO);



static Vertex* CreateQuad(Vertex *target, float origin, float height, float baseSize, float offset, float* color)
{
    target->position = { (origin + baseSize) - offset, height, 0.0f };
    target->color = { color[0], color[1], color[2], color[3] };
    target++;

    target->position = { (origin + baseSize) - offset, -1.0, 0.0f };
    target->color = { color[0], color[1], color[2], color[3] };
    target++;

    target->position = { origin + offset, -1.0, 0.0f };
    target->color = { color[0], color[1], color[2], color[3] };
    target++;

    target->position = { origin + offset, height, 0.0f };
    target->color = { color[0], color[1], color[2], color[3] };
    target++;

    return target;
}

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = initWindow();
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //Shaders
    Shader ourShader("res/shaders/shader.vert", "res/shaders/shader.frag");


    // Create frist batch of rectangles
    int size = max_arr_size;
    int prev_size = size;
    float offset = 0.0015 / size;
    float baseSize = 2.0 / size;
    std::array<float, max_arr_size> arr;
    randArray(arr, size);
    std::array <float, max_arr_size> heights = scaleArray(arr, size);
    float origin = -1.0;
    int indices[max_index_count];
    int idx_offset = 0;
    for (int i = 0; i < max_index_count; i+=6)
    {
        indices[i] = 0 + idx_offset;
        indices[i+1] = 1 + idx_offset;
        indices[i+2] = 2 + idx_offset;
        indices[i+3] = 2 + idx_offset;
        indices[i+4] = 3 + idx_offset;
        indices[i+5] = 0 + idx_offset;
        idx_offset += 4;
    }


    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    //Bind VAO
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, max_vertex_count * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");


    
    float currentColor[4];

    bool bSorting = false;
    int currentSortIndex = 0;
    int currentComparison = 0;
    std::array<float, max_arr_size> workingArray;

    int sortDelay = 1000;

    while (!glfwWindowShouldClose(window))
    {
        //input
        processInput(window);

        //render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        //IMGUI new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        if (bSorting) {
            // Single step of bubble sort
            if (currentComparison < size - currentSortIndex - 1) {
                // Highlight compared elements
                //heights[currentComparison] *= 1.05f;
                //heights[currentComparison + 1] *= 1.05f;

                if (workingArray[currentComparison] > workingArray[currentComparison + 1]) {
                    std::swap(workingArray[currentComparison], workingArray[currentComparison + 1]);
                    // Update heights array after swap
                    heights = scaleArray(workingArray, size);
                }

                currentComparison++;

                // Add delay for visualization
                std::this_thread::sleep_for(std::chrono::microseconds(sortDelay));
            }
            else {
                currentSortIndex++;
                currentComparison = 0;

                if (currentSortIndex >= size - 1) {
                    bSorting = false;
                    arr = workingArray;  // Update original array when done
                }
            }

            // Always render the working array state
            origin = -1.0 - baseSize;
            std::unique_ptr<Vertex[]> vertices = std::make_unique<Vertex[]>(max_vertex_count);
            Vertex* buffer = vertices.get();

            for (int i = 0; i < size; i++) {
                float* color = defaultColor;
                if (i == currentComparison || i == currentComparison + 1) {
                    color = highlightColor;
                }
                buffer = CreateQuad(buffer, origin + baseSize, heights.at(i), baseSize, offset, color);
                origin = origin + baseSize;
            }
            renderRectangles(vertices, size, ourShader, VBO, VAO);
        }
        else
        {
            if (size != prev_size)
            {
                offset = 0.015 / size;
                baseSize = 2.0 / size;
                randArray(arr, size);
                heights = scaleArray(arr, size);
                prev_size = size;
            }

            //Render all triangles
            origin = -1.0 - baseSize;
            std::unique_ptr<Vertex[]> vertices = std::make_unique<Vertex[]>(max_vertex_count);
            Vertex* buffer = vertices.get();

            for (int i = 0; i < size; i++)
            {
                if (i % 2 == 1)
                    buffer = CreateQuad(buffer, origin + baseSize, heights.at(i), baseSize, offset, highlightColor);
                else
                {
                    buffer = CreateQuad(buffer, origin + baseSize, heights.at(i), baseSize, offset, defaultColor);
                }
                origin = origin + baseSize;
            }
            renderRectangles(vertices, size, ourShader, VBO, VAO);
        }
        

        // ImGUI window creation
        ImGui::Begin("Controls");
        ImGui::Text("Select the size of the array");
        ImGui::SliderInt("Size", &size,2, max_arr_size);
        ImGui::ColorEdit4("Color", defaultColor);
        ImGui::ColorEdit4("Highlight Color", highlightColor);
        ImGui::SliderInt("Time Delay", &sortDelay, 0, 10000);

        if (ImGui::Button("Randomize Array")) {
            randArray(arr, size);
            heights = scaleArray(arr, size);
        }
        if (ImGui::Button("Array Data")) {
            std::cout << "Array Data" << "\n";
            for (int i = 0; i < size; i++)
            {
                std::cout << "Index: " << i << " Data:  " << arr.at(i) << " | Height: " << heights.at(i) << "\n";
            }
        }

        if (ImGui::Button("Stop Sorting")) {
            if (bSorting) {  // Only reset if not already sorting
                bSorting = false;
            }
        }

        if (ImGui::Button("Bubble Sort")) {
            if (!bSorting) {  // Only reset if not already sorting
                bSorting = true;
                workingArray = arr;  // Work on a copy
                currentSortIndex = 0;
                currentComparison = 0;
            }
        }

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    ourShader.del();
}


void printArray(float* array, int size) {
    for (int i = 0; i < size; ++i) {
        std::cout << "  " << array[i];
    }
    std::cout << "\n";
}



GLFWwindow* initWindow()
{
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    return window;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        bSorting = true;;
}

void randArray(std::array<float, max_arr_size>& arr, int size)
{
    //srand((unsigned)time(NULL));
    
    for (int i = 0; i < size; i++)
    {
        arr.at(i) = rand() % SCR_HEIGHT + 1;
    }
}


std::array<float, max_arr_size> scaleArray(std::array<float, max_arr_size> arr, int size, float new_min, float new_max)
{   
    int max_val = *std::max_element(arr.begin(), arr.begin() + size);
    int min_val = *std::min_element(arr.begin(), arr.begin() + size);
    int range_value = max_val - min_val;
    std::array<float, max_arr_size> scaledArray{};

    for (int i = 0; i < size; i++)
    {
        scaledArray.at(i) = (arr[i] - min_val) / range_value;
    }
    float new_range = new_max - new_min;
    for (int i = 0; i < size; i++)
    {
        scaledArray.at(i) = (scaledArray.at(i) *new_range+new_min);
    }

    return scaledArray;
}


void renderRectangles(std::unique_ptr<Vertex[]>& vertices, int size, Shader ourShader, unsigned int VBO, unsigned int VAO)
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size * 4 * sizeof(Vertex), vertices.get());

    glClear(GL_COLOR_BUFFER_BIT);

    ourShader.use();
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6 * size, GL_UNSIGNED_INT, nullptr);
}
