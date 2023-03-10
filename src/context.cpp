#include "context.h"
#include "image.h"
#include <imgui.h>

ContextUPtr Context::Create()
{
    auto context = ContextUPtr(new Context());
    if(!context->init())
        return nullptr;
    return std::move(context);
}

void Context::Reshape(int width, int height)
{
	m_width = width;
	m_height = height;
	glViewport(0, 0, m_width, m_height);
}

void Context::ProcessInput(GLFWwindow* window)
{
	if(!m_cameraControl)
		return;

	const float cameraSpeed = 0.05f;
	if(glfwGetKey(window, GLFW_KEY_W)==GLFW_PRESS)
		m_cameraPos += cameraSpeed * m_cameraFront;
	if(glfwGetKey(window, GLFW_KEY_S)==GLFW_PRESS)
		m_cameraPos -= cameraSpeed * m_cameraFront;

	auto cameraRight = glm::normalize(glm::cross(m_cameraUp, -m_cameraFront));
	if(glfwGetKey(window, GLFW_KEY_D)==GLFW_PRESS)
		m_cameraPos += cameraSpeed * cameraRight;
	if(glfwGetKey(window, GLFW_KEY_A)==GLFW_PRESS)
		m_cameraPos -= cameraSpeed * cameraRight;

	auto cameraUp = glm::normalize(glm::cross(-m_cameraFront, cameraRight));
	if(glfwGetKey(window, GLFW_KEY_E)==GLFW_PRESS)
		m_cameraPos += cameraSpeed * cameraUp;
	if(glfwGetKey(window, GLFW_KEY_Q)==GLFW_PRESS)
		m_cameraPos -= cameraSpeed * cameraUp;
}

void Context::MouseMove(double x, double y)
{
	if(!m_cameraControl)
		return;
		
	auto pos = glm::vec2((float)x, (float)y);
	auto deltaPos = pos - m_prevMousePos;
	m_prevMousePos = pos;

	const float cameraRotSpeed = 0.8f;
	m_cameraYaw -= deltaPos.x * cameraRotSpeed;
	m_cameraPitch -= deltaPos.y * cameraRotSpeed;

	if(m_cameraYaw < 0.0f)		m_cameraYaw += 360.0f;
	if(m_cameraYaw > 360.0f)	m_cameraYaw -= 360.0f;

	if(m_cameraPitch > 89.0f)	m_cameraPitch = 89.0f;
	if(m_cameraPitch < -89.0f)	m_cameraPitch = -89.0f;
}

void Context::MouseButton(int button, int action, double x, double y)
{
	//if(button==GLFW_MOUSE_BUTTON_LEFT)
	if(button==GLFW_MOUSE_BUTTON_RIGHT)
	{
		if(action==GLFW_PRESS)
		{
			m_prevMousePos = glm::vec2((float)x, (float)y);
			m_cameraControl = true;
		}
		else if(action==GLFW_RELEASE)
		{
			m_cameraControl = false;
		}
	}
}

bool Context::init()
{
	m_box = Mesh::CreateBox();
	m_model = Model::Load("./model/backpack.obj");
	if(!m_model)
		return false;

	m_simpleProgram = Program::Create("./shader/simple.vs", "./shader/simple.fs");
	if(!m_simpleProgram)
		return false;

	m_program = Program::Create("./shader/lighting.vs", "./shader/lighting.fs");
	if(!m_program)
		return false;

    glClearColor(0.1f, 0.2f, 0.3f, 0.0f);

#if 1
    auto image = Image::Load("./image/container.jpg");
    if(!image)
        return false;
    SPDLOG_INFO("image: {}x{}, {} channels",
        image->GetWidth(), image->GetHeight(), image->GetChannelCount());
#else
    auto image = Image::Create(512, 512);
    image->SetCheckImage(16, 16);
#endif
    m_texture = Texture::CreateFromImage(image.get());

    auto image2 = Image::Load("./image/awesomeface.png");
    m_texture2 = Texture::CreateFromImage(image2.get());

    //m_material.diffuse = Texture::CreateFromImage(Image::Load("./image/container2.png").get());
    //m_material.specular = Texture::CreateFromImage(Image::Load("./image/container2_specular.png").get());
    m_material.diffuse = Texture::CreateFromImage(
		Image::CreateSingleColorImage(4, 4, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)).get());
    m_material.specular = Texture::CreateFromImage(
		Image::CreateSingleColorImage(4, 4, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)).get());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture->Get());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texture2->Get());

    m_program->Use();
	m_program->SetUniform("tex", 0);
	m_program->SetUniform("tex2", 1);

    return true;
}

