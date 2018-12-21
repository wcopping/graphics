#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
  unsigned int ID;
  // constructor generates the shader on the fly
  // ------------------------------------------------------------------------
  Shader(const char* vertex_path, const char* fragment_path, const char* geometry_path = nullptr)
  {
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertex_code;
    std::string fragment_code;
    std::string geometry_code;
    std::ifstream v_shader_file;
    std::ifstream f_shader_file;
    std::ifstream g_shader_file;
    // ensure ifstream objects can throw exceptions:
    v_shader_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    f_shader_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    g_shader_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try 
    {
      // open files
      v_shader_file.open(vertex_path);
      f_shader_file.open(fragment_path);
      std::stringstream vShaderStream, fShaderStream;
      // read file's buffer contents into streams
      vShaderStream << v_shader_file.rdbuf();
      fShaderStream << f_shader_file.rdbuf();
      // close file handlers
      v_shader_file.close();
      f_shader_file.close();
      // convert stream into string
      vertex_code   = vShaderStream.str();
      fragment_code = fShaderStream.str();
      // if geometry shader path is present, also load a geometry shader
      if(geometry_path != nullptr) {
        g_shader_file.open(geometry_path);
        std::stringstream g_shader_stream;
        g_shader_stream << g_shader_file.rdbuf();
        g_shader_file.close();
        geometry_code = g_shader_stream.str();
      }
    }
    catch (std::ifstream::failure e)
    {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* v_shader_code = vertex_code.c_str();
    const char * f_shader_code = fragment_code.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &v_shader_code, NULL);
    glCompileShader(vertex);
    check_compile_errors(vertex, "VERTEX");
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &f_shader_code, NULL);
    glCompileShader(fragment);
    check_compile_errors(fragment, "FRAGMENT");
    // if geometry shader is given, compile geometry shader
    unsigned int geometry;
    if(geometry_path != nullptr)
    {
      const char * g_shader_code = geometry_code.c_str();
      geometry = glCreateShader(GL_GEOMETRY_SHADER);
      glShaderSource(geometry, 1, &g_shader_code, NULL);
      glCompileShader(geometry);
      check_compile_errors(geometry, "GEOMETRY");
    }
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (geometry_path != nullptr)
      glAttachShader(ID, geometry);
    glLinkProgram(ID);
    check_compile_errors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometry_path != nullptr)
      glDeleteShader(geometry);
  }
  // activate the shader
  // ------------------------------------------------------------------------
  void use() 
  { 
    glUseProgram(ID); 
  }
  // utility uniform functions
  // ------------------------------------------------------------------------
  void set_bool(const std::string &name, bool value) const
  {         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
  }
  // ------------------------------------------------------------------------
  void set_int(const std::string &name, int value) const
  { 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
  }
  // ------------------------------------------------------------------------
  void set_float(const std::string &name, float value) const
  { 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
  }
   // ------------------------------------------------------------------------
  void set_vec2(const std::string &name, const glm::vec2 &value) const
  {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  void set_vec2(const std::string &name, float x, float y) const
  {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
  }
  // ------------------------------------------------------------------------
  void set_vec3(const std::string &name, const glm::vec3 &value) const
  {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  void set_vec3(const std::string &name, float x, float y, float z) const
  {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
  }
  // ------------------------------------------------------------------------
  void set_vec4(const std::string &name, const glm::vec4 &value) const
  {
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  void set_vec4(const std::string &name, float x, float y, float z, float w) const
  {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
  }
  // ------------------------------------------------------------------------
  void set_mat2(const std::string &name, const glm::mat2 &mat) const
  {
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }
  // ------------------------------------------------------------------------
  void set_mat3(const std::string &name, const glm::mat3 &mat) const
  {
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }
  // ------------------------------------------------------------------------
  void set_mat4(const std::string &name, const glm::mat4 &mat) const
  {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }


private:
  // utility function for checking shader compilation/linking errors.
  // ------------------------------------------------------------------------
  void check_compile_errors(unsigned int shader, std::string type)
  {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success)
      {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
      }
    }
    else
    {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success)
      {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
      }
    }
  }
};
#endif

