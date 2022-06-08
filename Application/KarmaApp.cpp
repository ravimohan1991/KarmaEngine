#include<iostream>
#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"

using namespace std;

int main()
{
    std::cout<< "Lul Aukaat!!!" << endl;
    std::cout<< "GLFW version: " << glfwGetVersionString() << endl;
    glm::vec2 lul = glm::vec2(1.0f, 1.0f);
    std::cout<< "ImGui version: " << ImGui::GetVersion() << endl;
	return 0;
}