void Context::Render()
{
	if (ImGui::Begin("ui window")) {
		if (ImGui::ColorEdit4("clear color", glm::value_ptr(m_clearColor))) {
			glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
		}
		ImGui::Separator();
		ImGui::DragFloat3("camera pos", glm::value_ptr(m_cameraPos), 0.01f);
		ImGui::DragFloat("camera yaw", &m_cameraYaw, 0.5f);
		ImGui::DragFloat("camera pitch", &m_cameraPitch, 0.5f, -89.0f, 89.0f);
		ImGui::Separator();
		if (ImGui::Button("reset camera")) {
			m_cameraYaw = 0.0f;
			m_cameraPitch = 0.0f;
			m_cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
		}		
		if (ImGui::CollapsingHeader("light", ImGuiTreeNodeFlags_DefaultOpen)) {
#if 0// point light
			ImGui::DragFloat3("l.position", glm::value_ptr(m_light.position), 0.01f);
			ImGui::DragFloat("l.distance", &m_light.distance, 0.5f, 0.0f, 3000.0f);
#elif 0// direction light
			ImGui::DragFloat3("l.direction", glm::value_ptr(m_light.direction), 0.01f);
#elif 1// point light
			ImGui::DragFloat3("l.position", glm::value_ptr(m_light.position), 0.01f);
			ImGui::DragFloat("l.distance", &m_light.distance, 0.5f, 0.0f, 3000.0f);
			ImGui::DragFloat3("l.direction", glm::value_ptr(m_light.direction), 0.01f);
			ImGui::DragFloat2("l.cutoff", glm::value_ptr(m_light.cutoff), 0.5f, 0.0f, 180.0f);
#endif
			ImGui::ColorEdit3("l.ambient", glm::value_ptr(m_light.ambient));
			ImGui::ColorEdit3("l.diffuse", glm::value_ptr(m_light.diffuse));
			ImGui::ColorEdit3("l.specular", glm::value_ptr(m_light.specular));
			ImGui::Checkbox("falsh light", &m_flashLightMode);
		}

		if (ImGui::CollapsingHeader("material", ImGuiTreeNodeFlags_DefaultOpen)) {
			//ImGui::ColorEdit3("m.ambient", glm::value_ptr(m_material.ambient));
			//ImGui::ColorEdit3("m.diffuse", glm::value_ptr(m_material.diffuse));
			//ImGui::ColorEdit3("m.specular", glm::value_ptr(m_material.specular));
			ImGui::DragFloat("m.shininess", &m_material.shininess, 1.0f, 1.0f, 256.0f);
		}
		ImGui::Checkbox("animation", &m_animation);
	}
	ImGui::End();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	m_cameraFront = 
		glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraYaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraPitch), glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);// ????????? ??????(???????????? ??????)
		//glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);// ?????? ??????

    auto projection = glm::perspective(glm::radians(45.0f),
		(float)m_width/(float)m_height, 0.01f, 30.0f);

	auto view = glm::lookAt(m_cameraPos,
		m_cameraPos + m_cameraFront, m_cameraUp);

	glm::vec3 lightPos = m_light.position;
	glm::vec3 lightDir = m_light.direction;
	if(m_flashLightMode) {// ??????????????? ??????
		lightPos = m_cameraPos;
		lightDir = m_cameraFront;
	} else {
		auto lightModelTransform =
		glm::translate(glm::mat4(1.0), m_light.position) *
		glm::scale(glm::mat4(1.0), glm::vec3(0.1f));
		m_simpleProgram->Use();
		m_simpleProgram->SetUniform("color", glm::vec4(m_light.ambient + m_light.diffuse, 1.0f));
		m_simpleProgram->SetUniform("transform", projection * view * lightModelTransform);
		//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		m_box->Draw(m_simpleProgram.get());
	}

	m_program->Use();
	m_program->SetUniform("viewPos", m_cameraPos);
#if 0// point light
	m_program->SetUniform("light.position", m_light.position);
	m_program->SetUniform("light.attenuation", GetAttenuationCoeff(m_light.distance));
#elif 0// direction light
	m_program->SetUniform("light.direction", m_light.direction);
#elif 1// point light
	m_program->SetUniform("light.position", lightPos);
	m_program->SetUniform("light.attenuation", GetAttenuationCoeff(m_light.distance));
	m_program->SetUniform("light.direction", lightDir);
	//m_program->SetUniform("light.cutoff", cosf(glm::radians(m_light.cutoff)));
	m_program->SetUniform("light.cutoff", glm::vec2(
		cosf(glm::radians(m_light.cutoff[0])),
		cosf(glm::radians(m_light.cutoff[0] + m_light.cutoff[1]))));
#endif
	m_program->SetUniform("light.ambient", m_light.ambient);
	m_program->SetUniform("light.diffuse", m_light.diffuse);
	m_program->SetUniform("light.specular", m_light.specular);
	//m_program->SetUniform("material.ambient", m_material.ambient);
	//m_program->SetUniform("material.diffuse", m_material.diffuse);
	//m_program->SetUniform("material.specular", m_material.specular);
	m_program->SetUniform("material.diffuse", 0);
	m_program->SetUniform("material.specular", 1);
	m_program->SetUniform("material.shininess", m_material.shininess);

	glActiveTexture(GL_TEXTURE0);
	m_material.diffuse->Bind();
	glActiveTexture(GL_TEXTURE1);
	m_material.specular->Bind();

	auto modelTransform = glm::mat4(1.0f);
	auto transform = projection * view * modelTransform;
	m_program->SetUniform("transform", transform);
	m_program->SetUniform("modelTransform", modelTransform);
	//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);// 6 x 6
	m_model->Draw(m_program.get());
}

#include <iostream>
#include <stdio.h>
#if (OS==OS_WINDOW)
    #include <direct.h>
#elif (OS==OS_MAC)
    #include <filesystem>
#endif

Context::Context()
{
    if (__cplusplus == 202002L) std::cout << "C++20";
    else if (__cplusplus == 201703L) std::cout << "C++17";
    else if (__cplusplus == 201402L) std::cout << "C++14";
    else if (__cplusplus == 201103L) std::cout << "C++11";
    else if (__cplusplus == 199711L) std::cout << "C++98";
    else std::cout << "pre-standard C++";

    char curDir[1000];
#if (OS==OS_WINDOW)
    _getcwd(curDir, 1000);
#elif (OS==OS_MAC)
    getcwd(curDir, 1000);
#endif
    printf(" curr path ?????? ?????? %s\n", curDir);
}
