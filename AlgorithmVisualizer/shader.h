#pragma once
#include<iostream>
#include <glm/glm.hpp>

class Shader
{
public:
	// Shader ID
	unsigned int ID;
	Shader(const char* vertexPath, const char* fragmentPath);
	// use/activate shader
	void use();
	void del();
	// utility form functions
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void setMat4(const std::string& name, const glm::mat4 &mat) const;
	void setVec4(const std::string& name, float num1, float num2, float num3, float num4) const;
private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
	void checkCompileErrors(unsigned int shader, std::string type);
};