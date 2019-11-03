#include "menu.hpp"

bool Menu::init(GLFWwindow* window, vec2 screen)
{
	m_window = window;

	// Create a frame buffer
	m_frame_buffer = 0;
	glGenFramebuffers(1, &m_frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

	int fb_width, fb_height;
	glfwGetFramebufferSize(m_window, &fb_width, &fb_height);
	m_screen_scale = static_cast<float>(fb_width) / screen.x;

	// Initialize the screen texture
	m_screen_tex.create_from_screen(m_window);

	//-------------------------------------------------------------------------
	// Loading music and sounds
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return false;
	}

	m_background_music = Mix_LoadMUS(audio_path("goldleaf.wav"));

	if (m_background_music == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("goldleaf.wav"),
			audio_path("salmon_dead.wav"),
			audio_path("salmon_eat.wav"));
		return false;
	}

	// Playing background music indefinitely
	Mix_PlayMusic(m_background_music, -1);

	return true;
}

bool Menu::setup(std::vector<std::pair<std::string, Status>> buttons)
{
	float start = 400.f - (buttons.size() * 2.f * brick_size + (buttons.size() - 1) * brick_size) / 2.f;

	for (auto& s : buttons)
	{
		Button* b = new Button();
		b->set_texture_name(s.first);
		b->set_status(s.second);
		b->init(next_ui_id++, { 600.f, start + brick_size });
		m_entities.push_back(b);
		start += brick_size * 3.f;
	}

	m_rs.process(next_ui_id);

	return true;
}

void Menu::destroy()
{
	m_rs.destroy();
	m_rs.clear();

	for (auto& e : m_entities)
	{
		delete e;
	}

	m_entities.clear();
}

void Menu::draw()
{
	// Clearing error buffer
	gl_flush_errors();

	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);

	// Fake projection matrix, scales with respect to window coordinates
	// PS: 1.f / w in [1][1] is correct.. do you know why ? (:
	float left = 0.f;// *-0.5;
	float top = 0.f;// (float)h * -0.5;
	float right = (float)w / m_screen_scale;// *0.5;
	float bottom = (float)h / m_screen_scale;// *0.5;


	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	mat3 projection_2D{ { sx, 0.f, 0.f },{ 0.f, sy, 0.f },{ tx, ty, 1.f } };

	/////////////////////
	// Render to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(0, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_rs.render(projection_2D);
	//////////////////
	// Presenting
	glfwSwapBuffers(m_window);
}

bool Menu::is_over() const
{
	return m_is_over || glfwWindowShouldClose(m_window);
}

bool Menu::handle_key_press(GLFWwindow*, int key, int, int action, int mod)
{
	return true;
}

void Menu::handle_mouse_move(GLFWwindow* window, double xpos, double ypos)
{
	for (auto& e : m_entities)
	{
		e->hover({ (float)xpos, (float)ypos });
	}
}

Status Menu::handle_mouse_button(int button, int action)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		for (auto& e : m_entities)
		{
			if (e->is_click())
			{
				return e->click();
			}
		}
	}

	return Status::nothing;
}

void Menu::start_music()
{
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return;
	}

	m_background_music = Mix_LoadMUS(audio_path("goldleaf.wav"));

	if (m_background_music == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("goldleaf.wav"),
			audio_path("salmon_dead.wav"),
			audio_path("salmon_eat.wav"));
		return;
	}

	// Playing background music indefinitely
	Mix_PlayMusic(m_background_music, -1);
}

void Menu::stop_music()
{
	if (m_background_music != nullptr)
		Mix_FreeMusic(m_background_music);

	Mix_CloseAudio();
}
